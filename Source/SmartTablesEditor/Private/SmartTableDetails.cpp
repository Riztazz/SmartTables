// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTableDetails.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailPropertyRow.h"
#include "IPropertyUtilities.h"
#include "Logging/StructuredLog.h"
#include "Misc/MessageDialog.h"
#include "PropertyHandle.h"
#include "SmartTable.h"
#include "SmartTableColumnsBuilder.h"
#include "SmartTableLog.h"
#include "SmartTableStyleDetails.h"
#include "SmartTableStyleNode.h"
#include "Styling/AppStyle.h"
#include "Styling/SlateColor.h"
#include "UObject/UnrealType.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "SmartTablesEditor"

namespace
{
    FString ExportColumns( USmartTable & Table, const TArray< FSmartTableColumn > & Columns )
    {
        const FArrayProperty * ColumnsProperty = CastField< FArrayProperty >( FindFProperty< FProperty >( USmartTable::StaticClass(), USmartTable::ColumnsPropertyName() ) );

        checkf( ColumnsProperty, TEXT( "USmartTable::Columns is not a reflected array; the details panel cannot write it" ) );

        FString Exported;
        ColumnsProperty->ExportText_Direct( Exported, &Columns, nullptr, &Table, PPF_None );

        return Exported;
    }
}

TSharedRef< IDetailCustomization > FSmartTableDetails::MakeInstance()
{
    return MakeShared< FSmartTableDetails >();
}

void FSmartTableDetails::CustomizeDetails( IDetailLayoutBuilder & DetailBuilder )
{
    const double Started = FPlatformTime::Seconds();

    DetailBuilder.GetObjectsBeingCustomized( CustomizedObjects );

    ColumnsHandle           = DetailBuilder.GetProperty( USmartTable::ColumnsPropertyName(), USmartTable::StaticClass() );
    ColumnSourceHandle      = DetailBuilder.GetProperty( USmartTable::ColumnSourcePropertyName(), USmartTable::StaticClass() );
    ExpectedItemClassHandle = DetailBuilder.GetProperty( USmartTable::ExpectedItemClassPropertyName(), USmartTable::StaticClass() );
    SourceTableHandle       = DetailBuilder.GetProperty( USmartTable::SourceTablePropertyName(), USmartTable::StaticClass() );
    SourceFileHandle        = DetailBuilder.GetProperty( USmartTable::SourceFilePropertyName(), USmartTable::StaticClass() );

    if ( ColumnSourceHandle.IsValid() )
    {
        ColumnSourceHandle->SetOnPropertyValuePreChange( FSimpleDelegate::CreateSP( this, &FSmartTableDetails::OnColumnSourcePreChange ) );
        ColumnSourceHandle->SetOnPropertyValueChanged( FSimpleDelegate::CreateSP( this, &FSmartTableDetails::OnColumnSourceChanged ) );
    }

    SmartTablesEditor::HideLastColumnStyle( DetailBuilder );

    IDetailCategoryBuilder & Category = DetailBuilder.EditCategory( TEXT( "Smart Table" ) );

    if ( ColumnsHandle.IsValid() && ColumnsHandle->AsArray().IsValid() )
    {
        DetailBuilder.HideProperty( ColumnsHandle );
        Category.AddCustomBuilder( MakeShared< FSmartTableColumnsBuilder >( ColumnsHandle.ToSharedRef() ) );
    }

    // clang-format off
    Category.AddCustomRow( LOCTEXT( "CopyColumnsRow", "Copy Columns From Source" ) )
        .WholeRowContent()
        [
            SNew( SButton )
                .HAlign( HAlign_Center )
                .ToolTipText( LOCTEXT( "CopyColumnsTip", "Adds a column for every field the linked source has and this table does not. Never edits or removes a column already declared." ) )
                .OnClicked( this, &FSmartTableDetails::OnCopyColumnsClicked )
                [
                    SNew( STextBlock )
                        .Text( LOCTEXT( "CopyColumns", "Copy Columns From Source" ) )
                ]
        ];
    // clang-format on
    DeferStyleRows( DetailBuilder );

    const double ElapsedMs = ( FPlatformTime::Seconds() - Started ) * 1000.0;
    UE_LOGFMT( LogSmartTables, Verbose, "Details panel built in {Ms}ms.", FString::Printf( TEXT( "%.1f" ), ElapsedMs ) );
}

void FSmartTableDetails::DeferStyleRows( IDetailLayoutBuilder & DetailBuilder )
{
    UClass * BaseClass = DetailBuilder.GetBaseClass();
    if ( !BaseClass )
    {
        UE_LOGFMT( LogSmartTables, Verbose, "Details panel: this selection shares no base class, and no style rows move." );
        return;
    }

    const TSharedPtr< IPropertyUtilities > Utilities = DetailBuilder.GetPropertyUtilities();
    const TWeakPtr< IDetailsView > DetailsView       = DetailBuilder.GetDetailsViewSharedPtr();

    for ( TFieldIterator< FProperty > It( BaseClass ); It; ++It )
    {
        FProperty * Property = *It;

        UClass * OwnerClass = Property->GetOwnerClass();
        if ( !OwnerClass || !OwnerClass->IsChildOf( USmartTable::StaticClass() ) )
        {
            continue;
        }

        if ( !Property->HasAnyPropertyFlags( CPF_Edit ) || !FSmartTableStyleNode::ShouldDefer( *Property ) )
        {
            continue;
        }

        const TSharedPtr< IPropertyHandle > Handle = DetailBuilder.GetProperty( Property->GetFName(), Property->GetOwnerClass() );
        if ( !Handle.IsValid() || !Handle->IsValidHandle() )
        {
            UE_LOGFMT( LogSmartTables, Verbose, "Details panel: {Property} hands back no handle, and its row keeps the stock layout.", Property->GetFName() );
            continue;
        }

        if ( Handle->IsExpanded() )
        {
            continue;
        }

        IDetailPropertyRow * Row = DetailBuilder.EditDefaultProperty( Handle );
        if ( !Row )
        {
            UE_LOGFMT( LogSmartTables, Verbose, "Details panel: {Property} has no default row, and it keeps the stock layout.", Property->GetFName() );
            continue;
        }

        const TSharedRef< IPropertyHandle > HandleRef = Handle.ToSharedRef();

        // clang-format off
        Row->CustomWidget( false )
            .FilterString( FSmartTableStyleNode::FilterText( HandleRef ) )
            .NameContent()
            [
                Handle->CreatePropertyNameWidget()
            ]
            .ValueContent()
            [
                SNew( SHorizontalBox )
                + SHorizontalBox::Slot()
                    .AutoWidth()
                    .VAlign( VAlign_Center )
                    .Padding( 0.0f, 0.0f, 4.0f, 0.0f )
                    [
                        SNew( SButton )
                            .ButtonStyle( FAppStyle::Get(), "SimpleButton" )
                            .ContentPadding( 0.0f )
                            .ToolTipText( LOCTEXT( "OpenStyleTip", "Open this style for editing." ) )
                            .OnClicked_Lambda( [ HandleRef, Utilities ]()
                                {
                                    HandleRef->SetExpanded( true );

                                    if ( Utilities.IsValid() )
                                    {
                                        Utilities->RequestForceRefresh();
                                    }

                                    return FReply::Handled();
                                } )
                            [
                                SNew( SImage )
                                    .Image( FAppStyle::Get().GetBrush( "TreeArrow_Collapsed" ) )
                                    .ColorAndOpacity( FSlateColor::UseForeground() )
                            ]
                    ]

                + SHorizontalBox::Slot()
                    .VAlign( VAlign_Center )
                    [
                        Handle->CreatePropertyValueWidgetWithCustomization( DetailsView.Pin().Get() )
                    ]
            ];
        // clang-format on
    }
}

FReply FSmartTableDetails::OnCopyColumnsClicked()
{
    if ( !ColumnsHandle.IsValid() )
    {
        return FReply::Handled();
    }

    TArray< FString > PerObjectValues;
    PerObjectValues.Reserve( CustomizedObjects.Num() );

    for ( const TWeakObjectPtr< UObject > & Object : CustomizedObjects )
    {
        USmartTable * Table = Cast< USmartTable >( Object.Get() );
        if ( !Table )
        {
            PerObjectValues.Add( FString() );
            continue;
        }

        PerObjectValues.Add( ExportColumns( *Table, Table->MergedColumnsFromSource() ) );
    }

    ColumnsHandle->SetPerObjectValues( PerObjectValues );

    return FReply::Handled();
}

TOptional< ESmartTableColumnSource > FSmartTableDetails::CurrentColumnSource() const
{
    if ( !ColumnSourceHandle.IsValid() )
    {
        return {};
    }

    uint8 Value = 0;
    if ( ColumnSourceHandle->GetValue( Value ) != FPropertyAccess::Success )
    {
        return {};
    }

    return static_cast< ESmartTableColumnSource >( Value );
}

void FSmartTableDetails::OnColumnSourcePreChange()
{
    if ( bRestoringColumnSource || !ColumnSourceHandle.IsValid() )
    {
        return;
    }

    ColumnSourceBeforeEdit.Reset();
    ColumnSourceHandle->GetPerObjectValues( ColumnSourceBeforeEdit );
}

void FSmartTableDetails::OnColumnSourceChanged()
{
    if ( !ColumnSourceHandle.IsValid() || bRestoringColumnSource )
    {
        return;
    }

    const TOptional< ESmartTableColumnSource > Edited = CurrentColumnSource();
    if ( !Edited.IsSet() )
    {
        UE_LOGFMT( LogSmartTables, Verbose, "Column Source change dropped. The selection holds more than one mode, so there is nothing to read from." );
        return;
    }

    const ESmartTableColumnSource NewSource = *Edited;

    TArray< FText > Lost;
    const bool bManySelected = CustomizedObjects.Num() > 1;

    for ( const TWeakObjectPtr< UObject > & Object : CustomizedObjects )
    {
        const USmartTable * Table = Cast< USmartTable >( Object.Get() );
        if ( !Table )
        {
            continue;
        }

        const FText TableLost = Table->DescribeSourcesLostBySwitchingTo( NewSource );
        if ( TableLost.IsEmpty() )
        {
            continue;
        }

        Lost.Add( bManySelected ? FText::Format( LOCTEXT( "LostOnTable", "{0}: {1}" ), FText::FromString( Table->GetName() ), TableLost ) : TableLost );
    }

    if ( Lost.IsEmpty() )
    {
        return;
    }

    const FText Summary = FText::Join( FText::FromString( LINE_TERMINATOR ), Lost );

    const EAppReturnType::Type Answer = FMessageDialog::Open( EAppMsgType::YesNo, EAppReturnType::Yes,
        bManySelected ? FText::Format( LOCTEXT( "ConfirmSourceSwitchMany", "These tables will stop pointing at:{0}{1}{0}Nothing on disk or in the Content Browser is deleted. Continue?" ), FText::FromString( LINE_TERMINATOR ), Summary )
                      : FText::Format( LOCTEXT( "ConfirmSourceSwitch", "This table will stop pointing at {0}. Nothing on disk or in the Content Browser is deleted. Continue?" ), Summary ),
        LOCTEXT( "ConfirmSourceSwitchTitle", "Smart Table" ) );

    if ( Answer != EAppReturnType::Yes )
    {
        if ( ColumnSourceBeforeEdit.Num() == CustomizedObjects.Num() )
        {
            TGuardValue< bool > Restoring( bRestoringColumnSource, true );

            ColumnSourceHandle->SetPerObjectValues( ColumnSourceBeforeEdit );
        }
        else
        {
            UE_LOGFMT( LogSmartTables, Warning, "The undo of a refused Column Source change did not run: the panel holds {Objects} object(s) but the snapshot holds {Snapshot}. Set Column Source back by hand.", CustomizedObjects.Num(), ColumnSourceBeforeEdit.Num() );
        }

        return;
    }

    ClearUnlessMode( ExpectedItemClassHandle, ESmartTableColumnSource::ItemClass, NewSource );
    ClearUnlessMode( SourceTableHandle, ESmartTableColumnSource::DataTable, NewSource );
    ClearUnlessMode( SourceFileHandle, ESmartTableColumnSource::File, NewSource );
}

void FSmartTableDetails::ClearUnlessMode( const TSharedPtr< IPropertyHandle > & Handle, ESmartTableColumnSource OwningMode, ESmartTableColumnSource CurrentMode )
{
    if ( !Handle.IsValid() || OwningMode == CurrentMode )
    {
        return;
    }

    if ( Handle == SourceFileHandle )
    {
        const TSharedPtr< IPropertyHandle > PathHandle = Handle->GetChildHandle( GET_MEMBER_NAME_CHECKED( FFilePath, FilePath ) );
        if ( PathHandle.IsValid() )
        {
            PathHandle->SetValue( FString() );
        }

        return;
    }

    Handle->SetValueFromFormattedString( TEXT( "None" ) );
}

#undef LOCTEXT_NAMESPACE
