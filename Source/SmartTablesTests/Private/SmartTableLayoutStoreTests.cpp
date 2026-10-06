// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTablesTestsMacros.h"

#if SMARTTABLES_WITH_TESTS

#include "AITestsCommon.h"

#include "Misc/ConfigCacheIni.h"
#include "SmartTable.h"
#include "SmartTableDataTableModel.h"
#include "SmartTableLayoutStore.h"
#include "SmartTableObjectModel.h"

#define LOCTEXT_NAMESPACE "SmartTablesTest"

UE_DISABLE_OPTIMIZATION_SHIP

namespace SmartTable::Test
{
    static const TCHAR * StoreTestTableId = TEXT( "SmartTablesAutomationTable" );

    static FSmartTableLayout LayoutWorthStoring()
    {
        FSmartTableLayout Layout;

        FSmartTableColumnLayout & Dragged = Layout.Columns.AddDefaulted_GetRef();
        Dragged.ColumnId                  = TEXT( "Notes" );
        Dragged.Width                     = 317.5f;
        Dragged.bUserWidth                = true;

        FSmartTableColumnLayout & Hidden = Layout.Columns.AddDefaulted_GetRef();
        Hidden.ColumnId                  = TEXT( "DeltaV" );
        Hidden.bHidden                   = true;

        FSmartTableSortColumn & Sort = Layout.SortSpec.Columns.AddDefaulted_GetRef();
        Sort.ColumnId                = TEXT( "MassTonnes" );
        Sort.Mode                    = ESmartTableSortMode::Descending;

        return Layout;
    }

    static void ForgetStoredLayout()
    {
        GConfig->RemoveKey( TEXT( "SmartTables.Layouts" ), StoreTestTableId, GGameUserSettingsIni );
        GConfig->Flush( false, GGameUserSettingsIni );
    }

    static USmartTableDataTableRow * LayoutTestRow( const TCHAR * Name )
    {
        USmartTableDataTableRow * Row = NewObject< USmartTableDataTableRow >();
        Row->RowName                  = Name;

        return Row;
    }

    struct FNothingIsStoredUntilSomethingStoresIt : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTableLayoutStore * Store = GetMutableDefault< USmartTableLayoutStore >();

            FSmartTableLayout Loaded;
            AITEST_FALSE( "The base store stores nothing", Store->LoadLayout( TEXT( "Anything" ), Loaded ) );

            Store->SaveLayout( TEXT( "Anything" ), LayoutWorthStoring() );
            AITEST_FALSE( "...and saving to it still stores nothing", Store->LoadLayout( TEXT( "Anything" ), Loaded ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FNothingIsStoredUntilSomethingStoresIt, "SmartTables.LayoutStore.NothingIsStoredUntilSomethingStoresIt" );

    struct FConfigStoreRoundTripsEveryFieldTheUserChanged : FAITestBase
    {
        virtual bool InstantTest() override
        {
            ForgetStoredLayout();

            USmartTableConfigLayoutStore * Store = NewObject< USmartTableConfigLayoutStore >();

            FSmartTableLayout Loaded;
            AITEST_FALSE( "An unknown table has nothing stored", Store->LoadLayout( StoreTestTableId, Loaded ) );

            Store->SaveLayout( StoreTestTableId, LayoutWorthStoring() );

            FSmartTableLayout Restored;
            AITEST_TRUE( "A stored layout loads", Store->LoadLayout( StoreTestTableId, Restored ) );
            AITEST_EQUAL( "Both columns came back", Restored.Columns.Num(), 2 );

            AITEST_EQUAL( "...the first by id", Restored.Columns[ 0 ].ColumnId.ToString(), FString( TEXT( "Notes" ) ) );
            AITEST_EQUAL( "...with its width", Restored.Columns[ 0 ].Width, 317.5f );

            AITEST_TRUE( "...and knowing the user chose it", Restored.Columns[ 0 ].bUserWidth );

            AITEST_EQUAL( "...the second by id", Restored.Columns[ 1 ].ColumnId.ToString(), FString( TEXT( "DeltaV" ) ) );
            AITEST_TRUE( "...still hidden", Restored.Columns[ 1 ].bHidden );
            AITEST_FALSE( "...and still the table's to size", Restored.Columns[ 1 ].bUserWidth );

            AITEST_EQUAL( "The sort came back too", Restored.SortSpec.Columns.Num(), 1 );
            AITEST_TRUE( "...pointing at the same column", Restored.SortSpec.GetModeFor( TEXT( "MassTonnes" ) ) == ESmartTableSortMode::Descending );

            ForgetStoredLayout();
            AITEST_FALSE( "Removing the entry removes the layout", Store->LoadLayout( StoreTestTableId, Loaded ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FConfigStoreRoundTripsEveryFieldTheUserChanged, "SmartTables.LayoutStore.ConfigStoreRoundTripsEveryFieldTheUserChanged" );

    struct FStoredLayoutsAreKeyedByTable : FAITestBase
    {
        virtual bool InstantTest() override
        {
            ForgetStoredLayout();

            USmartTableConfigLayoutStore * Store = NewObject< USmartTableConfigLayoutStore >();
            Store->SaveLayout( StoreTestTableId, LayoutWorthStoring() );

            FSmartTableLayout Other;
            AITEST_FALSE( "A different table id reads nothing", Store->LoadLayout( TEXT( "SmartTablesAutomationOtherTable" ), Other ) );

            ForgetStoredLayout();

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FStoredLayoutsAreKeyedByTable, "SmartTables.LayoutStore.StoredLayoutsAreKeyedByTable" );

    struct FAVersionZeroLayoutWakesWithoutItsStoredWidths : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FSmartTableLayout Old = LayoutWorthStoring();
            Old.Version           = 0;

            AITEST_TRUE( "An old layout is migrated", SmartTable::MigrateStoredLayout( Old ) );
            AITEST_EQUAL( "...and says so afterwards", Old.Version, FSmartTableLayout::CurrentVersion );

            AITEST_EQUAL( "The width is gone", Old.Columns[ 0 ].Width, 0.0f );
            AITEST_FALSE( "...and no longer claims the user chose it", Old.Columns[ 0 ].bUserWidth );

            AITEST_EQUAL( "Hiding survives", Old.Columns.Num(), 2 );
            AITEST_TRUE( "...on the column that had it", Old.Columns[ 1 ].bHidden );
            AITEST_EQUAL( "...in the order it was stored", Old.Columns[ 1 ].ColumnId.ToString(), FString( TEXT( "DeltaV" ) ) );
            AITEST_TRUE( "The sort survives", Old.SortSpec.GetModeFor( TEXT( "MassTonnes" ) ) == ESmartTableSortMode::Descending );

            FSmartTableLayout Current = LayoutWorthStoring();
            Current.Version           = FSmartTableLayout::CurrentVersion;

            AITEST_FALSE( "A current layout needs no migration", SmartTable::MigrateStoredLayout( Current ) );
            AITEST_EQUAL( "...and keeps its width", Current.Columns[ 0 ].Width, 317.5f );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAVersionZeroLayoutWakesWithoutItsStoredWidths, "SmartTables.LayoutStore.AVersionZeroLayoutWakesWithoutItsStoredWidths" );

    struct FASavedLayoutCarriesTheCurrentVersion : FAITestBase
    {
        virtual bool InstantTest() override
        {
            ForgetStoredLayout();

            USmartTableConfigLayoutStore * Store = NewObject< USmartTableConfigLayoutStore >();

            FSmartTableLayout Saved = LayoutWorthStoring();
            Saved.Version           = FSmartTableLayout::CurrentVersion;
            Store->SaveLayout( StoreTestTableId, Saved );

            FSmartTableLayout Restored;
            AITEST_TRUE( "It loads", Store->LoadLayout( StoreTestTableId, Restored ) );
            AITEST_EQUAL( "...at the version it was written at", Restored.Version, FSmartTableLayout::CurrentVersion );
            AITEST_FALSE( "...so it is not migrated again", SmartTable::MigrateStoredLayout( Restored ) );
            AITEST_EQUAL( "...and keeps the width", Restored.Columns[ 0 ].Width, 317.5f );

            ForgetStoredLayout();

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FASavedLayoutCarriesTheCurrentVersion, "SmartTables.LayoutStore.ASavedLayoutCarriesTheCurrentVersion" );

    struct FASortIsPartOfTheSavedLayout : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTable * Table = NewObject< USmartTable >();

            USmartTableObjectModel * Model = NewObject< USmartTableObjectModel >();
            Model->SetItems( { LayoutTestRow( TEXT( "Charlie" ) ), LayoutTestRow( TEXT( "Alpha" ) ) } );

            FSmartTableColumn Column;
            Column.ColumnId = TEXT( "Name" );
            Table->SetColumns( { Column } );
            Table->SetModel( Model );

            AITEST_TRUE( "A fresh table has no sort in its layout", Table->GetLayout().SortSpec.IsEmpty() );

            Table->SortByColumn( TEXT( "Name" ), ESmartTableSortMode::Ascending );

            const FSmartTableLayout Sorted = Table->GetLayout();
            AITEST_FALSE( "Sorting puts a spec in the layout", Sorted.SortSpec.IsEmpty() );
            AITEST_EQUAL( "...naming the column that was sorted", Sorted.SortSpec.Columns[ 0 ].ColumnId, FName( TEXT( "Name" ) ) );

            Table->SortByColumn( TEXT( "Name" ), ESmartTableSortMode::None );
            AITEST_TRUE( "Clearing the sort clears it from the layout too", Table->GetLayout().SortSpec.IsEmpty() );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FASortIsPartOfTheSavedLayout, "SmartTables.LayoutStore.ASortIsPartOfTheSavedLayout" );

    struct FAStoreArrivingAfterTheIdStillRestores : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTableConfigLayoutStore * Store = NewObject< USmartTableConfigLayoutStore >();

            FSmartTableLayout Saved;
            FSmartTableSortColumn & Level = Saved.SortSpec.Columns.AddDefaulted_GetRef();
            Level.ColumnId                = TEXT( "Name" );
            Level.Mode                    = ESmartTableSortMode::Descending;
            Store->SaveLayout( TEXT( "StoreAfterId" ), Saved );

            USmartTable * Table = NewObject< USmartTable >();

            FSmartTableColumn Column;
            Column.ColumnId = TEXT( "Name" );
            Table->SetColumns( { Column } );

            USmartTableObjectModel * Model = NewObject< USmartTableObjectModel >();
            Model->SetItems( { LayoutTestRow( TEXT( "Charlie" ) ), LayoutTestRow( TEXT( "Alpha" ) ) } );
            Table->SetModel( Model );

            Table->SetTableId( TEXT( "StoreAfterId" ) );
            AITEST_TRUE( "With no store yet, nothing is restored", Table->GetLayout().SortSpec.IsEmpty() );

            Table->SetLayoutStore( Store );

            const FSmartTableLayout Restored = Table->GetLayout();
            AITEST_FALSE( "Assigning the store restores the stored layout", Restored.SortSpec.IsEmpty() );
            AITEST_EQUAL( "...including which column it sorted", Restored.SortSpec.Columns[ 0 ].ColumnId, FName( TEXT( "Name" ) ) );
            AITEST_EQUAL( "...and which way", Restored.SortSpec.Columns[ 0 ].Mode, ESmartTableSortMode::Descending );

            Store->SaveLayout( TEXT( "StoreAfterId" ), FSmartTableLayout() );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAStoreArrivingAfterTheIdStillRestores, "SmartTables.LayoutStore.AStoreArrivingAfterTheIdStillRestores" );

    struct FARestoredSortSurvivesTheModelArriving : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTableConfigLayoutStore * Store = NewObject< USmartTableConfigLayoutStore >();

            FSmartTableLayout Saved;
            FSmartTableSortColumn & Level = Saved.SortSpec.Columns.AddDefaulted_GetRef();
            Level.ColumnId                = TEXT( "Name" );
            Level.Mode                    = ESmartTableSortMode::Descending;
            Store->SaveLayout( TEXT( "ModelAfterStore" ), Saved );

            USmartTable * Table = NewObject< USmartTable >();

            FSmartTableColumn Column;
            Column.ColumnId = TEXT( "Name" );
            Table->SetColumns( { Column } );

            Table->SetTableId( TEXT( "ModelAfterStore" ) );
            Table->SetLayoutStore( Store );

            AITEST_FALSE( "The layout holds the stored sort with no model yet", Table->GetLayout().SortSpec.IsEmpty() );
            AITEST_TRUE( "...and nothing is sorted, because there is nothing to sort", Table->GetSortSpec().IsEmpty() );

            USmartTableObjectModel * Model = NewObject< USmartTableObjectModel >();
            Model->SetItems( { LayoutTestRow( TEXT( "Charlie" ) ), LayoutTestRow( TEXT( "Alpha" ) ) } );
            Table->SetModel( Model );

            const FSmartTableSortSpec Live = Table->GetSortSpec();
            AITEST_FALSE( "The model arriving takes the sort the layout was holding", Live.IsEmpty() );
            AITEST_EQUAL( "...the column it was filed under", Live.Columns[ 0 ].ColumnId, FName( TEXT( "Name" ) ) );
            AITEST_EQUAL( "...the way round it was filed", Live.Columns[ 0 ].Mode, ESmartTableSortMode::Descending );

            Store->SaveLayout( TEXT( "ModelAfterStore" ), FSmartTableLayout() );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FARestoredSortSurvivesTheModelArriving, "SmartTables.LayoutStore.ARestoredSortSurvivesTheModelArriving" );

    struct FAnIdWithNothingFiledUnderItKeepsTheSortInHand : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTable * Table = NewObject< USmartTable >();

            FSmartTableColumn Column;
            Column.ColumnId = TEXT( "Name" );
            Table->SetColumns( { Column } );

            USmartTableObjectModel * Model = NewObject< USmartTableObjectModel >();
            Model->SetItems( { LayoutTestRow( TEXT( "Charlie" ) ), LayoutTestRow( TEXT( "Alpha" ) ) } );
            Table->SetModel( Model );

            Table->SortByColumn( TEXT( "Name" ), ESmartTableSortMode::Descending );
            AITEST_FALSE( "The table is sorted from code", Table->GetSortSpec().IsEmpty() );

            Table->SetTableId( TEXT( "NothingFiledHere" ) );

            const FSmartTableSortSpec Live = Table->GetSortSpec();
            AITEST_FALSE( "Giving it an id with no store behind it keeps that sort", Live.IsEmpty() );
            AITEST_EQUAL( "...the same column", Live.Columns[ 0 ].ColumnId, FName( TEXT( "Name" ) ) );
            AITEST_EQUAL( "...the same way round", Live.Columns[ 0 ].Mode, ESmartTableSortMode::Descending );

            AITEST_FALSE( "...and the layout says what the rows say", Table->GetLayout().SortSpec.IsEmpty() );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAnIdWithNothingFiledUnderItKeepsTheSortInHand, "SmartTables.LayoutStore.AnIdWithNothingFiledUnderItKeepsTheSortInHand" );
}
UE_ENABLE_OPTIMIZATION_SHIP

#undef LOCTEXT_NAMESPACE
#endif
