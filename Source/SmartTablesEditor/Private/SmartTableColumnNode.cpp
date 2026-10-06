// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTableColumnNode.h"

#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "PropertyCustomizationHelpers.h"
#include "PropertyHandle.h"
#include "SmartTableColumnsBuilder.h"
#include "SmartTableStyleNode.h"
#include "SmartTableTypes.h"
#include "UObject/UnrealType.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "SmartTablesEditor"

FSmartTableColumnNode::FSmartTableColumnNode( TSharedRef< IPropertyHandle > InElementHandle, int32 InIndex, TWeakPtr< FSmartTableColumnsBuilder > InOwner )
    : ElementHandle( InElementHandle )
    , Index( InIndex )
    , Owner( InOwner )
{
}

FName FSmartTableColumnNode::GetName() const
{
    return FName( *FString::Printf( TEXT( "SmartTableColumn_%d" ), Index ) );
}

TSharedPtr< IPropertyHandle > FSmartTableColumnNode::GetPropertyHandle() const
{
    return ElementHandle;
}

FText FSmartTableColumnNode::Title() const
{
    const TSharedPtr< IPropertyHandle > IdHandle = ElementHandle->GetChildHandle( GET_MEMBER_NAME_CHECKED( FSmartTableColumn, ColumnId ) );

    FName ColumnId;
    if ( IdHandle.IsValid() && IdHandle->GetValue( ColumnId ) == FPropertyAccess::Success && !ColumnId.IsNone() )
    {
        return FText::FromName( ColumnId );
    }

    return FText::Format( LOCTEXT( "UnnamedColumn", "Column {0}" ), FText::AsNumber( Index ) );
}

void FSmartTableColumnNode::GenerateHeaderRowContent( FDetailWidgetRow & NodeRow )
{
    const TWeakPtr< FSmartTableColumnsBuilder > WeakOwner = Owner;
    const int32 CapturedIndex                             = Index;

    auto Act = [ WeakOwner, CapturedIndex ]( void ( FSmartTableColumnsBuilder::*Op )( int32 ) )
    {
        return FExecuteAction::CreateLambda( [ WeakOwner, CapturedIndex, Op ]()
        {
            if ( const TSharedPtr< FSmartTableColumnsBuilder > Pinned = WeakOwner.Pin() )
            {
                ( Pinned.Get()->*Op )( CapturedIndex );
            }
        } );
    };

    FString FilterNames = Title().ToString();
    for ( TFieldIterator< FProperty > It( FSmartTableColumn::StaticStruct() ); It; ++It )
    {
        FilterNames.AppendChar( TEXT( ' ' ) );
        FilterNames.Append( It->GetDisplayNameText().ToString() );
    }

    // clang-format off
    NodeRow.FilterString( FText::FromString( FilterNames ) )
        .NameContent()
        [
            SNew( STextBlock )
                .Text( this, &FSmartTableColumnNode::Title )
                .Font( IDetailLayoutBuilder::GetDetailFont() )
        ]
        .ValueContent()
        .MinDesiredWidth( 180.0f )
        [
            SNew( SHorizontalBox )
            + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    PropertyCustomizationHelpers::MakeInsertDeleteDuplicateButton( Act( &FSmartTableColumnsBuilder::InsertAt ), Act( &FSmartTableColumnsBuilder::DeleteAt ), Act( &FSmartTableColumnsBuilder::DuplicateAt ) )
                ]

            + SHorizontalBox::Slot()
                .AutoWidth()
                .Padding( 4.0f, 0.0f, 0.0f, 0.0f )
                [
                    SNew( SButton )
                        .ToolTipText( LOCTEXT( "MoveUpTip", "Move this column one place earlier." ) )
                        .OnClicked_Lambda( [ WeakOwner, CapturedIndex ]()
                            {
                                if ( const TSharedPtr< FSmartTableColumnsBuilder > Pinned = WeakOwner.Pin() )
                                {
                                    Pinned->MoveBy( CapturedIndex, -1 );
                                }

                                return FReply::Handled();
                            } )
                        [
                            SNew( STextBlock )
                                .Text( LOCTEXT( "MoveUp", "Up" ) )
                                .Font( IDetailLayoutBuilder::GetDetailFont() )
                        ]
                ]

            + SHorizontalBox::Slot()
                .AutoWidth()
                .Padding( 4.0f, 0.0f, 0.0f, 0.0f )
                [
                    SNew( SButton )
                        .ToolTipText( LOCTEXT( "MoveDownTip", "Move this column one place later." ) )
                        .OnClicked_Lambda( [ WeakOwner, CapturedIndex ]()
                            {
                                if ( const TSharedPtr< FSmartTableColumnsBuilder > Pinned = WeakOwner.Pin() )
                                {
                                    Pinned->MoveBy( CapturedIndex, 1 );
                                }

                                return FReply::Handled();
                            } )
                        [
                            SNew( STextBlock )
                                .Text( LOCTEXT( "MoveDown", "Down" ) )
                                .Font( IDetailLayoutBuilder::GetDetailFont() )
                        ]
                ]
        ];
    // clang-format on
}

void FSmartTableColumnNode::GenerateRealChildren( IDetailChildrenBuilder & ChildrenBuilder )
{
    FSmartTableStyleNode::AddChildren( ChildrenBuilder, ElementHandle );
}

#undef LOCTEXT_NAMESPACE
