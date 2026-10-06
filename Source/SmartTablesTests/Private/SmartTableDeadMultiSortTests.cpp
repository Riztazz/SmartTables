// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTablesTestsMacros.h"

#if SMARTTABLES_WITH_TESTS

#include "AITestsCommon.h"

#include "SmartTable.h"
#include "SmartTableSorting.h"
#include "SmartTableTestTypes.h"

#define LOCTEXT_NAMESPACE "SmartTablesTest"

UE_DISABLE_OPTIMIZATION_SHIP

namespace SmartTable::Test
{
    struct FDeadSortFixture
    {
        TArray< FName > ColumnIds;
        TArray< TArray< FString > > Rows;

        FSmartTableSortKey Read( int32 PresentedRow, FName ColumnId ) const
        {
            const int32 Column = ColumnIds.IndexOfByKey( ColumnId );
            if ( Column == INDEX_NONE || !Rows.IsValidIndex( PresentedRow ) )
            {
                return FSmartTableSortKey::MakeEmpty();
            }

            return FSmartTableSortKey::MakeText( Rows[ PresentedRow ][ Column ] );
        }

        TSet< FName > Powerless( FName Primary ) const
        {
            return Sorting::FindColumnsThatCannotBreakTies( Rows.Num(), Primary, ColumnIds, [ this ]( int32 PresentedRow, FName ColumnId )
            {
                return Read( PresentedRow, ColumnId );
            } );
        }
    };

    static FDeadSortFixture MakeFixture()
    {
        FDeadSortFixture Fixture;
        Fixture.ColumnIds = { TEXT( "Callsign" ), TEXT( "Class" ), TEXT( "Owner" ), TEXT( "Distance" ) };

        Fixture.Rows = {
            { TEXT( "Halcyon-1" ), TEXT( "Asteroid" ), TEXT( "Vanta" ), TEXT( "400" ) },
            { TEXT( "Kelvin-2" ), TEXT( "Asteroid" ), TEXT( "Vanta" ), TEXT( "300" ) },
            { TEXT( "Sable-3" ), TEXT( "Asteroid" ), TEXT( "Vanta" ), TEXT( "200" ) },
            { TEXT( "Ember-4" ), TEXT( "Booster" ), TEXT( "Keller" ), TEXT( "900" ) },
            { TEXT( "Vesta-5" ), TEXT( "Booster" ), TEXT( "Keller" ), TEXT( "800" ) },
        };

        return Fixture;
    }

    struct FUniquePrimaryLeavesEveryColumnPowerless : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FDeadSortFixture Fixture = MakeFixture();

            const TSet< FName > Powerless = Fixture.Powerless( TEXT( "Callsign" ) );

            AITEST_TRUE( "Class cannot break a unique primary", Powerless.Contains( TEXT( "Class" ) ) );
            AITEST_TRUE( "Owner cannot either", Powerless.Contains( TEXT( "Owner" ) ) );
            AITEST_TRUE( "Nor Distance", Powerless.Contains( TEXT( "Distance" ) ) );

            AITEST_FALSE( "The primary is not listed", Powerless.Contains( TEXT( "Callsign" ) ) );
            AITEST_EQUAL( "Every other column is powerless", Powerless.Num(), 3 );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FUniquePrimaryLeavesEveryColumnPowerless, "SmartTables.MultiSort.UniquePrimaryLeavesEveryColumnPowerless" );

    struct FTiedPrimaryFreesTheColumnsThatVary : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FDeadSortFixture Fixture = MakeFixture();

            const TSet< FName > Powerless = Fixture.Powerless( TEXT( "Class" ) );

            AITEST_FALSE( "Callsign varies within a class", Powerless.Contains( TEXT( "Callsign" ) ) );
            AITEST_FALSE( "Distance varies within a class", Powerless.Contains( TEXT( "Distance" ) ) );

            AITEST_TRUE( "Owner is constant inside every group", Powerless.Contains( TEXT( "Owner" ) ) );
            AITEST_EQUAL( "Only Owner is powerless", Powerless.Num(), 1 );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FTiedPrimaryFreesTheColumnsThatVary, "SmartTables.MultiSort.TiedPrimaryFreesTheColumnsThatVary" );

    struct FGroupsAreCheckedIndependently : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FDeadSortFixture Fixture;
            Fixture.ColumnIds = { TEXT( "Class" ), TEXT( "Note" ) };

            Fixture.Rows = {
                { TEXT( "Asteroid" ), TEXT( "same" ) },
                { TEXT( "Asteroid" ), TEXT( "same" ) },
                { TEXT( "Booster" ), TEXT( "left" ) },
                { TEXT( "Booster" ), TEXT( "right" ) },
            };

            const TSet< FName > Powerless = Fixture.Powerless( TEXT( "Class" ) );

            AITEST_FALSE( "A later group frees the column", Powerless.Contains( TEXT( "Note" ) ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FGroupsAreCheckedIndependently, "SmartTables.MultiSort.GroupsAreCheckedIndependently" );

    struct FTrailingGroupIsNotSkipped : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FDeadSortFixture Fixture;
            Fixture.ColumnIds = { TEXT( "Class" ), TEXT( "Note" ) };

            Fixture.Rows = {
                { TEXT( "Asteroid" ), TEXT( "alone" ) },
                { TEXT( "Booster" ), TEXT( "left" ) },
                { TEXT( "Booster" ), TEXT( "right" ) },
            };

            const TSet< FName > Powerless = Fixture.Powerless( TEXT( "Class" ) );

            AITEST_FALSE( "The final group still counts", Powerless.Contains( TEXT( "Note" ) ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FTrailingGroupIsNotSkipped, "SmartTables.MultiSort.TrailingGroupIsNotSkipped" );

    struct FDegenerateTablesReportEverythingPowerless : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FDeadSortFixture Empty;
            Empty.ColumnIds = { TEXT( "Class" ), TEXT( "Note" ) };

            AITEST_EQUAL( "No rows means nothing can be reordered", Empty.Powerless( TEXT( "Class" ) ).Num(), 1 );

            FDeadSortFixture Single;
            Single.ColumnIds = { TEXT( "Class" ), TEXT( "Note" ) };
            Single.Rows      = { { TEXT( "Asteroid" ), TEXT( "only" ) } };

            AITEST_EQUAL( "One row cannot tie", Single.Powerless( TEXT( "Class" ) ).Num(), 1 );

            FDeadSortFixture NoCandidates;
            NoCandidates.ColumnIds = { TEXT( "Class" ) };
            NoCandidates.Rows      = { { TEXT( "Asteroid" ) }, { TEXT( "Asteroid" ) } };

            AITEST_EQUAL( "A lone column offers no second level", NoCandidates.Powerless( TEXT( "Class" ) ).Num(), 0 );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FDegenerateTablesReportEverythingPowerless, "SmartTables.MultiSort.DegenerateTablesReportEverythingPowerless" );

    static USmartTableTestStructModel * MakeDeadSortModel( bool bCallsignsAreUnique )
    {
        USmartTableTestStructModel * Model = NewObject< USmartTableTestStructModel >();

        const TCHAR * Callsigns[] = { TEXT( "Halcyon-1" ), TEXT( "Kelvin-2" ), TEXT( "Sable-3" ), TEXT( "Ember-4" ) };

        TArray< FSmartTableTestRow > Rows;
        for ( int32 Index = 0; Index < 4; ++Index )
        {
            FSmartTableTestRow & Row = Rows.AddDefaulted_GetRef();

            Row.Callsign = bCallsignsAreUnique ? Callsigns[ Index ] : TEXT( "Halcyon-1" );

            Row.MassTonnes = 100.0 * ( Index + 1 );
        }

        Model->SetRows( MoveTemp( Rows ) );

        return Model;
    }

    struct FThePureGetterReportsRatherThanComputes : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTable * Table = NewObject< USmartTable >();

            FSmartTableColumn Callsign;
            Callsign.ColumnId = TEXT( "Callsign" );

            FSmartTableColumn Mass;
            Mass.ColumnId = TEXT( "MassTonnes" );

            Table->SetColumns( { Callsign, Mass } );
            Table->SetModel( MakeDeadSortModel( true ) );
            Table->SetAllowDeadSecondarySort( false );
            Table->SortByColumn( TEXT( "Callsign" ), ESmartTableSortMode::Ascending );

            AITEST_FALSE( "Sorting alone does not fill the answer in", Table->IsDeadSecondaryColumn( TEXT( "MassTonnes" ) ) );

            Table->RefreshDeadSecondaryColumns();

            AITEST_TRUE( "Refreshing is what fills it in", Table->IsDeadSecondaryColumn( TEXT( "MassTonnes" ) ) );
            AITEST_TRUE( "...and asking twice gives the same answer", Table->IsDeadSecondaryColumn( TEXT( "MassTonnes" ) ) );

            Table->SetModel( MakeDeadSortModel( false ) );
            Table->SortByColumn( TEXT( "Callsign" ), ESmartTableSortMode::Ascending );
            Table->RefreshDeadSecondaryColumns();

            AITEST_FALSE( "A primary that ties frees the second level again", Table->IsDeadSecondaryColumn( TEXT( "MassTonnes" ) ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FThePureGetterReportsRatherThanComputes, "SmartTables.MultiSort.ThePureGetterReportsRatherThanComputes" );

    struct FNewColumnsInvalidateTheDeadSecondaryAnswer : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTable * Table = NewObject< USmartTable >();

            FSmartTableColumn Callsign;
            Callsign.ColumnId = TEXT( "Callsign" );

            FSmartTableColumn Mass;
            Mass.ColumnId = TEXT( "MassTonnes" );

            Table->SetColumns( { Callsign, Mass } );
            Table->SetModel( MakeDeadSortModel( false ) );
            Table->SetAllowDeadSecondarySort( false );
            Table->SortByColumn( TEXT( "Callsign" ), ESmartTableSortMode::Ascending );
            Table->RefreshDeadSecondaryColumns();

            AITEST_FALSE( "MassTonnes can break the tie, so it is not dead", Table->IsDeadSecondaryColumn( TEXT( "MassTonnes" ) ) );

            FSmartTableColumn Registry;
            Registry.ColumnId = TEXT( "Registry" );

            Table->SetColumns( { Callsign, Registry } );
            Table->RefreshDeadSecondaryColumns();

            AITEST_TRUE( "A column the new set added is judged on its own merits", Table->IsDeadSecondaryColumn( TEXT( "Registry" ) ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FNewColumnsInvalidateTheDeadSecondaryAnswer, "SmartTables.MultiSort.NewColumnsInvalidateTheDeadSecondaryAnswer" );

    struct FEveryRouteToASecondLevelMeetsTheSameRefusal : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTable * Table = NewObject< USmartTable >();

            FSmartTableColumn Callsign;
            Callsign.ColumnId = TEXT( "Callsign" );

            FSmartTableColumn Mass;
            Mass.ColumnId = TEXT( "MassTonnes" );

            USmartTableTestStructModel * Model = MakeDeadSortModel( true );

            Table->SetColumns( { Callsign, Mass } );
            Table->SetModel( Model );
            Table->SetAllowDeadSecondarySort( false );
            Table->SortByColumn( TEXT( "Callsign" ), ESmartTableSortMode::Ascending );

            AITEST_EQUAL( "One level to start with", Table->GetSortSpec().Columns.Num(), 1 );

            Table->SortBySecondaryColumn( TEXT( "MassTonnes" ), ESmartTableSortMode::Ascending );

            AITEST_EQUAL( "The function refuses a level that would reorder nothing", Table->GetSortSpec().Columns.Num(), 1 );

            Table->CycleColumnSort( TEXT( "MassTonnes" ), true );

            AITEST_EQUAL( "...and so does the Shift gesture", Table->GetSortSpec().Columns.Num(), 1 );

            TArray< FSmartTableTestRow > Tied;
            for ( int32 Index = 0; Index < 4; ++Index )
            {
                FSmartTableTestRow & Row = Tied.AddDefaulted_GetRef();

                Row.Callsign   = TEXT( "Halcyon-1" );
                Row.MassTonnes = 100.0 * ( Index + 1 );
            }

            Model->SetRows( MoveTemp( Tied ) );

            Table->SortBySecondaryColumn( TEXT( "MassTonnes" ), ESmartTableSortMode::Ascending );

            AITEST_EQUAL( "A level that CAN break the tie is taken", Table->GetSortSpec().Columns.Num(), 2 );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FEveryRouteToASecondLevelMeetsTheSameRefusal, "SmartTables.MultiSort.EveryRouteToASecondLevelMeetsTheSameRefusal" );

}
UE_ENABLE_OPTIMIZATION_SHIP

#undef LOCTEXT_NAMESPACE
#endif
