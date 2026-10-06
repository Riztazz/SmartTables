// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTableSorting.h"

#include "SmartTableHelpers.h"

#include "Algo/Transform.h"
#include "Misc/AssertionMacros.h"

namespace
{

    int64 ConsumeNumber( const FString & Text, int32 & Index, int32 & OutLeadingZeroes )
    {
        OutLeadingZeroes = 0;
        while ( Index < Text.Len() && Text[ Index ] == TEXT( '0' ) )
        {
            ++OutLeadingZeroes;
            ++Index;
        }

        int64 Value = 0;
        while ( Index < Text.Len() && SmartTable::IsAsciiDigit( Text[ Index ] ) )
        {

            if ( Value < ( TNumericLimits< int64 >::Max() / 10 ) - 9 )
            {
                Value = ( Value * 10 ) + ( Text[ Index ] - TEXT( '0' ) );
            }

            ++Index;
        }

        return Value;
    }
}

int32 SmartTable::Sorting::CompareNatural( const FString & A, const FString & B )
{
    int32 IndexA = 0;
    int32 IndexB = 0;

    int32 ZeroTieBreak = 0;

    while ( IndexA < A.Len() && IndexB < B.Len() )
    {
        const bool bDigitA = SmartTable::IsAsciiDigit( A[ IndexA ] );
        const bool bDigitB = SmartTable::IsAsciiDigit( B[ IndexB ] );

        if ( bDigitA && bDigitB )
        {
            int32 ZeroesA      = 0;
            int32 ZeroesB      = 0;
            const int64 ValueA = ConsumeNumber( A, IndexA, ZeroesA );
            const int64 ValueB = ConsumeNumber( B, IndexB, ZeroesB );

            if ( ValueA != ValueB )
            {
                return ValueA < ValueB ? -1 : 1;
            }

            if ( ZeroTieBreak == 0 && ZeroesA != ZeroesB )
            {
                ZeroTieBreak = ZeroesA < ZeroesB ? -1 : 1;
            }

            continue;
        }

        const TCHAR CharA = FChar::ToLower( A[ IndexA ] );
        const TCHAR CharB = FChar::ToLower( B[ IndexB ] );
        if ( CharA != CharB )
        {
            return CharA < CharB ? -1 : 1;
        }

        ++IndexA;
        ++IndexB;
    }

    const int32 RemainingA = A.Len() - IndexA;
    const int32 RemainingB = B.Len() - IndexB;
    if ( RemainingA != RemainingB )
    {
        return RemainingA < RemainingB ? -1 : 1;
    }

    return ZeroTieBreak;
}

void SmartTable::Sorting::SortIndices( TArray< int32 > & InOutIndices, TArrayView< const TArray< FSmartTableSortKey > > KeyLevels, TArrayView< const ESmartTableSortMode > Modes )
{
    if ( KeyLevels.IsEmpty() || Modes.IsEmpty() )
    {
        return;
    }

    ensureMsgf( KeyLevels.Num() == Modes.Num(), TEXT( "%d sort key level(s) against %d direction(s)" ), KeyLevels.Num(), Modes.Num() );

    const int32 LevelCount = FMath::Min( KeyLevels.Num(), Modes.Num() );

    InOutIndices.Sort( [ &KeyLevels, &Modes, LevelCount ]( int32 IndexA, int32 IndexB )
    {
        for ( int32 Level = 0; Level < LevelCount; ++Level )
        {
            const TArray< FSmartTableSortKey > & Keys = KeyLevels[ Level ];
            if ( !Keys.IsValidIndex( IndexA ) || !Keys.IsValidIndex( IndexB ) )
            {
                continue;
            }

            const FSmartTableSortKey & KeyA = Keys[ IndexA ];
            const FSmartTableSortKey & KeyB = Keys[ IndexB ];

            if ( KeyA.IsEmpty() != KeyB.IsEmpty() )
            {
                return !KeyA.IsEmpty();
            }

            if ( KeyA.IsEmpty() )
            {

                continue;
            }

            const int32 Order = KeyA.Compare( KeyB );
            if ( Order != 0 )
            {
                return Modes[ Level ] == ESmartTableSortMode::Descending ? Order > 0 : Order < 0;
            }
        }

        return IndexA < IndexB;
    } );
}

TSet< FName > SmartTable::Sorting::FindColumnsThatCannotBreakTies( int32 NumPresentedRows, FName PrimaryColumn, TArrayView< const FName > Candidates, TFunctionRef< FSmartTableSortKey( int32, FName ) > ReadKey )
{

    TSet< FName > Powerless;
    Powerless.Reserve( Candidates.Num() );

    for ( const FName Candidate : Candidates )
    {
        if ( Candidate != PrimaryColumn )
        {
            Powerless.Add( Candidate );
        }
    }

    if ( Powerless.IsEmpty() || NumPresentedRows < 2 )
    {
        return Powerless;
    }

    TArray< FName, TInlineAllocator< 16 > > Freed;

    int32 RunStart              = 0;
    FSmartTableSortKey Previous = ReadKey( 0, PrimaryColumn );

    for ( int32 Index = 1; Index <= NumPresentedRows; ++Index )
    {
        FSmartTableSortKey Current;
        bool bRunEnds = true;

        if ( Index < NumPresentedRows )
        {
            Current  = ReadKey( Index, PrimaryColumn );
            bRunEnds = Current.Compare( Previous ) != 0;
        }

        if ( bRunEnds && Index - RunStart >= 2 )
        {
            Freed.Reset();

            for ( const FName Candidate : Powerless )
            {
                const FSmartTableSortKey First = ReadKey( RunStart, Candidate );

                for ( int32 Row = RunStart + 1; Row < Index; ++Row )
                {
                    if ( ReadKey( Row, Candidate ).Compare( First ) != 0 )
                    {
                        Freed.Add( Candidate );
                        break;
                    }
                }
            }

            for ( const FName Candidate : Freed )
            {
                Powerless.Remove( Candidate );
            }

            if ( Powerless.IsEmpty() )
            {
                return Powerless;
            }
        }

        if ( bRunEnds )
        {
            RunStart = Index;
        }

        Previous = MoveTemp( Current );
    }

    return Powerless;
}

ESmartTableSortMode SmartTable::Sorting::NextSortMode( ESmartTableSortMode Current, bool bAllowNone )
{
    if ( Current == ESmartTableSortMode::Ascending )
    {
        return ESmartTableSortMode::Descending;
    }

    if ( Current == ESmartTableSortMode::Descending )
    {
        return bAllowNone ? ESmartTableSortMode::None : ESmartTableSortMode::Ascending;
    }

    return ESmartTableSortMode::Ascending;
}

FSmartTableSortSpec SmartTable::Sorting::SpecWithPrimary( FName ColumnId, ESmartTableSortMode Mode )
{
    FSmartTableSortSpec Spec;
    if ( Mode != ESmartTableSortMode::None )
    {
        FSmartTableSortColumn & Column = Spec.Columns.AddDefaulted_GetRef();
        Column.ColumnId                = ColumnId;
        Column.Mode                    = Mode;
    }

    return Spec;
}

bool SmartTable::Sorting::HasLevelOtherThan( const FSmartTableSortSpec & Spec, FName ColumnId )
{
    return Spec.Columns.ContainsByPredicate( [ ColumnId ]( const FSmartTableSortColumn & Level )
    {
        return Level.ColumnId != ColumnId;
    } );
}

FSmartTableSortSpec SmartTable::Sorting::SpecWithSecondary( const FSmartTableSortSpec & Current, FName ColumnId, ESmartTableSortMode Mode )
{
    if ( Current.IsEmpty() )
    {
        return SpecWithPrimary( ColumnId, Mode );
    }

    FSmartTableSortSpec Spec = Current;

    Spec.Columns.RemoveAll( [ ColumnId ]( const FSmartTableSortColumn & Candidate )
    {
        return Candidate.ColumnId == ColumnId;
    } );

    if ( Mode != ESmartTableSortMode::None )
    {
        Spec.Columns.SetNum( FMath::Min( Spec.Columns.Num(), 1 ) );

        FSmartTableSortColumn & Column = Spec.Columns.AddDefaulted_GetRef();
        Column.ColumnId                = ColumnId;
        Column.Mode                    = Mode;
    }

    return Spec;
}

TArray< FName > SmartTable::Sorting::SortableColumnIds( TArrayView< const FSmartTableColumn > Columns )
{
    TArray< FName > Ids;
    Ids.Reserve( Columns.Num() );

    Algo::TransformIf( Columns, Ids, []( const FSmartTableColumn & Column )
    {
        return Column.bSortable;
    }, &FSmartTableColumn::ColumnId );

    return Ids;
}
