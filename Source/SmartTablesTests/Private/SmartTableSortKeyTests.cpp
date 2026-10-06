// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTablesTestsMacros.h"

#if SMARTTABLES_WITH_TESTS

#include "AITestsCommon.h"

#include "SmartTableSortKey.h"
#include "SmartTableTypes.h"

#define LOCTEXT_NAMESPACE "SmartTablesTest"

UE_DISABLE_OPTIMIZATION_SHIP

namespace SmartTable::Test
{
    struct FSortKeyFactoriesSayWhatTheyHold : FAITestBase
    {
        virtual bool InstantTest() override
        {
            AITEST_TRUE( "A default-constructed key is empty", FSmartTableSortKey().IsEmpty() );
            AITEST_TRUE( "...and so is the one that says so", FSmartTableSortKey::MakeEmpty().IsEmpty() );

            const FSmartTableSortKey Number = FSmartTableSortKey::MakeNumber( 2.5 );
            AITEST_TRUE( "A number is numeric", Number.Kind == ESmartTableSortKeyKind::Numeric );
            AITEST_FALSE( "...and is not empty", Number.IsEmpty() );
            AITEST_EQUAL( "...and keeps its value", Number.Number, 2.5 );

            const FSmartTableSortKey False = FSmartTableSortKey::MakeBool( false );
            AITEST_TRUE( "A bool is stored as a number", False.Kind == ESmartTableSortKeyKind::Numeric );
            AITEST_TRUE( "false sorts below true", False.Compare( FSmartTableSortKey::MakeBool( true ) ) < 0 );

            const FSmartTableSortKey Text = FSmartTableSortKey::MakeText( TEXT( "Vanta" ) );
            AITEST_TRUE( "Text is a string", Text.Kind == ESmartTableSortKeyKind::String );
            AITEST_EQUAL( "...and keeps it", Text.Text, FString( TEXT( "Vanta" ) ) );
            AITEST_FALSE( "A non-empty string is not an empty key", Text.IsEmpty() );

            AITEST_FALSE( "An empty string is still an answer", FSmartTableSortKey::MakeText( FString() ).IsEmpty() );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FSortKeyFactoriesSayWhatTheyHold, "SmartTables.SortKey.FactoriesSayWhatTheyHold" );

    struct FSortKeyOrdersTheKindsSoTheOrderIsTotal : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const FSmartTableSortKey Small = FSmartTableSortKey::MakeNumber( 1.25 );
            const FSmartTableSortKey Large = FSmartTableSortKey::MakeNumber( 1.5 );

            AITEST_TRUE( "Numbers order numerically", Small.Compare( Large ) < 0 );
            AITEST_TRUE( "...both ways", Large.Compare( Small ) > 0 );
            AITEST_EQUAL( "...and equal numbers tie", Small.Compare( FSmartTableSortKey::MakeNumber( 1.25 ) ), 0 );

            const FSmartTableSortKey Two = FSmartTableSortKey::MakeText( TEXT( "Asteroid #2" ) );
            const FSmartTableSortKey Ten = FSmartTableSortKey::MakeText( TEXT( "Asteroid #10" ) );
            AITEST_TRUE( "Strings order naturally", Two.Compare( Ten ) < 0 );

            AITEST_TRUE( "A number sorts ahead of a string", Small.Compare( Two ) < 0 );
            AITEST_TRUE( "...and a string after a number", Two.Compare( Small ) > 0 );

            AITEST_TRUE( "Which is a total order, so the three of them cannot disagree", Small.Compare( Large ) < 0 && Large.Compare( Two ) < 0 && Small.Compare( Two ) < 0 );

            AITEST_EQUAL( "An empty key ties with another empty one", FSmartTableSortKey::MakeEmpty().Compare( FSmartTableSortKey::MakeEmpty() ), 0 );

            AITEST_TRUE( "Compare knows nothing about the empty rule, which the sort applies before asking", FSmartTableSortKey::MakeEmpty().Compare( Large ) < 0 );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FSortKeyOrdersTheKindsSoTheOrderIsTotal, "SmartTables.SortKey.OrdersTheKindsSoTheOrderIsTotal" );

    struct FSortSpecAnswersWhatTheHeaderAsks : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FSmartTableSortSpec Spec;
            AITEST_TRUE( "A fresh spec is unsorted", Spec.IsEmpty() );
            AITEST_TRUE( "...and sorts nothing", Spec.GetModeFor( TEXT( "Mass" ) ) == ESmartTableSortMode::None );
            AITEST_EQUAL( "...at no priority", Spec.GetPriorityFor( TEXT( "Mass" ) ), INDEX_NONE );

            FSmartTableSortColumn & Primary = Spec.Columns.AddDefaulted_GetRef();
            Primary.ColumnId                = TEXT( "Mass" );
            Primary.Mode                    = ESmartTableSortMode::Descending;

            FSmartTableSortColumn & Secondary = Spec.Columns.AddDefaulted_GetRef();
            Secondary.ColumnId                = TEXT( "Callsign" );
            Secondary.Mode                    = ESmartTableSortMode::Ascending;

            AITEST_FALSE( "A spec with levels is not empty", Spec.IsEmpty() );
            AITEST_TRUE( "The primary reports its mode", Spec.GetModeFor( TEXT( "Mass" ) ) == ESmartTableSortMode::Descending );
            AITEST_TRUE( "...and so does the secondary", Spec.GetModeFor( TEXT( "Callsign" ) ) == ESmartTableSortMode::Ascending );
            AITEST_TRUE( "A column nobody sorted by has no mode", Spec.GetModeFor( TEXT( "Owner" ) ) == ESmartTableSortMode::None );

            AITEST_EQUAL( "The primary is priority 0", Spec.GetPriorityFor( TEXT( "Mass" ) ), 0 );
            AITEST_EQUAL( "The secondary is priority 1", Spec.GetPriorityFor( TEXT( "Callsign" ) ), 1 );
            AITEST_EQUAL( "An unsorted column has none", Spec.GetPriorityFor( TEXT( "Owner" ) ), INDEX_NONE );

            FSmartTableSortSpec Same = Spec;
            AITEST_TRUE( "An identical spec compares equal", Same == Spec );

            Same.Columns[ 0 ].Mode = ESmartTableSortMode::Ascending;
            AITEST_FALSE( "A flipped direction is a different question", Same == Spec );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FSortSpecAnswersWhatTheHeaderAsks, "SmartTables.SortSpec.AnswersWhatTheHeaderAsks" );

    struct FColumnFallsBackToItsOwnId : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FSmartTableColumn Column;
            Column.ColumnId = TEXT( "DisplayName" );

            AITEST_TRUE( "An unbound column binds to its own id", Column.GetValueBinding() == FName( TEXT( "DisplayName" ) ) );
            AITEST_TRUE( "...and sorts by the same thing", Column.GetSortBinding() == FName( TEXT( "DisplayName" ) ) );

            Column.BindingName = TEXT( "Callsign" );
            AITEST_TRUE( "A binding name wins over the id", Column.GetValueBinding() == FName( TEXT( "Callsign" ) ) );
            AITEST_TRUE( "...and the sort follows the value", Column.GetSortBinding() == FName( TEXT( "Callsign" ) ) );

            Column.SortBindingName = TEXT( "DistanceKm" );
            AITEST_TRUE( "The value binding is unchanged", Column.GetValueBinding() == FName( TEXT( "Callsign" ) ) );
            AITEST_TRUE( "...and the sort binding takes over", Column.GetSortBinding() == FName( TEXT( "DistanceKm" ) ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FColumnFallsBackToItsOwnId, "SmartTables.Column.FallsBackToItsOwnId" );
}
UE_ENABLE_OPTIMIZATION_SHIP

#undef LOCTEXT_NAMESPACE
#endif
