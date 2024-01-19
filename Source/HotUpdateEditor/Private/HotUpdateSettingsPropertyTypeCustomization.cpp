// Copyright Epic Games, Inc. All Rights Reserved.

#include "HotUpdateSettingsPropertyTypeCustomization.h"

//////////////////////////////////////////////////////////////////////////
// FHotUpdateSettingsPropertyTypeCustomization

TSharedRef<IPropertyTypeCustomization> FHotUpdateSettingsPropertyTypeCustomization::MakeInstance()
{
	return MakeShareable(new FHotUpdateSettingsPropertyTypeCustomization);
}

void FHotUpdateSettingsPropertyTypeCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{

}

void FHotUpdateSettingsPropertyTypeCustomization::CustomizeChildren(TSharedRef<class IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{

}