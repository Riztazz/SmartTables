// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTablesTestsMacros.h"

#if SMARTTABLES_WITH_TESTS

#include "AITestsCommon.h"

#include "Table/SmartTableColumnView.h"

#define LOCTEXT_NAMESPACE "SmartTablesTest"

UE_DISABLE_OPTIMIZATION_SHIP

namespace SmartTable::Test
{
    static FSmartTableColumn ViewColumn( const TCHAR * ColumnId, ESmartTableColumnSizing Sizing, float Width, bool bHiddenByDefault = false )
    {
        FSmartTableColumn Column;
        Column.ColumnId         = ColumnId;
        Column.Sizing           = Sizing;
        Column.Width            = Width;
        Column.bHiddenByDefault = bHiddenByDefault;

        return Column;
    }

    static FSmartTableColumnLayout ViewOverride( const TCHAR * ColumnId, bool bHidden )
    {
        FSmartTableColumnLayout Override;
        Override.ColumnId = ColumnId;
        Override.bHidden  = bHidden;

        return Override;
    }

    static FString ViewIdsAsText( const TArray< FName > & Ids )
    {
        return FString::JoinBy( Ids, TEXT( "," ), []( FName Id )
        {
            return Id.ToString();
        } );
    }

    struct FAnOverrideDecidesWhetherAColumnIsShown : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TArray< FSmartTableColumn > Columns = { ViewColumn( TEXT( "Name" ), ESmartTableColumnSizing::Fill, 1.0f ), ViewColumn( TEXT( "Mass" ), ESmartTableColumnSizing::Fixed, 80.0f, true ) };

            FSmartTableLayout Layout;
            Layout.Columns = { ViewOverride( TEXT( "Name" ), true ), ViewOverride( TEXT( "Mass" ), false ) };

            const TMap< FName, float > Resolved;
            const FColumnView View( Columns, Layout, Resolved );

            AITEST_FALSE( "A column the layout hides does not draw, however it was authored", View.IsShown( TEXT( "Name" ) ) );
            AITEST_TRUE( "A column hidden by default and shown by the layout draws", View.IsShown( TEXT( "Mass" ) ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAnOverrideDecidesWhetherAColumnIsShown, "SmartTables.ColumnView.AnOverrideDecidesWhetherAColumnIsShown" );

    struct FAColumnWithNoOverrideIsShownAsAuthored : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TArray< FSmartTableColumn > Columns = { ViewColumn( TEXT( "Name" ), ESmartTableColumnSizing::Fill, 1.0f ), ViewColumn( TEXT( "Mass" ), ESmartTableColumnSizing::Fixed, 80.0f, true ) };

            const FSmartTableLayout Layout;
            const TMap< FName, float > Resolved;
            const FColumnView View( Columns, Layout, Resolved );

            AITEST_TRUE( "A column authored as shown draws", View.IsShown( TEXT( "Name" ) ) );
            AITEST_FALSE( "A column authored as hidden does not", View.IsShown( TEXT( "Mass" ) ) );
            AITEST_FALSE( "An id no column has is never shown", View.IsShown( TEXT( "Nobody" ) ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAColumnWithNoOverrideIsShownAsAuthored, "SmartTables.ColumnView.AColumnWithNoOverrideIsShownAsAuthored" );

    struct FTheShownOrderFollowsTheStoredOrderAndDropsHiddenColumns : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TArray< FSmartTableColumn > Columns = { ViewColumn( TEXT( "A" ), ESmartTableColumnSizing::Fill, 1.0f ), ViewColumn( TEXT( "B" ), ESmartTableColumnSizing::Fill, 1.0f ), ViewColumn( TEXT( "C" ), ESmartTableColumnSizing::Fill, 1.0f ), ViewColumn( TEXT( "D" ), ESmartTableColumnSizing::Fill, 1.0f ) };

            FSmartTableLayout Layout;
            Layout.Order   = { TEXT( "C" ), TEXT( "A" ) };
            Layout.Columns = { ViewOverride( TEXT( "B" ), true ) };

            const TMap< FName, float > Resolved;
            const FColumnView View( Columns, Layout, Resolved );

            AITEST_EQUAL( "The authored ids keep the authored order", ViewIdsAsText( View.AuthoredIds() ), FString( TEXT( "A,B,C,D" ) ) );
            AITEST_EQUAL( "The stored order leads and the rest follow as authored", ViewIdsAsText( View.Order() ), FString( TEXT( "C,A,B,D" ) ) );
            AITEST_EQUAL( "The shown order is that, less the hidden column", ViewIdsAsText( View.ShownOrder() ), FString( TEXT( "C,A,D" ) ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FTheShownOrderFollowsTheStoredOrderAndDropsHiddenColumns, "SmartTables.ColumnView.TheShownOrderFollowsTheStoredOrderAndDropsHiddenColumns" );

    struct FAFillColumnHasNoWidthUntilAPassResolvesOne : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TArray< FSmartTableColumn > Columns = { ViewColumn( TEXT( "Name" ), ESmartTableColumnSizing::Fill, 1.0f ), ViewColumn( TEXT( "Mass" ), ESmartTableColumnSizing::Fixed, 80.0f ) };

            const FSmartTableLayout Layout;
            TMap< FName, float > Resolved;
            const FColumnView View( Columns, Layout, Resolved );

            AITEST_EQUAL( "A Fill column has no pixel width before a pass", View.CurrentWidth( TEXT( "Name" ) ), 0.0f );
            AITEST_EQUAL( "A Fixed column has its authored pixels", View.CurrentWidth( TEXT( "Mass" ) ), 80.0f );
            AITEST_EQUAL( "An id no column has is zero wide", View.CurrentWidth( TEXT( "Nobody" ) ), 0.0f );

            Resolved.Add( TEXT( "Name" ), 240.0f );
            AITEST_EQUAL( "Once a pass resolves it, the Fill column has that width", View.CurrentWidth( TEXT( "Name" ) ), 240.0f );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAFillColumnHasNoWidthUntilAPassResolvesOne, "SmartTables.ColumnView.AFillColumnHasNoWidthUntilAPassResolvesOne" );

    struct FAChosenWidthIsWhatTheMetricsAskFor : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TArray< FSmartTableColumn > Columns = { ViewColumn( TEXT( "Name" ), ESmartTableColumnSizing::Fill, 1.0f ), ViewColumn( TEXT( "Mass" ), ESmartTableColumnSizing::Fixed, 80.0f ) };

            FSmartTableLayout Layout;
            FSmartTableColumnLayout & Chosen = Layout.Columns.AddDefaulted_GetRef();
            Chosen.ColumnId                  = TEXT( "Mass" );
            Chosen.bUserWidth                = true;
            Chosen.Width                     = 120.0f;

            const TMap< FName, float > Resolved;
            const FColumnView View( Columns, Layout, Resolved );

            const TArray< ColumnLayout::FColumn > Metrics = View.Metrics();
            AITEST_EQUAL( "Every column has a metric", Metrics.Num(), 2 );
            AITEST_FALSE( "A column nobody sized asks for nothing", Metrics[ 0 ].UserWidth.IsSet() );
            AITEST_TRUE( "A chosen width is asked for", Metrics[ 1 ].UserWidth.IsSet() );
            AITEST_EQUAL( "...at the width that was chosen", Metrics[ 1 ].UserWidth.GetValue(), 120.0f );
            AITEST_EQUAL( "...and it is the width in force before any pass", Metrics[ 1 ].CurrentWidth, 120.0f );

            const TArray< ColumnLayout::FColumn > Shown = View.ShownMetrics();
            AITEST_EQUAL( "Both columns draw", Shown.Num(), 2 );
            AITEST_EQUAL( "...in the order they draw", Shown[ 0 ].ColumnId, FName( TEXT( "Name" ) ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAChosenWidthIsWhatTheMetricsAskFor, "SmartTables.ColumnView.AChosenWidthIsWhatTheMetricsAskFor" );
}

UE_ENABLE_OPTIMIZATION_SHIP

#undef LOCTEXT_NAMESPACE

#endif
