// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "Table/SmartTableRowSpace.h"

#include "Algo/MaxElement.h"
#include "Algo/MinElement.h"
#include "Algo/Transform.h"
#include "Math/UnrealMathUtility.h"

namespace
{
    SmartTable::RowSpace::FFoundRows FindByIdsAmong( int32 NumToWalk, TFunctionRef< int32( int32 ) > RowAt, const TSet< FName > & Ids, FName CaretId, TFunctionRef< FName( int32 ) > RowIdOf )
    {
        SmartTable::RowSpace::FFoundRows Found;
        Found.Rows.Reserve( Ids.Num() );

        for ( int32 Walked = 0; Walked < NumToWalk; ++Walked )
        {
            const int32 Row   = RowAt( Walked );
            const FName RowId = RowIdOf( Row );

            if ( Ids.Contains( RowId ) )
            {
                Found.Rows.Add( Row );
            }

            if ( !CaretId.IsNone() && RowId == CaretId )
            {
                Found.Caret = Row;
            }
        }

        return Found;
    }

    int32 FindIdNearAmong( int32 NumToWalk, TFunctionRef< int32( int32 ) > RowAt, FName Id, int32 Around, TFunctionRef< FName( int32 ) > RowIdOf )
    {
        if ( NumToWalk <= 0 || Id.IsNone() )
        {
            return INDEX_NONE;
        }

        const int32 Start = FMath::Clamp( Around, 0, NumToWalk - 1 );

        for ( int32 Distance = 0; Start - Distance >= 0 || Start + Distance < NumToWalk; ++Distance )
        {
            const int32 Above = Start - Distance;
            if ( Above >= 0 && RowIdOf( RowAt( Above ) ) == Id )
            {
                return Above;
            }

            const int32 Below = Start + Distance;
            if ( Distance > 0 && Below < NumToWalk && RowIdOf( RowAt( Below ) ) == Id )
            {
                return Below;
            }
        }

        return INDEX_NONE;
    }
}

int32 SmartTable::RowSpace::StepCaret( int32 Caret, int64 Delta, int32 NumPresented )
{
    if ( NumPresented <= 0 )
    {
        return INDEX_NONE;
    }

    if ( Caret == INDEX_NONE )
    {
        return Delta > 0 ? 0 : NumPresented - 1;
    }

    return static_cast< int32 >( FMath::Clamp< int64 >( Caret + Delta, 0, NumPresented - 1 ) );
}

int32 SmartTable::RowSpace::StepCaretByPage( int32 Caret, int32 PageRows, int32 PageDelta, int32 NumPresented )
{
    return StepCaret( Caret, static_cast< int64 >( PageRows ) * PageDelta, NumPresented );
}

int32 SmartTable::RowSpace::GapForBlockMove( TConstArrayView< int32 > PresentedRows, int32 Delta, int32 NumPresented )
{
    if ( PresentedRows.IsEmpty() )
    {
        return INDEX_NONE;
    }

    const int64 Gap = Delta < 0 ? static_cast< int64 >( *Algo::MinElement( PresentedRows ) ) + Delta : static_cast< int64 >( *Algo::MaxElement( PresentedRows ) ) + Delta + 1;

    return Gap < 0 || Gap > NumPresented ? INDEX_NONE : static_cast< int32 >( Gap );
}

int32 SmartTable::RowSpace::NaturalGapFor( int32 PresentedGap, int32 NumRows, TFunctionRef< int32( int32 ) > PresentedToNatural )
{
    if ( PresentedGap <= 0 )
    {
        return 0;
    }

    const int32 RowAbove = PresentedToNatural( PresentedGap - 1 );

    return RowAbove == INDEX_NONE ? NumRows : RowAbove + 1;
}

TSet< FName > SmartTable::RowSpace::IdsOf( TConstArrayView< int32 > NaturalRows, TFunctionRef< FName( int32 ) > RowIdOf )
{
    TSet< FName > Ids;
    Ids.Reserve( NaturalRows.Num() );

    Algo::Transform( NaturalRows, Ids, RowIdOf );

    return Ids;
}

SmartTable::RowSpace::FFoundRows SmartTable::RowSpace::FindByIds( TConstArrayView< int32 > Rows, const TSet< FName > & Ids, FName CaretId, TFunctionRef< FName( int32 ) > RowIdOf )
{
    return FindByIdsAmong( Rows.Num(), [ Rows ]( int32 Walked )
    {
        return Rows[ Walked ];
    }, Ids, CaretId, RowIdOf );
}

SmartTable::RowSpace::FFoundRows SmartTable::RowSpace::FindByIdsInEveryRow( int32 NumRows, const TSet< FName > & Ids, FName CaretId, TFunctionRef< FName( int32 ) > RowIdOf )
{
    return FindByIdsAmong( NumRows, []( int32 Walked )
    {
        return Walked;
    }, Ids, CaretId, RowIdOf );
}

int32 SmartTable::RowSpace::FindIdNear( TConstArrayView< int32 > Rows, FName Id, int32 Around, TFunctionRef< FName( int32 ) > RowIdOf )
{
    return FindIdNearAmong( Rows.Num(), [ Rows ]( int32 Walked )
    {
        return Rows[ Walked ];
    }, Id, Around, RowIdOf );
}

int32 SmartTable::RowSpace::FindIdNearInEveryRow( int32 NumRows, FName Id, int32 Around, TFunctionRef< FName( int32 ) > RowIdOf )
{
    return FindIdNearAmong( NumRows, []( int32 Walked )
    {
        return Walked;
    }, Id, Around, RowIdOf );
}
