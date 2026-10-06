// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "Model/SmartTablePreviewModel.h"

#include "SmartTableConstants.h"

#include "Engine/DataTable.h"
#include "Logging/StructuredLog.h"
#include "Misc/AssertionMacros.h"
#include "SmartTableLog.h"

#define LOCTEXT_NAMESPACE "SmartTables"

void USmartTablePreviewModel::SetPreview( const TArray< FSmartTableColumn > & InColumns, const UStruct * InSchema, const UDataTable * InRowSource, int32 InNumRows )
{
    Columns   = InColumns;
    Schema    = InSchema;
    RowSource = InRowSource;

    RowNames.Reset();
    if ( RowSource )
    {

        RowNames = RowSource->GetRowNames();
        if ( RowNames.Num() > InNumRows )
        {
            RowNames.SetNum( InNumRows );
        }
    }

    NumRows = RowSource ? RowNames.Num() : FMath::Max( 0, InNumRows );

    UE_LOGFMT( LogSmartTablesData, Verbose, "Designer preview: {Rows} row(s) across {Columns} column(s), values from {Source}.", NumRows, Columns.Num(), RowSource ? RowSource->GetName() : ( Schema ? Schema->GetName() : FString( TEXT( "no source" ) ) ) );

    NotifyNumRowsChanged();
}

const UStruct * USmartTablePreviewModel::EffectiveSchema() const
{
    return RowSource ? RowSource->GetRowStruct() : Schema.Get();
}

int32 USmartTablePreviewModel::GetNumRows_Implementation()
{
    return NumRows;
}

FText USmartTablePreviewModel::GetCellText_Implementation( int32 NaturalRow, FName ColumnId )
{
    const int32 ColumnIndex = Columns.IndexOfByPredicate( [ ColumnId ]( const FSmartTableColumn & Candidate )
    {
        return Candidate.ColumnId == ColumnId;
    } );

    if ( ColumnIndex == INDEX_NONE )
    {
        UE_LOGFMT( LogSmartTablesData, VeryVerbose, "The preview carries no column '{Column}', and row {Row} draws nothing.", ColumnId, NaturalRow );
        return FText::GetEmpty();
    }

    const FSmartTableColumn & Column = Columns[ ColumnIndex ];

    const UStruct * Against         = EffectiveSchema();
    const SmartTable::FBinding Bind = Against ? SmartTable::ResolveBinding( Against, Column.GetValueBinding() ) : SmartTable::FBinding();

    if ( Against && !Bind.IsValid() )
    {
        return SmartTable::Text::UnresolvedBinding( Column.GetValueBinding() );
    }

    if ( RowSource )
    {

        if ( RowNames.IsValidIndex( NaturalRow ) )
        {
            if ( const uint8 * RowData = RowSource->FindRowUnchecked( RowNames[ NaturalRow ] ) )
            {
                const FText Value = SmartTable::ReadStructAsText( Bind, RowData, Column );
                if ( !Value.IsEmpty() )
                {
                    return Value;
                }
            }
        }
    }
    else if ( const UClass * AsClass = Cast< UClass >( Against ) )
    {
        if ( UObject * ClassDefaults = AsClass->GetDefaultObject() )
        {
            const FText Value = SmartTable::ReadAsText( Bind, ClassDefaults, Column );
            if ( !Value.IsEmpty() )
            {
                return Value;
            }
        }
    }

    const FText Label = Column.GetLabel();

    return FText::Format( LOCTEXT( "PreviewCell", "{0} {1}" ), Label, FText::AsNumber( NaturalRow + 1 ) );
}

#undef LOCTEXT_NAMESPACE
