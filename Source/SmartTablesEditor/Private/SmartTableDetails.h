// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#pragma once

#include "IDetailCustomization.h"
#include "Input/Reply.h"
#include "SmartTableTypes.h"
#include "Templates/SharedPointer.h"
#include "UObject/WeakObjectPtr.h"

class IPropertyHandle;

class FSmartTableDetails : public IDetailCustomization
{
public:
    static TSharedRef< IDetailCustomization > MakeInstance();

    virtual void CustomizeDetails( IDetailLayoutBuilder & DetailBuilder ) override;

private:
    FReply OnCopyColumnsClicked();

    void DeferStyleRows( IDetailLayoutBuilder & DetailBuilder );

    TOptional< ESmartTableColumnSource > CurrentColumnSource() const;

    void OnColumnSourcePreChange();
    void OnColumnSourceChanged();

    void ClearUnlessMode( const TSharedPtr< IPropertyHandle > & Handle, ESmartTableColumnSource OwningMode, ESmartTableColumnSource CurrentMode );

    TSharedPtr< IPropertyHandle > ColumnsHandle;
    TSharedPtr< IPropertyHandle > ColumnSourceHandle;
    TSharedPtr< IPropertyHandle > ExpectedItemClassHandle;
    TSharedPtr< IPropertyHandle > SourceTableHandle;
    TSharedPtr< IPropertyHandle > SourceFileHandle;

    TArray< TWeakObjectPtr< UObject > > CustomizedObjects;

    TArray< FString > ColumnSourceBeforeEdit;

    bool bRestoringColumnSource = false;
};
