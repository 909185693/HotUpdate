// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Layout/Visibility.h"
#include "IDetailCustomization.h"
#include "PropertyHandle.h"

//////////////////////////////////////////////////////////////////////////
// FSpritePolygonCollectionCustomization

class FHotUpdateSettingsDetailCustomization : public IDetailCustomization
{
public:
	// Makes a new instance of this customization
	static TSharedRef<IDetailCustomization> MakeInstance();

	// IDetailCustomization interface
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override;
	// End of IDetailCustomization interface

protected:
	virtual TSharedRef<SWidget> GetReleaseChannelContent();
	virtual FText GetCurrentReleaseChannel() const;
	virtual void OnReleaseChannelChanged(int32 Index);

	virtual FName GetReleaseChannel() const;
	virtual void SetReleaseChannel(const FName& NewReleaseChannel);

private:
	TSharedPtr<IPropertyHandle> ReleaseChannelProperty;
};
