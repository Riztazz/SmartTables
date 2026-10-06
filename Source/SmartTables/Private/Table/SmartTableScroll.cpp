// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "Logging/StructuredLog.h"
#include "Math/UnrealMathUtility.h"
#include "SmartTable.h"
#include "SmartTableLog.h"
#include "SmartTableModel.h"
#include "SmartTableScrollMove.h"
#include "Table/SmartTableRowSpace.h"
#include "View/SmartTableListView.h"
#include "View/SmartTableRow.h"

void USmartTable::SampleViewAtEnd()
{
    if ( !ListView.IsValid() )
    {
        return;
    }

    const float RemainingFraction = static_cast< float >( ListView->GetScrollDistanceRemaining().Y );
    const int32 NumRows           = Model ? Model->GetNumPresentedRows() : 0;

    Scroll.bViewAtEnd = FSmartTableScrollMove::IsViewAtEnd( RemainingFraction, NumRows );
}

void USmartTable::ScrollItemIntoView( UObject * Item )
{
    if ( !ListView.IsValid() || !Model )
    {
        UE_LOGFMT( LogSmartTablesInput, Verbose, "ScrollItemIntoView({Item}) scrolled nowhere. The table still has no {Missing}.", GetNameSafe( Item ), ListView.IsValid() ? TEXT( "model" ) : TEXT( "built widget" ) );
        return;
    }

    const int32 NaturalRow   = Model->NaturalRowOfItem( Item );
    const int32 PresentedRow = PresentedRowOf( NaturalRow );

    if ( PresentedRow == INDEX_NONE )
    {
        UE_LOGFMT( LogSmartTablesInput, Verbose, "ScrollItemIntoView({Item}) scrolled nowhere. No drawn row holds that item.", GetNameSafe( Item ) );
        return;
    }

    BeginScrollToRow( PresentedRow );
}

void USmartTable::ScrollRowIntoView( int32 NaturalRow )
{

    const int32 PresentedRow = ListView.IsValid() ? PresentedRowOf( NaturalRow ) : INDEX_NONE;
    if ( PresentedRow == INDEX_NONE )
    {
        UE_LOGFMT( LogSmartTablesInput, Verbose, "ScrollRowIntoView({Row}) scrolled nowhere: {Reason}.", NaturalRow, ListView.IsValid() ? TEXT( "that row is not currently drawn - a filter may be hiding it" ) : TEXT( "the widget is not built yet" ) );
        return;
    }

    BeginScrollToRow( PresentedRow );
}

void USmartTable::ScrollToEnd()
{
    if ( !ListView.IsValid() || RowIndices.IsEmpty() )
    {
        UE_LOGFMT( LogSmartTablesInput, Verbose, "ScrollToEnd() scrolled nowhere: {Reason}.", ListView.IsValid() ? TEXT( "the table is showing no rows" ) : TEXT( "the widget is not built yet" ) );
        return;
    }

    BeginScrollToRow( RowIndices.Num() - 1 );
}

void USmartTable::BeginScrollToRow( int32 PresentedRow )
{
    if ( !ListView.IsValid() || !Model || !RowIndices.IsValidIndex( PresentedRow ) )
    {
        return;
    }

    const int32 NaturalRow = RowIndices[ PresentedRow ];

    const float RowsOnScreen = RowHeight > 0.0f ? ListView->GetTickSpaceGeometry().GetLocalSize().Y / RowHeight : 0.0f;
    const float From         = ListView->GetScrollOffset();

    if ( Scroll.Aim( PresentedRow, RowIndices.Num(), Model->KeepRow( NaturalRow ), RowsOnScreen, From, ScrollToRowSeconds ) )
    {

        UE_LOGFMT( LogSmartTablesInput, Verbose, "'{Table}' scrolls to row {Row}, shown at {Presented} of {Count}: offset {From} to {Target}.", GetName(), NaturalRow, PresentedRow, RowIndices.Num(), FMath::RoundToInt( From ), FMath::RoundToInt( Scroll.Move->Offset ) );

        return;
    }

    PlaceScrollOnRow( NaturalRow );

    UE_LOGFMT( LogSmartTablesInput, Verbose, "'{Table}' is placed on row {Row}, shown at {Presented} of {Count}: {Reason}.", GetName(), NaturalRow, PresentedRow, RowIndices.Num(), ScrollToRowSeconds > 0.0f ? TEXT( "no distance left" ) : TEXT( "no move time set" ) );
}

void USmartTable::RetargetScrollAfterStructuralChange()
{
    if ( !Scroll.Move.IsSet() || !Model )
    {
        return;
    }

    const FSmartTableKeptRow Target = Scroll.Move->Row;
    const int32 NaturalNow          = Model->FindKeptRow( Target );
    const int32 PresentedRow        = PresentedRowOf( NaturalNow );

    if ( PresentedRow == INDEX_NONE )
    {

        UE_LOGFMT( LogSmartTablesInput, Verbose, "The scroll for '{Table}' aimed at row '{Row}', which the table no longer holds. It stops where it stands.", GetName(), Target.RowId );

        Scroll.Stop();

        return;
    }

    if ( NaturalNow != Target.NaturalRow )
    {

        BeginScrollToRow( PresentedRow );
    }
}

FSmartTableHeldRow USmartTable::TopRowOnScreen()
{
    FSmartTableHeldRow Held;

    if ( !ListView.IsValid() )
    {
        return Held;
    }

    Held.Offset       = ListView->GetScrollOffset();
    Held.PresentedRow = FMath::FloorToInt32( Held.Offset );

    if ( !Scroll.LeftAtTop.RowId.IsNone() && Scroll.LeftAtTop.Offset == Held.Offset )
    {
        return Scroll.LeftAtTop;
    }

    if ( !RowIndices.IsValidIndex( Held.PresentedRow ) )
    {
        return Held;
    }

    const int32 TopRow = RowIndices[ Held.PresentedRow ];

    ForEachGeneratedRow( [ &Held, TopRow ]( SSmartTableRow & Row )
    {
        if ( Row.GetNaturalRow() == TopRow )
        {
            Held.RowId = Row.GetDrawnRowId();
        }
    } );

    return Held;
}

void USmartTable::RememberTopRow()
{
    FSmartTableHeldRow & Top = Scroll.LeftAtTop;

    Top.Offset       = ListView.IsValid() ? ListView->GetScrollOffset() : 0.0f;
    Top.PresentedRow = FMath::FloorToInt32( Top.Offset );
    Top.RowId        = Model && RowIndices.IsValidIndex( Top.PresentedRow ) ? Model->GetRowId( RowIndices[ Top.PresentedRow ] ) : NAME_None;
}

void USmartTable::ScrollAfterRowSetChange( const FSmartTableRowSetDiff & Diff, const FSmartTableHeldRow & Held, int32 PresentedBefore, bool bWasAtEnd )
{
    if ( !ListView.IsValid() || !Model )
    {
        return;
    }

    FSmartTableRowSetFacts Facts;
    Facts.bScrollToAddedRow = bScrollToAddedRow;
    Facts.bStickToEnd       = bStickToEnd;
    Facts.bViewAtEnd        = bWasAtEnd;
    Facts.bMoving           = Scroll.Move.IsSet();
    Facts.PresentedBefore   = PresentedBefore;
    Facts.PresentedAfter    = RowIndices.Num();
    Facts.OffsetBefore      = Held.Offset;

    const FSmartTableRowSetScroll Answer = FSmartTableScrollMove::ScrollAfterRowSet( Facts, Diff, [ this, &Diff ]()
    {
        return LastDrawnArrival( Diff );
    } );

    switch ( Answer.Kind )
    {
        case ESmartTableRowSetScroll::ToRow:
            BeginScrollToRow( Answer.PresentedRow );
            break;

        case ESmartTableRowSetScroll::Hold:
            HoldRowAtTop( Held, Diff );
            break;

        case ESmartTableRowSetScroll::Stay:
            break;
    }
}

void USmartTable::HoldRowAtTop( const FSmartTableHeldRow & Held, const FSmartTableRowSetDiff & Diff )
{
    if ( Held.RowId.IsNone() || Diff.Left.Contains( Held.RowId ) )
    {
        UE_LOGFMT( LogSmartTablesInput, Verbose, "'{Table}' holds no row at the top: {Reason}.", GetName(), Held.RowId.IsNone() ? TEXT( "no built row sat there" ) : TEXT( "the row that sat there left" ) );
        return;
    }

    const int32 PresentedNow = SmartTable::RowSpace::FindIdNear( RowIndices, Held.RowId, Held.PresentedRow, [ this ]( int32 NaturalRow )
    {
        return Model->GetRowId( NaturalRow );
    } );

    if ( PresentedNow == INDEX_NONE || PresentedNow == Held.PresentedRow )
    {
        return;
    }

    ListView->SetScrollOffset( Held.OffsetAt( PresentedNow ) );

    UE_LOGFMT( LogSmartTablesInput, Verbose, "'{Table}' keeps row '{Row}' at the top of the view: offset {From} to {To}.", GetName(), Held.RowId, Held.Offset, Held.OffsetAt( PresentedNow ) );
}

int32 USmartTable::LastDrawnArrival( const FSmartTableRowSetDiff & Diff ) const
{
    int32 Last = INDEX_NONE;

    for ( const FSmartTableKeptRow & Row : Diff.Arrived )
    {
        Last = FMath::Max( Last, PresentedRowOf( Model->FindKeptRow( Row ) ) );
    }

    return Last;
}

void USmartTable::PlaceScrollOnRow( int32 NaturalRow )
{
    if ( !ListView.IsValid() )
    {
        return;
    }

    if ( !RowIndices.IsEmpty() && RowIndices.Last() == NaturalRow )
    {
        ListView->ScrollToBottom();
        return;
    }

    ListView->RequestScrollIntoView( NaturalRow );
}

void USmartTable::StartAtEndIfOwed()
{
    if ( !bStartAtEnd || Scroll.bOpenedAtEnd || !ListView.IsValid() || RowIndices.IsEmpty() )
    {
        return;
    }

    if ( ListView->GetTickSpaceGeometry().GetLocalSize().Y <= 0.0f )
    {

        return;
    }

    Scroll.bOpenedAtEnd = true;

    PlaceScrollOnRow( RowIndices.Last() );

    Scroll.bViewAtEnd = true;

    UE_LOGFMT( LogSmartTablesInput, Verbose, "Table '{Table}' opens on its last row, {Row} of {Count} shown.", GetName(), RowIndices.Last(), RowIndices.Num() );
}

void USmartTable::StepScrollAnimation( float DeltaTime )
{
    if ( !Scroll.Move.IsSet() || !ListView.IsValid() )
    {
        return;
    }

    float Offset        = 0.0f;
    const int32 Arrived = Scroll.Step( ListView->GetScrollOffset(), DeltaTime, Offset );

    ListView->SetScrollOffset( Offset );

    if ( Arrived != INDEX_NONE )
    {
        PlaceScrollOnRow( Arrived );
    }
}
