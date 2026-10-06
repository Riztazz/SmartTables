// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#pragma once

#include "IDetailCustomNodeBuilder.h"
#include "Templates/SharedPointer.h"

class IPropertyHandle;
class IPropertyHandleArray;

class FSmartTableColumnsBuilder : public IDetailCustomNodeBuilder, public TSharedFromThis< FSmartTableColumnsBuilder >
{
public:
    explicit FSmartTableColumnsBuilder( TSharedRef< IPropertyHandle > InColumnsHandle );

    virtual void SetOnRebuildChildren( FSimpleDelegate InOnRegenerateChildren ) override;
    virtual void GenerateHeaderRowContent( FDetailWidgetRow & NodeRow ) override;
    virtual void GenerateChildContent( IDetailChildrenBuilder & ChildrenBuilder ) override;
    virtual FName GetName() const override;

    int32 NumColumns() const;

    void AddColumn();
    void EmptyColumns();
    void InsertAt( int32 Index );
    void DuplicateAt( int32 Index );
    void DeleteAt( int32 Index );
    void MoveBy( int32 Index, int32 Delta );

private:
    void Rebuild() const;

    TSharedRef< IPropertyHandle > ColumnsHandle;
    TSharedPtr< IPropertyHandleArray > ArrayHandle;

    FSimpleDelegate OnRebuildChildren;
};
