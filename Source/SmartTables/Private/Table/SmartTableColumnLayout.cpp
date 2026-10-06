// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTableColumnLayout.h"

#include "Algo/Accumulate.h"
#include "Algo/Transform.h"
#include "Math/UnrealMathUtility.h"

namespace
{
    using namespace SmartTable::ColumnLayout;

    TOptional< float > ClaimedWidth( const FColumn & Column )
    {
        if ( Column.UserWidth.IsSet() )
        {
            return Column.UserWidth.GetValue();
        }

        if ( Column.Sizing == ESmartTableColumnSizing::Fixed )
        {
            return Column.AuthoredWidth;
        }

        return TOptional< float >();
    }

    template< typename VisitType >
    void ForEachColumnSpan( TArrayView< const FColumn > ShownColumns, float LeadingOffset, VisitType && Visit )
    {
        float Left = LeadingOffset;

        for ( const FColumn & Column : ShownColumns )
        {
            if ( Column.CurrentWidth <= 0.0f )
            {
                return;
            }

            const float Right = Left + Column.CurrentWidth;

            if ( Visit( Column, Left, Right ) )
            {
                return;
            }

            Left = Right;
        }
    }
}

TArray< SmartTable::ColumnLayout::FResolvedWidth > SmartTable::ColumnLayout::ResolveWidths( TArrayView< const FColumn > Columns, float TotalWidth, float MinWidth, bool bStretchLast )
{
    TArray< FResolvedWidth > Resolved;
    if ( TotalWidth <= 0.0f )
    {
        return Resolved;
    }

    float FillTotal = 0.0f;
    int32 NumFills  = 0;
    float SpokenFor = 0.0f;

    for ( const FColumn & Column : Columns )
    {
        if ( const TOptional< float > Claimed = ClaimedWidth( Column ) )
        {
            SpokenFor += FMath::Max( Claimed.GetValue(), MinWidth );
        }
        else
        {
            FillTotal += Column.AuthoredWidth;
            ++NumFills;
        }
    }

    const float Divisible = FMath::Max( TotalWidth - SpokenFor, 0.0f );

    Resolved.Reserve( Columns.Num() );
    for ( const FColumn & Column : Columns )
    {

        const float Share = FillTotal > 0.0f ? Divisible * ( Column.AuthoredWidth / FillTotal ) : Divisible / FMath::Max( NumFills, 1 );

        Resolved.Add( { Column.ColumnId, FMath::Max( ClaimedWidth( Column ).Get( Share ), MinWidth ) } );
    }

    if ( bStretchLast && !Resolved.IsEmpty() )
    {
        const TArrayView< const FResolvedWidth > Before( Resolved.GetData(), Resolved.Num() - 1 );
        const float Others = Algo::TransformAccumulate( Before, []( const FResolvedWidth & Width )
        {
            return Width.Width;
        }, 0.0f );

        FResolvedWidth & Last = Resolved.Last();
        Last.Width            = StretchLast( Last.Width, TotalWidth, Others );
    }

    return Resolved;
}

SmartTable::ColumnLayout::FEdgeHit SmartTable::ColumnLayout::FindEdgeAt( TArrayView< const FColumn > ShownColumns, float LocalX, float GripWidth, float LeadingOffset )
{
    FEdgeHit Hit;

    ForEachColumnSpan( ShownColumns, LeadingOffset, [ & ]( const FColumn & Column, float Left, float Right )
    {
        if ( Column.bResizable && FMath::Abs( LocalX - Right ) <= GripWidth )
        {
            Hit = FEdgeHit{ Column.ColumnId, Left };

            return true;
        }

        return false;
    } );

    return Hit;
}

SmartTable::ColumnLayout::FGrantedDrag SmartTable::ColumnLayout::ResolveGrowDrag( float StartDraggedWidth, float DesiredWidth, TArrayView< const float > StartRightWidths, float MinWidth )
{
    FGrantedDrag Result;
    Result.RightWidths = StartRightWidths;

    float Givable = 0.0f;
    for ( const float Width : StartRightWidths )
    {
        if ( Width <= 0.0f )
        {

            Result.Granted = FMath::Max( DesiredWidth, MinWidth );

            return Result;
        }

        Givable += FMath::Max( Width - MinWidth, 0.0f );
    }

    const float Ceiling = FMath::Max( StartDraggedWidth + Givable, MinWidth );

    Result.Granted = FMath::Clamp( DesiredWidth, MinWidth, Ceiling );

    float Taken = Result.Granted - StartDraggedWidth;

    for ( int32 Index = 0; Index < Result.RightWidths.Num(); ++Index )
    {
        const float Start = StartRightWidths[ Index ];
        const float Next  = FMath::Max( Start - Taken, MinWidth );

        Result.RightWidths[ Index ] = Next;

        Taken -= Start - Next;
    }

    Result.bNegotiated = true;

    return Result;
}

float SmartTable::ColumnLayout::StretchLast( float BaseWidth, float TotalWidth, float OthersTotal )
{
    return FMath::Max( BaseWidth, TotalWidth - OthersTotal );
}

SmartTable::ColumnLayout::EHeaderWidthMode SmartTable::ColumnLayout::HeaderWidthModeFor( const FSmartTableColumn & Column, bool bWidthsLive )
{
    if ( bWidthsLive && Column.bResizable )
    {
        return EHeaderWidthMode::Manual;
    }

    return Column.Sizing == ESmartTableColumnSizing::Fixed ? EHeaderWidthMode::Fixed : EHeaderWidthMode::Fill;
}

TArray< FName > SmartTable::ColumnLayout::MovedColumns( const TMap< FName, float > & Before, TArrayView< const FResolvedWidth > After, float Epsilon )
{
    TArray< FName > Moved;

    Algo::TransformIf( After, Moved, [ &Before, Epsilon ]( const FResolvedWidth & Entry )
    {
        const float * Previous = Before.Find( Entry.ColumnId );

        return !Previous || !FMath::IsNearlyEqual( *Previous, Entry.Width, Epsilon );
    }, &FResolvedWidth::ColumnId );

    return Moved;
}

float SmartTable::ColumnLayout::AutoScrollStep( float CursorX, float ViewportWidth, float EdgeZone, float Step )
{
    if ( CursorX > ViewportWidth - EdgeZone )
    {
        return Step;
    }

    return CursorX < EdgeZone ? -Step : 0.0f;
}

TArray< FName > SmartTable::ColumnLayout::MergeOrder( TArrayView< const FName > AuthoredIds, TArrayView< const FName > RequestedIds )
{
    TArray< FName > Merged;
    Merged.Reserve( AuthoredIds.Num() );

    for ( const FName ColumnId : RequestedIds )
    {
        if ( AuthoredIds.Contains( ColumnId ) && !Merged.Contains( ColumnId ) )
        {
            Merged.Add( ColumnId );
        }
    }

    for ( const FName ColumnId : AuthoredIds )
    {
        if ( !Merged.Contains( ColumnId ) )
        {
            Merged.Add( ColumnId );
        }
    }

    return Merged;
}

int32 SmartTable::ColumnLayout::LandingIndex( TArrayView< const FName > Order, int32 From, int32 Delta, TFunctionRef< bool( FName ) > IsShown )
{
    const int32 Step = Delta > 0 ? 1 : -1;

    int32 To        = INDEX_NONE;
    int32 StepsLeft = FMath::Abs( Delta );

    for ( int32 Index = From + Step; Order.IsValidIndex( Index ) && StepsLeft > 0; Index += Step )
    {
        if ( !IsShown( Order[ Index ] ) )
        {
            continue;
        }

        --StepsLeft;
        To = Index;
    }

    return To;
}

FName SmartTable::ColumnLayout::ColumnAt( TArrayView< const FColumn > ShownColumns, float LocalX, float LeadingOffset )
{
    FName Found;

    ForEachColumnSpan( ShownColumns, LeadingOffset, [ & ]( const FColumn & Column, float Left, float Right )
    {
        if ( LocalX >= Left && LocalX < Right )
        {
            Found = Column.ColumnId;

            return true;
        }

        return false;
    } );

    return Found;
}

TArray< int32 > SmartTable::ColumnLayout::WidthsToWrite( TArrayView< const float > Targets, TArrayView< const float > Drawn, float Epsilon )
{
    TArray< int32 > Moved;

    for ( int32 Index = 0; Index < Targets.Num(); ++Index )
    {
        if ( Drawn.IsValidIndex( Index ) && FMath::IsNearlyEqual( Drawn[ Index ], Targets[ Index ], Epsilon ) )
        {
            continue;
        }

        Moved.Add( Index );
    }

    return Moved;
}
