// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "LightComponentDetails.h"
#include "Components/SceneComponent.h"
#include "Components/LightComponentBase.h"
#include "Components/LocalLightComponent.h"
#include "Misc/Attribute.h"
#include "Components/LightComponent.h"
#include "PropertyHandle.h"
#include "DetailLayoutBuilder.h"
#include "IDetailPropertyRow.h"
#include "DetailCategoryBuilder.h"

#define LOCTEXT_NAMESPACE "LightComponentDetails"

TSharedRef<IDetailCustomization> FLightComponentDetails::MakeInstance()
{
	return MakeShareable( new FLightComponentDetails );
}

void FLightComponentDetails::CustomizeDetails( IDetailLayoutBuilder& DetailBuilder )
{
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);
	ULightComponent* Component = Cast<ULightComponent>(Objects[0].Get());

	// Mobility property is on the scene component base class not the light component and that is why we have to use USceneComponent::StaticClass
	TSharedRef<IPropertyHandle> MobilityHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULightComponent, Mobility), USceneComponent::StaticClass());
	// Set a mobility tooltip specific to lights
	MobilityHandle->SetToolTipText(LOCTEXT("LightMobilityTooltip", "Mobility for lights controls what the light is allowed to do at runtime and therefore what rendering methods are used.\n* A movable light uses fully dynamic lighting and anything can change in game, however it has a large performance cost, typically proportional to the light's influence size.\n* A stationary light will only have its shadowing and bounced lighting from static geometry baked by Lightmass, all other lighting will be dynamic.  It can change color and intensity in game. \n* A static light is fully baked into lightmaps and therefore has no performance cost, but also can't change in game."));

	IDetailCategoryBuilder& LightCategory = DetailBuilder.EditCategory( "Light", FText::GetEmpty(), ECategoryPriority::TypeSpecific );

	// The bVisible checkbox in the rendering category is frequently used on lights
	// Editing the rendering category and giving it TypeSpecific priority will place it just under the Light category
	DetailBuilder.EditCategory("Rendering", FText::GetEmpty(), ECategoryPriority::TypeSpecific);

	LightIntensityProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULightComponent, Intensity), ULightComponentBase::StaticClass());
	IESBrightnessTextureProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULightComponent, IESTexture));
	IESBrightnessEnabledProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULightComponent, bUseIESBrightness));
	IESBrightnessScaleProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULightComponent, IESBrightnessScale));

	if( !IESBrightnessEnabledProperty->IsValidHandle() )
	{
		// Brightness and color should be listed first
		LightCategory.AddProperty( LightIntensityProperty );
		LightCategory.AddProperty( DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULightComponent, LightColor), ULightComponentBase::StaticClass() ) );
	}
	else
	{
		IDetailCategoryBuilder& LightProfilesCategory = DetailBuilder.EditCategory( "Light Profiles", FText::GetEmpty(), ECategoryPriority::Default );

		LightCategory.AddProperty( LightIntensityProperty )
			.IsEnabled( TAttribute<bool>( this, &FLightComponentDetails::IsLightBrightnessEnabled ) )
			.OverrideResetToDefault(FResetToDefaultOverride::Create(FIsResetToDefaultVisible::CreateSP(this, &FLightComponentDetails::IsIntensityResetToDefaultVisible, Component), FResetToDefaultHandler::CreateSP(this, &FLightComponentDetails::ResetIntensityToDefault, Component)));

		LightCategory.AddProperty( DetailBuilder.GetProperty("LightColor", ULightComponentBase::StaticClass() ) );

		LightProfilesCategory.AddProperty( IESBrightnessTextureProperty );

		LightProfilesCategory.AddProperty( IESBrightnessEnabledProperty )
			.IsEnabled( TAttribute<bool>( this, &FLightComponentDetails::IsUseIESBrightnessEnabled ) );

		LightProfilesCategory.AddProperty( IESBrightnessScaleProperty)
			.IsEnabled( TAttribute<bool>( this, &FLightComponentDetails::IsIESBrightnessScaleEnabled ) );
	}

	// NVCHANGE_BEGIN: Add VXGI
	MobilityProperty = MobilityHandle;

	IDetailCategoryBuilder& VxgiCategory = DetailBuilder.EditCategory("VXGI", FText::GetEmpty(), ECategoryPriority::TypeSpecific);

	CastVXGIIndirectLightingProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULightComponent, bCastVxgiIndirectLighting));
	if (CastVXGIIndirectLightingProperty->IsValidHandle())
	{
		VxgiCategory.AddProperty(CastVXGIIndirectLightingProperty)
			.IsEnabled(TAttribute<bool>(this, &FLightComponentDetails::IsCastVXGIIndirectLightingEnabled));
	}
	// NVCHANGE_END: Add VXGI
}

bool FLightComponentDetails::IsLightBrightnessEnabled() const
{
	return !IsIESBrightnessScaleEnabled();
}

bool FLightComponentDetails::IsUseIESBrightnessEnabled() const
{
	UObject* IESTexture;
	IESBrightnessTextureProperty->GetValue(IESTexture);
	return (IESTexture != NULL);
}

bool FLightComponentDetails::IsIESBrightnessScaleEnabled() const
{
	bool Enabled;
	IESBrightnessEnabledProperty->GetValue(Enabled);
	return IsUseIESBrightnessEnabled() && Enabled;
}

// NVCHANGE_BEGIN: Add VXGI
bool FLightComponentDetails::IsCastVXGIIndirectLightingEnabled() const
{
	uint8 Mobility;
	MobilityProperty->GetValue(Mobility);
	return (Mobility == EComponentMobility::Movable);
}
// NVCHANGE_END: Add VXGI
void FLightComponentDetails::SetComponentIntensity(ULightComponent* Component, float InIntensity)
{
	check(Component);

	UProperty* IntensityProperty = FindFieldChecked<UProperty>(ULightComponent::StaticClass(), GET_MEMBER_NAME_CHECKED(ULightComponent, Intensity));
	FPropertyChangedEvent PropertyChangedEvent(IntensityProperty);

	const float PreviousIntensity = Component->Intensity;
	Component->SetLightBrightness(InIntensity);
	Component->PostEditChangeProperty(PropertyChangedEvent);
	Component->MarkRenderStateDirty();

	// Propagate changes to instances.
	TArray<UObject*> Instances;
	Component->GetArchetypeInstances(Instances);
	for (UObject* Instance : Instances)
	{
		ULocalLightComponent* InstanceComponent = Cast<ULocalLightComponent>(Instance);
		if (InstanceComponent && InstanceComponent->Intensity == PreviousIntensity)
		{
			InstanceComponent->Intensity = Component->Intensity;
			InstanceComponent->PostEditChangeProperty(PropertyChangedEvent);
			InstanceComponent->MarkRenderStateDirty();
		}
	}
}

void FLightComponentDetails::ResetIntensityToDefault(TSharedPtr<IPropertyHandle> PropertyHandle, ULightComponent* Component)
{
	ULightComponent* ArchetypeComponent = Component ? Cast<ULocalLightComponent>(Component->GetArchetype()) : nullptr;
	if (ArchetypeComponent)
	{
		SetComponentIntensity(Component, ArchetypeComponent->ComputeLightBrightness());
	}
	else
	{
		// Fall back to default handler. 
		PropertyHandle->ResetToDefault();
	}
}

bool FLightComponentDetails::IsIntensityResetToDefaultVisible(TSharedPtr<IPropertyHandle> PropertyHandle, ULightComponent* Component) const
{
	ULightComponent* ArchetypeComponent = Component ? Cast<ULocalLightComponent>(Component->GetArchetype()) : nullptr;
	if (ArchetypeComponent)
	{
		return !FMath::IsNearlyEqual(Component->ComputeLightBrightness(), ArchetypeComponent->ComputeLightBrightness());
	}
	else
	{
		// Fall back to default handler
		return PropertyHandle->DiffersFromDefault();
	}
}

#undef LOCTEXT_NAMESPACE
