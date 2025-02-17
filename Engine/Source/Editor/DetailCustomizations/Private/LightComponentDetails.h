// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class IDetailLayoutBuilder;
class IPropertyHandle;
class ULightComponent;

class FLightComponentDetails : public IDetailCustomization
{
public:
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance();

	/** IDetailCustomization interface */
	virtual void CustomizeDetails( IDetailLayoutBuilder& DetailBuilder ) override;

	static void SetComponentIntensity(ULightComponent* Component, float InIntensity);

private:

	/** Helper functions which tell whether the various custom controls are enabled or not */
	bool IsLightBrightnessEnabled() const;
	bool IsUseIESBrightnessEnabled() const;
	bool IsIESBrightnessScaleEnabled() const;
	// NVCHANGE_BEGIN: Add VXGI
	bool IsCastVXGIIndirectLightingEnabled() const;
	// NVCHANGE_END: Add VXGI

	void ResetIntensityToDefault(TSharedPtr<IPropertyHandle> PropertyHandle, ULightComponent* Component);
	bool IsIntensityResetToDefaultVisible(TSharedPtr<IPropertyHandle> PropertyHandle, ULightComponent* Component) const;

private:
	TSharedPtr<IPropertyHandle> IESBrightnessTextureProperty;
	TSharedPtr<IPropertyHandle> IESBrightnessEnabledProperty;
	TSharedPtr<IPropertyHandle> IESBrightnessScaleProperty;
	TSharedPtr<IPropertyHandle> LightIntensityProperty;
	// NVCHANGE_BEGIN: Add VXGI
	TSharedPtr<IPropertyHandle> CastVXGIIndirectLightingProperty;
	TSharedPtr<IPropertyHandle> MobilityProperty;
	// NVCHANGE_END: Add VXGI
};
