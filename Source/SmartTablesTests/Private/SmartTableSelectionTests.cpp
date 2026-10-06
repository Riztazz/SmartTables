// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTablesTestsMacros.h"

#if SMARTTABLES_WITH_TESTS

#include "AITestsCommon.h"

#include "Framework/Application/SlateApplication.h"
#include "SmartTableDataTableModel.h"
#include "SmartTableTestTypes.h"

#define LOCTEXT_NAMESPACE "SmartTablesTest"

UE_DISABLE_OPTIMIZATION_SHIP

namespace SmartTable::Test
{
    struct FAFilterThatHidesTheWholeSelectionSaysSo : FAITestBase
    {
        virtual bool InstantTest() override
        {
            AITEST_TRUE( "Slate is up, so a table builds its widget", FSlateApplication::IsInitialized() );

            USmartTableTestHarness * Table    = NamedRowTable( { NamedRow( TEXT( "Vanta" ) ), NamedRow( TEXT( "Garris" ) ), NamedRow( TEXT( "Keller" ) ) } );
            const TSharedRef< SWidget > Built = Table->TakeWidget();

            Table->SetRowsSelected( { 0, 2 }, true );
            AITEST_EQUAL( "Two rows are picked up", Table->GetSelectedRows().Num(), 2 );

            Table->OnSelectionChanged.AddDynamic( Table, &USmartTableTestHarness::HearSelection );

            Table->SetFilterText( FText::FromString( TEXT( "Garris" ) ) );

            AITEST_EQUAL( "A filter that hides both picked rows is one broadcast", Table->SelectionsHeard.Num(), 1 );
            AITEST_TRUE( "...carrying an empty selection", Table->SelectionsHeard[ 0 ].IsEmpty() );
            AITEST_TRUE( "...which is what the table now holds", Table->GetSelectedRows().IsEmpty() );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAFilterThatHidesTheWholeSelectionSaysSo, "SmartTables.Selection.AFilterThatHidesTheWholeSelectionSaysSo" );

    struct FAFilterThatHidesPartOfTheSelectionSaysWhatIsLeft : FAITestBase
    {
        virtual bool InstantTest() override
        {
            AITEST_TRUE( "Slate is up, so a table builds its widget", FSlateApplication::IsInitialized() );

            USmartTableTestHarness * Table    = NamedRowTable( { NamedRow( TEXT( "Vanta" ) ), NamedRow( TEXT( "Garris" ) ), NamedRow( TEXT( "Keller" ) ) } );
            const TSharedRef< SWidget > Built = Table->TakeWidget();

            Table->SetRowsSelected( { 0, 1 }, true );
            AITEST_EQUAL( "Two rows are picked up", Table->GetSelectedRows().Num(), 2 );

            Table->OnSelectionChanged.AddDynamic( Table, &USmartTableTestHarness::HearSelection );

            Table->SetFilterText( FText::FromString( TEXT( "Garris" ) ) );

            AITEST_EQUAL( "A filter that hides one picked row is one broadcast", Table->SelectionsHeard.Num(), 1 );
            AITEST_EQUAL( "...carrying the row still drawn", Table->SelectionsHeard[ 0 ].Num(), 1 );
            AITEST_EQUAL( "...which is Garris", Table->SelectionsHeard[ 0 ][ 0 ], 1 );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAFilterThatHidesPartOfTheSelectionSaysWhatIsLeft, "SmartTables.Selection.AFilterThatHidesPartOfTheSelectionSaysWhatIsLeft" );
}

UE_ENABLE_OPTIMIZATION_SHIP

#undef LOCTEXT_NAMESPACE

#endif
