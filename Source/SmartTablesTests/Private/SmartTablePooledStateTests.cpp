// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTablesTestsMacros.h"

#if SMARTTABLES_WITH_TESTS

#include "AITestsCommon.h"

#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Application/SlateUser.h"
#include "Input/Events.h"
#include "SmartTable.h"
#include "SmartTableCell.h"
#include "SmartTableRowWidget.h"
#include "SmartTableStyle.h"
#include "SmartTableTestTypes.h"

#define LOCTEXT_NAMESPACE "SmartTablesTest"

UE_DISABLE_OPTIMIZATION_SHIP

namespace SmartTable::Test
{
    struct FTheDefaultRowHeightIsOneLineOfTheDefaultCellText : FAITestBase
    {
        virtual bool InstantTest() override
        {
            AITEST_TRUE( "Slate is up, so a font can be measured", FSlateApplication::IsInitialized() );

            const USmartTableStyle * Style = GetDefault< USmartTableStyle >();
            const USmartTable * Table      = GetDefault< USmartTable >();
            AITEST_NOT_NULL( "The style exists", Style );
            AITEST_NOT_NULL( "The table exists", Table );

            const TSharedRef< FSlateFontMeasure > Measure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
            const float Line                              = static_cast< float >( Measure->GetMaxCharacterHeight( Style->CellTextStyle.Font ) );

            AITEST_TRUE( "One line has a height at all", Line > 0.0f );
            AITEST_TRUE( "The default row fits that line", Table->GetRowHeight() >= Line );
            AITEST_TRUE( "And is not tall enough for two", Table->GetRowHeight() < Line * 2.0f );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FTheDefaultRowHeightIsOneLineOfTheDefaultCellText, "SmartTables.Style.TheDefaultRowHeightIsOneLineOfTheDefaultCellText" );

    struct FAWrapperNeverKeepsTheReasonItWasBuiltWith : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTableRowWidget * Wrapper = NewObject< USmartTableTestRowWidget >();
            AITEST_NOT_NULL( "A wrapper builds", Wrapper );

            const ESmartTableAssignReason Every[] = {
                ESmartTableAssignReason::Scrolled,
                ESmartTableAssignReason::Refreshed,
                ESmartTableAssignReason::Added,
                ESmartTableAssignReason::ValueChanged,
            };

            for ( const ESmartTableAssignReason Reason : Every )
            {
                Wrapper->AssignRow( nullptr, 3, Reason );
                AITEST_EQUAL( "The reason handed in is the one it reports", Wrapper->GetAssignReason(), Reason );
            }

            Wrapper->ReleaseRow();
            AITEST_NULL( "Release drops the table", Wrapper->GetTable() );
            AITEST_EQUAL( "And the row it drew", Wrapper->GetRowIndex(), INDEX_NONE );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAWrapperNeverKeepsTheReasonItWasBuiltWith, "SmartTables.Cell.AWrapperNeverKeepsTheReasonItWasBuiltWith" );

    struct FAReleasedCellPointsAtNothing : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTableCell * Cell = NewObject< USmartTableTextCell >();
            AITEST_NOT_NULL( "A cell builds", Cell );

            USmartTable * Table = NewObject< USmartTable >();
            AITEST_NOT_NULL( "A table builds", Table );

            Cell->AssignCell( Table, nullptr, 7, TEXT( "Name" ), ESmartTableAssignReason::Scrolled );
            AITEST_EQUAL( "It holds the table it was given", Cell->GetTable(), Table );
            AITEST_EQUAL( "And the row", Cell->GetRowIndex(), 7 );

            Cell->ReleaseCell();

            AITEST_NULL( "Release drops the table", Cell->GetTable() );
            AITEST_NULL( "Release drops the model", Cell->GetModel() );
            AITEST_EQUAL( "And the row it drew", Cell->GetRowIndex(), INDEX_NONE );
            AITEST_TRUE( "And the column", Cell->GetColumnId().IsNone() );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAReleasedCellPointsAtNothing, "SmartTables.Cell.AReleasedCellPointsAtNothing" );
    struct FAPointerWithNoCursorNeverStartsADrag : FAITestBase
    {
        virtual bool InstantTest() override
        {
            AITEST_TRUE( "Slate is up, so a user can be registered", FSlateApplication::IsInitialized() );

            const TSharedRef< FSlateVirtualUserHandle > Handle = FSlateApplication::Get().FindOrCreateVirtualUser( 7 );

            const TSharedPtr< const FSlateUser > User = FSlateApplication::Get().GetUser( Handle->GetUserIndex() );
            AITEST_TRUE( "The user it registered holds no cursor", User.IsValid() && User->IsVirtualUser() );

            const FPointerEvent FromVirtual( Handle->GetUserIndex(), 0, FVector2D::ZeroVector, FVector2D::ZeroVector, TSet< FKey >(), FKey(), 0.0f, FModifierKeysState() );
            AITEST_FALSE( "So no drag begins from it", USmartTable::CanPointerStartDrag( FromVirtual ) );

            const FPointerEvent FromNobody( 250, 0, FVector2D::ZeroVector, FVector2D::ZeroVector, TSet< FKey >(), FKey(), 0.0f, FModifierKeysState() );
            AITEST_FALSE( "Nor from an index no user answers to", USmartTable::CanPointerStartDrag( FromNobody ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAPointerWithNoCursorNeverStartsADrag, "SmartTables.Input.APointerWithNoCursorNeverStartsADrag" );
    struct FANativeCellNeverAsksTheModelForItsItem : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTableTestCountingModel * CountingModel = NewObject< USmartTableTestCountingModel >();
            USmartTable * Table                          = NewObject< USmartTable >();
            USmartTableCell * Cell                       = NewObject< USmartTableTextCell >();
            AITEST_NOT_NULL( "A model, a table and a cell build", Cell );

            for ( int32 Row = 0; Row < 4; ++Row )
            {
                Cell->AssignCell( Table, CountingModel, Row, TEXT( "Name" ), ESmartTableAssignReason::Scrolled );
                Cell->ReleaseCell();
            }

            AITEST_EQUAL( "Four rows drawn by a cell with no graph, and the model was never asked for one item", CountingModel->ItemsAsked, 0 );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FANativeCellNeverAsksTheModelForItsItem, "SmartTables.Cell.ANativeCellNeverAsksTheModelForItsItem" );

    struct FEveryRowIdIsOneNameAndANumber : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTableModel * PlainModel = NewObject< USmartTableTestCountingModel >();
            AITEST_NOT_NULL( "A model that leaves GetRowId to the base builds", PlainModel );

            const FName First  = PlainModel->GetRowId( 0 );
            const FName Second = PlainModel->GetRowId( 1 );
            const FName Far    = PlainModel->GetRowId( 999999 );

            AITEST_NOT_EQUAL( "Two rows answer two ids", First, Second );
            AITEST_NOT_EQUAL( "However far apart", First, Far );

            AITEST_EQUAL( "And all three are the same name", First.GetPlainNameString(), Second.GetPlainNameString() );
            AITEST_EQUAL( "...all three", First.GetPlainNameString(), Far.GetPlainNameString() );
            AITEST_NOT_EQUAL( "Told apart by the number they carry", First.GetNumber(), Second.GetNumber() );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FEveryRowIdIsOneNameAndANumber, "SmartTables.Model.EveryRowIdIsOneNameAndANumber" );

}
UE_ENABLE_OPTIMIZATION_SHIP

#undef LOCTEXT_NAMESPACE

#endif
