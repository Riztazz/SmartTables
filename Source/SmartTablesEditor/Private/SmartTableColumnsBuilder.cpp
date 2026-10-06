// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTableColumnsBuilder.h"

#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "PropertyCustomizationHelpers.h"
#include "PropertyHandle.h"
#include "SmartTableColumnNode.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "SmartTablesEditor"

FSmartTableColumnsBuilder::FSmartTableColumnsBuilder( TSharedRef< IPropertyHandle > InColumnsHandle )
    : ColumnsHandle( InColumnsHandle )
    , ArrayHandle( InColumnsHandle->AsArray() )
{
    checkf( ArrayHandle.IsValid(), TEXT( "FSmartTableColumnsBuilder was handed a property that is not an array" ) );
}

void FSmartTableColumnsBuilder::SetOnRebuildChildren( FSimpleDelegate InOnRegenerateChildren )
{
    OnRebuildChildren = InOnRegenerateChildren;
}

FName FSmartTableColumnsBuilder::GetName() const
{
    return TEXT( "SmartTableColumns" );
}

int32 FSmartTableColumnsBuilder::NumColumns() const
{
    uint32 Count = 0;
    ArrayHandle->GetNumElements( Count );

    return static_cast< int32 >( Count );
}

void FSmartTableColumnsBuilder::Rebuild() const
{
    OnRebuildChildren.ExecuteIfBound();
}

void FSmartTableColumnsBuilder::AddColumn()
{
    ArrayHandle->AddItem();
    Rebuild();
}

void FSmartTableColumnsBuilder::EmptyColumns()
{
    ArrayHandle->EmptyArray();
    Rebuild();
}

void FSmartTableColumnsBuilder::InsertAt( int32 Index )
{
    ArrayHandle->Insert( Index );
    Rebuild();
}

void FSmartTableColumnsBuilder::DuplicateAt( int32 Index )
{
    ArrayHandle->DuplicateItem( Index );
    Rebuild();
}

void FSmartTableColumnsBuilder::DeleteAt( int32 Index )
{
    ArrayHandle->DeleteItem( Index );
    Rebuild();
}

void FSmartTableColumnsBuilder::MoveBy( int32 Index, int32 Delta )
{
    const int32 Target = Index + Delta;

    if ( Target < 0 || Target >= NumColumns() )
    {
        return;
    }

    ArrayHandle->MoveElementTo( Index, Target );
    Rebuild();
}

void FSmartTableColumnsBuilder::GenerateHeaderRowContent( FDetailWidgetRow & NodeRow )
{
    const TWeakPtr< FSmartTableColumnsBuilder > WeakSelf = SharedThis( this );

    // clang-format off
    NodeRow.FilterString( LOCTEXT( "ColumnsFilter", "Columns" ) )
        .NameContent()
        [
            SNew( STextBlock )
                .Text( LOCTEXT( "Columns", "Columns" ) )
                .Font( IDetailLayoutBuilder::GetDetailFontBold() )
        ]
        .ValueContent()
        .MinDesiredWidth( 180.0f )
        [
            SNew( SHorizontalBox )
            + SHorizontalBox::Slot()
                .VAlign( VAlign_Center )
                .AutoWidth()
                [
                    SNew( STextBlock )
                        .Text_Lambda( [ WeakSelf ]()
                            {
                                const TSharedPtr< FSmartTableColumnsBuilder > Pinned = WeakSelf.Pin();

                                return Pinned.IsValid() ? FText::Format( LOCTEXT( "ColumnCount", "{0} columns" ), FText::AsNumber( Pinned->NumColumns() ) ) : FText::GetEmpty();
                            } )
                        .Font( IDetailLayoutBuilder::GetDetailFont() )
                ]

            + SHorizontalBox::Slot()
                .AutoWidth()
                .Padding( 8.0f, 0.0f, 0.0f, 0.0f )
                [
                    PropertyCustomizationHelpers::MakeAddButton( FSimpleDelegate::CreateLambda(
                        [ WeakSelf ]()
                        {
                            if ( const TSharedPtr< FSmartTableColumnsBuilder > Pinned = WeakSelf.Pin() )
                            {
                                Pinned->AddColumn();
                            }
                        } ),
                        LOCTEXT( "AddColumnTip", "Add a column to the end." ) )
                ]

            + SHorizontalBox::Slot()
                .AutoWidth()
                .Padding( 4.0f, 0.0f, 0.0f, 0.0f )
                [
                    PropertyCustomizationHelpers::MakeEmptyButton( FSimpleDelegate::CreateLambda(
                        [ WeakSelf ]()
                        {
                            if ( const TSharedPtr< FSmartTableColumnsBuilder > Pinned = WeakSelf.Pin() )
                            {
                                Pinned->EmptyColumns();
                            }
                        } ),
                        LOCTEXT( "EmptyColumnsTip", "Remove every column." ) )
                ]
        ];
    // clang-format on
}

void FSmartTableColumnsBuilder::GenerateChildContent( IDetailChildrenBuilder & ChildrenBuilder )
{
    const int32 Count = NumColumns();

    for ( int32 Index = 0; Index < Count; ++Index )
    {
        ChildrenBuilder.AddCustomBuilder( MakeShared< FSmartTableColumnNode >( ArrayHandle->GetElement( Index ), Index, SharedThis( this ) ) );
    }
}

#undef LOCTEXT_NAMESPACE
