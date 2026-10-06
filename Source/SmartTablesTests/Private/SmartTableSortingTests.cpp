// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTablesTestsMacros.h"

#if SMARTTABLES_WITH_TESTS

#include "AITestsCommon.h"

#include "SmartTableSorting.h"

#define LOCTEXT_NAMESPACE "SmartTablesTest"

UE_DISABLE_OPTIMIZATION_SHIP

namespace SmartTable::Test
{
    static TArray< int32 > Identity( int32 Count )
    {
        TArray< int32 > Indices;
        for ( int32 Index = 0; Index < Count; ++Index )
        {
            Indices.Add( Index );
        }

        return Indices;
    }

    static void SortOneLevel( TArray< int32 > & Indices, const TArray< FSmartTableSortKey > & Keys, ESmartTableSortMode Mode )
    {
        const TArray< TArray< FSmartTableSortKey > > Levels = { Keys };
        const TArray< ESmartTableSortMode > Modes           = { Mode };
        Sorting::SortIndices( Indices, Levels, Modes );
    }

    struct FNaturalOrderReadsDigitRunsAsNumbers : FAITestBase
    {
        virtual bool InstantTest() override
        {
            AITEST_TRUE( "Asteroid #2 sorts before #10", Sorting::CompareNatural( TEXT( "Asteroid #2" ), TEXT( "Asteroid #10" ) ) < 0 );
            AITEST_TRUE( "...and #10 after #9", Sorting::CompareNatural( TEXT( "Asteroid #10" ), TEXT( "Asteroid #9" ) ) > 0 );

            AITEST_TRUE( "Case is ignored", Sorting::CompareNatural( TEXT( "vanta" ), TEXT( "VANTA" ) ) == 0 );
            AITEST_TRUE( "A prefix comes first", Sorting::CompareNatural( TEXT( "Gar" ), TEXT( "Garris" ) ) < 0 );
            AITEST_TRUE( "Equal strings are equal", Sorting::CompareNatural( TEXT( "Keller" ), TEXT( "Keller" ) ) == 0 );

            AITEST_TRUE( "Leading zeroes break the tie", Sorting::CompareNatural( TEXT( "007" ), TEXT( "7" ) ) != 0 );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FNaturalOrderReadsDigitRunsAsNumbers, "SmartTables.Sorting.NaturalOrderReadsDigitRunsAsNumbers" );

    struct FNumericKeysDoNotOrderAsText : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TArray< FSmartTableSortKey > Keys = {
                FSmartTableSortKey::MakeNumber( 1.5 ),
                FSmartTableSortKey::MakeNumber( 1.25 ),
            };

            TArray< int32 > Indices = Identity( 2 );
            SortOneLevel( Indices, Keys, ESmartTableSortMode::Ascending );

            AITEST_EQUAL( "1.25 sorts below 1.5", Indices[ 0 ], 1 );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FNumericKeysDoNotOrderAsText, "SmartTables.Sorting.NumericKeysDoNotOrderAsText" );

    struct FEmptyKeysSortLastInBothDirections : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TArray< FSmartTableSortKey > Keys = {
                FSmartTableSortKey::MakeEmpty(),
                FSmartTableSortKey::MakeNumber( 10.0 ),
                FSmartTableSortKey::MakeEmpty(),
                FSmartTableSortKey::MakeNumber( 2.0 ),
            };

            TArray< int32 > Indices = Identity( 4 );
            SortOneLevel( Indices, Keys, ESmartTableSortMode::Ascending );
            AITEST_EQUAL( "Ascending leads with the smallest known value", Indices[ 0 ], 3 );
            AITEST_EQUAL( "...then the larger one", Indices[ 1 ], 1 );
            AITEST_TRUE( "...and the unknowns are last", Indices[ 2 ] == 0 || Indices[ 2 ] == 2 );

            Indices = Identity( 4 );
            SortOneLevel( Indices, Keys, ESmartTableSortMode::Descending );
            AITEST_EQUAL( "Descending leads with the largest known value", Indices[ 0 ], 1 );
            AITEST_EQUAL( "...then the smaller one", Indices[ 1 ], 3 );

            AITEST_TRUE( "...and the unknowns are STILL last", Indices[ 2 ] == 0 || Indices[ 2 ] == 2 );
            AITEST_TRUE( "...both of them", Indices[ 3 ] == 0 || Indices[ 3 ] == 2 );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FEmptyKeysSortLastInBothDirections, "SmartTables.Sorting.EmptyKeysSortLastInBothDirections" );

    struct FSmartTableSortIsStableForTiedRows : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TArray< FSmartTableSortKey > Keys = {
                FSmartTableSortKey::MakeNumber( 1.0 ),
                FSmartTableSortKey::MakeNumber( 1.0 ),
                FSmartTableSortKey::MakeNumber( 1.0 ),
            };

            TArray< int32 > Ascending = Identity( 3 );
            SortOneLevel( Ascending, Keys, ESmartTableSortMode::Ascending );
            AITEST_EQUAL( "Ascending keeps arrival order", Ascending, Identity( 3 ) );

            TArray< int32 > Descending = Identity( 3 );
            SortOneLevel( Descending, Keys, ESmartTableSortMode::Descending );
            AITEST_EQUAL( "So does descending", Descending, Identity( 3 ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FSmartTableSortIsStableForTiedRows, "SmartTables.Sorting.SortIsStableForTiedRows" );

    struct FSecondLevelBreaksTheFirstLevelsTies : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TArray< TArray< FSmartTableSortKey > > Levels = {
                { FSmartTableSortKey::MakeNumber( 1.0 ), FSmartTableSortKey::MakeNumber( 1.0 ), FSmartTableSortKey::MakeNumber( 0.0 ) },
                { FSmartTableSortKey::MakeText( TEXT( "Vanta" ) ), FSmartTableSortKey::MakeText( TEXT( "Garris" ) ), FSmartTableSortKey::MakeText( TEXT( "Keller" ) ) },
            };

            const TArray< ESmartTableSortMode > Modes = { ESmartTableSortMode::Ascending, ESmartTableSortMode::Ascending };

            TArray< int32 > Indices = Identity( 3 );
            Sorting::SortIndices( Indices, Levels, Modes );

            AITEST_EQUAL( "The primary level leads", Indices[ 0 ], 2 );
            AITEST_EQUAL( "Then the secondary orders the tie", Indices[ 1 ], 1 );
            AITEST_EQUAL( "...", Indices[ 2 ], 0 );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FSecondLevelBreaksTheFirstLevelsTies, "SmartTables.Sorting.SecondLevelBreaksTheFirstLevelsTies" );

    struct FBoolsLeadWithFalseAscending : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TArray< FSmartTableSortKey > Keys = {
                FSmartTableSortKey::MakeBool( true ),
                FSmartTableSortKey::MakeBool( false ),
            };

            TArray< int32 > Indices = Identity( 2 );
            SortOneLevel( Indices, Keys, ESmartTableSortMode::Ascending );
            AITEST_EQUAL( "Ascending leads with false", Indices[ 0 ], 1 );

            Indices = Identity( 2 );
            SortOneLevel( Indices, Keys, ESmartTableSortMode::Descending );
            AITEST_EQUAL( "Descending leads with true", Indices[ 0 ], 0 );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FBoolsLeadWithFalseAscending, "SmartTables.Sorting.BoolsLeadWithFalseAscending" );

    struct FDegenerateInputsDoNotCrash : FAITestBase
    {
        virtual bool InstantTest() override
        {
            TArray< int32 > Empty;
            SortOneLevel( Empty, TArray< FSmartTableSortKey >(), ESmartTableSortMode::Ascending );
            AITEST_TRUE( "An empty table sorts to nothing", Empty.IsEmpty() );

            TArray< int32 > Single = Identity( 1 );
            SortOneLevel( Single, { FSmartTableSortKey::MakeNumber( 1.0 ) }, ESmartTableSortMode::Descending );
            AITEST_EQUAL( "A single row survives", Single.Num(), 1 );

            TArray< int32 > Unsorted = Identity( 3 );
            Sorting::SortIndices( Unsorted, TArrayView< const TArray< FSmartTableSortKey > >(), TArrayView< const ESmartTableSortMode >() );
            AITEST_EQUAL( "No sort levels leaves the order untouched", Unsorted, Identity( 3 ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FDegenerateInputsDoNotCrash, "SmartTables.Sorting.DegenerateInputsDoNotCrash" );

    static FSmartTableSortColumn Level( const TCHAR * ColumnId, ESmartTableSortMode Mode )
    {
        FSmartTableSortColumn Column;
        Column.ColumnId = ColumnId;
        Column.Mode     = Mode;

        return Column;
    }

    static FSmartTableSortSpec MakeSpec( TArray< FSmartTableSortColumn > Columns )
    {
        FSmartTableSortSpec Spec;
        Spec.Columns = MoveTemp( Columns );

        return Spec;
    }

    struct FHeaderClickCyclesThroughBothDirectionsThenNone : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const ESmartTableSortMode First = Sorting::NextSortMode( ESmartTableSortMode::None, true );
            AITEST_TRUE( "An unsorted column starts ascending", First == ESmartTableSortMode::Ascending );

            const ESmartTableSortMode Second = Sorting::NextSortMode( First, true );
            AITEST_TRUE( "The second click descends", Second == ESmartTableSortMode::Descending );

            const ESmartTableSortMode Third = Sorting::NextSortMode( Second, true );
            AITEST_TRUE( "The third click clears the sort", Third == ESmartTableSortMode::None );

            AITEST_TRUE( "...and the fourth starts over", Sorting::NextSortMode( Third, true ) == ESmartTableSortMode::Ascending );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FHeaderClickCyclesThroughBothDirectionsThenNone, "SmartTables.Sorting.HeaderClickCyclesThroughBothDirectionsThenNone" );

    struct FTheCycleSkipsNoneWhenSortNoneIsDisallowed : FAITestBase
    {
        virtual bool InstantTest() override
        {
            AITEST_TRUE( "Descending wraps straight back to ascending", Sorting::NextSortMode( ESmartTableSortMode::Descending, false ) == ESmartTableSortMode::Ascending );

            AITEST_TRUE( "Ascending still descends", Sorting::NextSortMode( ESmartTableSortMode::Ascending, false ) == ESmartTableSortMode::Descending );

            AITEST_TRUE( "An unsorted column still starts ascending", Sorting::NextSortMode( ESmartTableSortMode::None, false ) == ESmartTableSortMode::Ascending );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FTheCycleSkipsNoneWhenSortNoneIsDisallowed, "SmartTables.Sorting.TheCycleSkipsNoneWhenSortNoneIsDisallowed" );

    struct FSecondaryOnAnEmptySpecBecomesThePrimary : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const FSmartTableSortSpec Promoted = Sorting::SpecWithSecondary( FSmartTableSortSpec(), TEXT( "Name" ), ESmartTableSortMode::Descending );

            AITEST_EQUAL( "One level, not two", Promoted.Columns.Num(), 1 );
            AITEST_TRUE( "...and it is the column asked for", Promoted.Columns[ 0 ].ColumnId == TEXT( "Name" ) );
            AITEST_TRUE( "...at the mode asked for", Promoted.Columns[ 0 ].Mode == ESmartTableSortMode::Descending );

            const FSmartTableSortSpec Nothing = Sorting::SpecWithSecondary( FSmartTableSortSpec(), TEXT( "Name" ), ESmartTableSortMode::None );
            AITEST_TRUE( "Promoting to None leaves the table unsorted", Nothing.IsEmpty() );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FSecondaryOnAnEmptySpecBecomesThePrimary, "SmartTables.Sorting.SecondaryOnAnEmptySpecBecomesThePrimary" );

    struct FSecondaryReplacementNeverExceedsTwoLevels : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const FSmartTableSortSpec Two = MakeSpec( { Level( TEXT( "Fleet" ), ESmartTableSortMode::Ascending ), Level( TEXT( "Name" ), ESmartTableSortMode::Descending ) } );

            const FSmartTableSortSpec Third = Sorting::SpecWithSecondary( Two, TEXT( "Mass" ), ESmartTableSortMode::Ascending );

            AITEST_EQUAL( "Still two levels", Third.Columns.Num(), 2 );
            AITEST_TRUE( "The primary is untouched", Third.Columns[ 0 ].ColumnId == TEXT( "Fleet" ) );
            AITEST_TRUE( "The newcomer took the second slot", Third.Columns[ 1 ].ColumnId == TEXT( "Mass" ) );

            const FSmartTableSortSpec Fourth = Sorting::SpecWithSecondary( Third, TEXT( "Crew" ), ESmartTableSortMode::Descending );
            AITEST_EQUAL( "A fourth request still leaves two", Fourth.Columns.Num(), 2 );
            AITEST_TRUE( "...with the newest second", Fourth.Columns[ 1 ].ColumnId == TEXT( "Crew" ) );

            const FSmartTableSortSpec Dropped = Sorting::SpecWithSecondary( Two, TEXT( "Name" ), ESmartTableSortMode::None );
            AITEST_EQUAL( "Clearing the secondary leaves the primary alone", Dropped.Columns.Num(), 1 );
            AITEST_TRUE( "...and it is still the primary", Dropped.Columns[ 0 ].ColumnId == TEXT( "Fleet" ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FSecondaryReplacementNeverExceedsTwoLevels, "SmartTables.Sorting.SecondaryReplacementNeverExceedsTwoLevels" );

    struct FSecondaryOnThePrimaryColumnMovesItRatherThanDuplicating : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const FSmartTableSortSpec Two = MakeSpec( { Level( TEXT( "Fleet" ), ESmartTableSortMode::Ascending ), Level( TEXT( "Name" ), ESmartTableSortMode::Descending ) } );

            const FSmartTableSortSpec Swapped = Sorting::SpecWithSecondary( Two, TEXT( "Fleet" ), ESmartTableSortMode::Descending );

            AITEST_EQUAL( "No duplicate level appeared", Swapped.Columns.Num(), 2 );
            AITEST_TRUE( "The old secondary is now primary", Swapped.Columns[ 0 ].ColumnId == TEXT( "Name" ) );
            AITEST_TRUE( "...and the old primary is now secondary", Swapped.Columns[ 1 ].ColumnId == TEXT( "Fleet" ) );
            AITEST_TRUE( "...at its new mode", Swapped.Columns[ 1 ].Mode == ESmartTableSortMode::Descending );

            const FSmartTableSortSpec One     = MakeSpec( { Level( TEXT( "Fleet" ), ESmartTableSortMode::Ascending ) } );
            const FSmartTableSortSpec Flipped = Sorting::SpecWithSecondary( One, TEXT( "Fleet" ), ESmartTableSortMode::Descending );

            AITEST_EQUAL( "A lone column stays a lone column", Flipped.Columns.Num(), 1 );
            AITEST_TRUE( "...still primary", Flipped.Columns[ 0 ].ColumnId == TEXT( "Fleet" ) );
            AITEST_TRUE( "...with the new direction", Flipped.Columns[ 0 ].Mode == ESmartTableSortMode::Descending );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FSecondaryOnThePrimaryColumnMovesItRatherThanDuplicating, "SmartTables.Sorting.SecondaryOnThePrimaryColumnMovesItRatherThanDuplicating" );

    struct FOnlySortableColumnsAreCandidates : FAITestBase
    {
        virtual bool InstantTest() override
        {
            TArray< FSmartTableColumn > Columns;
            for ( const TCHAR * ColumnId : { TEXT( "Name" ), TEXT( "Notes" ), TEXT( "Mass" ) } )
            {
                FSmartTableColumn & Column = Columns.AddDefaulted_GetRef();
                Column.ColumnId            = ColumnId;
            }
            Columns[ 1 ].bSortable = false;

            const TArray< FName > Candidates = Sorting::SortableColumnIds( Columns );

            AITEST_EQUAL( "Only the sortable columns are candidates", Candidates.Num(), 2 );
            AITEST_TRUE( "...in the order they were authored", Candidates[ 0 ] == TEXT( "Name" ) && Candidates[ 1 ] == TEXT( "Mass" ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FOnlySortableColumnsAreCandidates, "SmartTables.Sorting.OnlySortableColumnsAreCandidates" );
}
UE_ENABLE_OPTIMIZATION_SHIP

#undef LOCTEXT_NAMESPACE
#endif
