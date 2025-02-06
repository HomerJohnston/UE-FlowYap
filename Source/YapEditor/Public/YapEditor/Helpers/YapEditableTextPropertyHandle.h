// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Engine/GameViewportClient.h"
#include "PropertyHandle.h"
#include "DetailWidgetRow.h"
#include "IPropertyUtilities.h"
#include "SYapTextPropertyEditableTextBox.h"

#define LOCTEXT_NAMESPACE "YapEditor"

/** Allows STextPropertyEditableTextBox to edit a property handle */
class FYapEditableTextPropertyHandle : public IEditableTextProperty
{
public:
    FYapEditableTextPropertyHandle(FText& InText, UObject* InYapDialogueNode)
        : Text(InText)
		, YapDialogueNode(InYapDialogueNode)
    {
        static const FName NAME_MaxLength = "MaxLength";
    }

    virtual bool IsMultiLineText() const override
    {
        return true; // TODO make this a project setting???
    }

    virtual bool IsPassword() const override
    {
        return false;
    }

    virtual bool IsReadOnly() const override
    {
        return false; // TODO make dialogue elements lockable?
        //return !PropertyHandle->IsValidHandle() || PropertyHandle->IsEditConst();
    }

    virtual bool IsDefaultValue() const override
    {
        return Text.IsEmpty();
    }

    virtual FText GetToolTipText() const override
    {
        return Text; // TODO
    }

    virtual int32 GetNumTexts() const override
    {
        return 1;
    }

    virtual FText GetText(const int32 InIndex) const override
    {
        return Text;
    }

    virtual void SetText(const int32 InIndex, const FText& InText) override
    {
    	FYapScopedTransaction Transaction("TODO", LOCTEXT("EditTextProperties", "Edit Text Properties"), YapDialogueNode.Get());
        Text = InText;
    }

    virtual bool IsValidText(const FText& InText, FText& OutErrorMsg) const override
    {
        return true;
    }

#if USE_STABLE_LOCALIZATION_KEYS
    virtual void GetStableTextId(const int32 InIndex, const ETextPropertyEditAction InEditAction, const FString& InTextSource, const FString& InProposedNamespace, const FString& InProposedKey, FString& OutStableNamespace, FString& OutStableKey) const override
    {
    	check(InIndex == 0);
    	StaticStableTextId(YapDialogueNode.Get(), InEditAction, InTextSource, InProposedNamespace, InProposedKey, OutStableNamespace, OutStableKey);
        /*
        if (PropertyHandle->IsValidHandle())
        {
            static const FName NAME_UniqueIdWhenEditedOnDefault  = "UniqueIdWhenEditedOnDefault";
            static const FName NAME_UniqueIdWhenEditedOnInstance = "UniqueIdWhenEditedOnInstance";

            UPackage* PropertyPackage = nullptr;
            bool bForceUniqueId = false;

            // We can't query if this property handle is under an object or an external struct, so try and get the objects first before falling back to using the packages
            // Note: We want to use the object where possible so that we can tell if we're editing a CDO/archetype or an instance
            {
                TArray<UObject*> PropertyObjects;
                PropertyHandle->GetOuterObjects(PropertyObjects);

                if (PropertyObjects.IsValidIndex(InIndex))
                {
                    PropertyPackage = PropertyObjects[InIndex]->GetPackage();
                    bForceUniqueId = PropertyHandle->HasMetaData(PropertyObjects[InIndex]->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject) ? NAME_UniqueIdWhenEditedOnDefault : NAME_UniqueIdWhenEditedOnInstance);
                }
            }
            if (!PropertyPackage || PropertyPackage == GetTransientPackage())
            {
                TArray<UPackage*> PropertyPackages;
                PropertyHandle->GetOuterPackages(PropertyPackages);

                check(PropertyPackages.IsValidIndex(InIndex));
                PropertyPackage = PropertyPackages[InIndex];
            }

            ensureAlwaysMsgf(PropertyPackage, TEXT("A package must be available for key stabilization to work correctly. Did you forget to set the package on a FStructOnScope?"));
            StaticStableTextId(PropertyPackage, InEditAction, InTextSource, InProposedNamespace, bForceUniqueId ? FString() : InProposedKey, OutStableNamespace, OutStableKey);
        }
        */
    }
#endif // USE_STABLE_LOCALIZATION_KEYS

private:
    FText& Text;

	TWeakObjectPtr<UObject> YapDialogueNode;
	
    /** The maximum length of the value that can be edited, or <=0 for unlimited */
    int32 MaxLength = 0;
};

#undef LOCTEXT_NAMESPACE