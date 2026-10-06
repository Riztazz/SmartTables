// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTablesTestsMacros.h"

#if SMARTTABLES_WITH_TESTS

#include "AITestsCommon.h"

#include "Framework/Application/SlateApplication.h"
#include "SmartTable.h"
#include "SmartTableDataTableModel.h"
#include "SmartTableOpenMenu.h"
#include "SmartTableStyle.h"
#include "SmartTableTestTypes.h"
#include "SmartTableTypes.h"
#include "Styling/ISlateStyle.h"
#include "Styling/SlateBrush.h"
#include "Styling/SlateColor.h"
#include "Styling/SlateTypes.h"

#define LOCTEXT_NAMESPACE "SmartTablesTest"

UE_DISABLE_OPTIMIZATION_SHIP

namespace SmartTable::Test
{
    static FSmartTableMenuStyle MenuStyleTinted( const FLinearColor & Tint )
    {
        FSmartTableMenuStyle Style;
        Style.BackgroundBrush.TintColor = FSlateColor( Tint );

        return Style;
    }

    static FLinearColor BackgroundTintOf( const ISlateStyle & Style )
    {
        return Style.GetBrush( TEXT( "Menu.Background" ) )->TintColor.GetSpecifiedColor();
    }

    static const FLinearColor MarkedLabelColour( 0.8f, 0.6f, 0.4f );

    static FSmartTableMenuStyle MenuStyleMarking( bool bFill, bool bOutline )
    {
        FSmartTableMenuStyle Style;
        Style.LabelTextStyle.SetColorAndOpacity( FSlateColor( MarkedLabelColour ) );
        Style.SearchHighlight.bFill    = bFill;
        Style.SearchHighlight.bOutline = bOutline;

        return Style;
    }

    static FSlateBrush SearchMarkOf( const FSmartTableMenuStyle & Style )
    {
        FSmartTableOpenMenu Host;

        return Host.Style( Style, TEXT( "SmartTableMenu.Test" ) ).GetWidgetStyle< FTextBlockStyle >( TEXT( "Menu.Label" ) ).HighlightShape;
    }

    struct FTheStyleSetIsBuiltOnceAndDroppedWhenTheStyleMoves : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FSmartTableOpenMenu Host;

            const FSmartTableMenuStyle Red  = MenuStyleTinted( FLinearColor::Red );
            const FSmartTableMenuStyle Blue = MenuStyleTinted( FLinearColor::Blue );

            AITEST_TRUE( "The first call builds the set from the style it is given", BackgroundTintOf( Host.Style( Red, TEXT( "SmartTableMenu.Test" ) ) ).Equals( FLinearColor::Red ) );

            AITEST_TRUE( "A second call keeps it, since an FMenuBuilder holds a plain pointer to it", BackgroundTintOf( Host.Style( Blue, TEXT( "SmartTableMenu.Test" ) ) ).Equals( FLinearColor::Red ) );

            Host.DropStyle();

            AITEST_TRUE( "A style that moved builds a new set", BackgroundTintOf( Host.Style( Blue, TEXT( "SmartTableMenu.Test" ) ) ).Equals( FLinearColor::Blue ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FTheStyleSetIsBuiltOnceAndDroppedWhenTheStyleMoves, "SmartTables.OpenMenu.TheStyleSetIsBuiltOnceAndDroppedWhenTheStyleMoves" );

    struct FDismissingNothingIsSafeAndRepeatable : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FSmartTableOpenMenu Host;

            Host.Dismiss();
            Host.Dismiss();
            Host.DropStyle();

            const FSmartTableMenuStyle Named;
            AITEST_TRUE( "A host nothing opened still builds its style", &Host.Style( Named, TEXT( "SmartTableMenu.Test" ) ) != nullptr );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FDismissingNothingIsSafeAndRepeatable, "SmartTables.OpenMenu.DismissingNothingIsSafeAndRepeatable" );

    struct FASearchMatchIsMarkedInTheLabelColour : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const FSlateBrush Mark = SearchMarkOf( MenuStyleMarking( true, true ) );

            const FLinearColor Fill    = Mark.TintColor.GetSpecifiedColor();
            const FLinearColor Outline = Mark.OutlineSettings.Color.GetSpecifiedColor();

            AITEST_TRUE( "Both parts on draw a rounded box", Mark.DrawAs == ESlateBrushDrawType::RoundedBox );
            AITEST_TRUE( "The fill is the label colour", Fill.CopyWithNewOpacity( 1.0f ).Equals( MarkedLabelColour ) );
            AITEST_TRUE( "...and see through, so the letters under it still read", Fill.A > 0.0f && Fill.A < 1.0f );
            AITEST_TRUE( "The outline is the label colour", Outline.CopyWithNewOpacity( 1.0f ).Equals( MarkedLabelColour ) );
            AITEST_TRUE( "...and it draws", Mark.OutlineSettings.Width > 0.0f && Outline.A > 0.0f );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FASearchMatchIsMarkedInTheLabelColour, "SmartTables.OpenMenu.ASearchMatchIsMarkedInTheLabelColour" );

    struct FEachMarkPartDrawsLighterWhileTheOtherIsOn : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const FSlateBrush Both        = SearchMarkOf( MenuStyleMarking( true, true ) );
            const FSlateBrush FillOnly    = SearchMarkOf( MenuStyleMarking( true, false ) );
            const FSlateBrush OutlineOnly = SearchMarkOf( MenuStyleMarking( false, true ) );

            AITEST_TRUE( "The fill is lighter with an outline around it", Both.TintColor.GetSpecifiedColor().A < FillOnly.TintColor.GetSpecifiedColor().A );
            AITEST_TRUE( "The outline is lighter with a fill inside it", Both.OutlineSettings.Color.GetSpecifiedColor().A < OutlineOnly.OutlineSettings.Color.GetSpecifiedColor().A );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FEachMarkPartDrawsLighterWhileTheOtherIsOn, "SmartTables.OpenMenu.EachMarkPartDrawsLighterWhileTheOtherIsOn" );

    struct FAMarkPartSwitchedOffDrawsNothing : FAITestBase
    {
        virtual bool InstantTest() override
        {
            AITEST_TRUE( "Fill off leaves the box clear", SearchMarkOf( MenuStyleMarking( false, true ) ).TintColor.GetSpecifiedColor().A == 0.0f );
            AITEST_TRUE( "Outline off leaves no line", SearchMarkOf( MenuStyleMarking( true, false ) ).OutlineSettings.Width == 0.0f );
            AITEST_TRUE( "Both off draws no mark at all", SearchMarkOf( MenuStyleMarking( false, false ) ).DrawAs == ESlateBrushDrawType::NoDrawType );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAMarkPartSwitchedOffDrawsNothing, "SmartTables.OpenMenu.AMarkPartSwitchedOffDrawsNothing" );

    struct FAnInheritedLabelColourKeepsTheAuthoredHighlight : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FSlateBrush Authored;
            Authored.TintColor = FSlateColor( FLinearColor::Red );

            FSmartTableMenuStyle Style = MenuStyleMarking( true, true );
            Style.LabelTextStyle.SetColorAndOpacity( FSlateColor::UseForeground() );
            Style.LabelTextStyle.SetHighlightShape( Authored );

            AITEST_TRUE( "A label colour set to Inherit has no colour to mark with, and the authored shape stays", SearchMarkOf( Style ).TintColor.GetSpecifiedColor().Equals( FLinearColor::Red ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAnInheritedLabelColourKeepsTheAuthoredHighlight, "SmartTables.OpenMenu.AnInheritedLabelColourKeepsTheAuthoredHighlight" );

    struct FARightClickMenuNeedsAShownHeaderOrARowMenu : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTable * Table = NewObject< USmartTable >();

            AITEST_TRUE( "A shown header with its menu on opens one", Table->HasRightClickMenu() );

            Table->SetShowHeader( false );

            AITEST_FALSE( "A hidden header, and rows with no menu of their own, open nothing", Table->HasRightClickMenu() );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FARightClickMenuNeedsAShownHeaderOrARowMenu, "SmartTables.OpenMenu.ARightClickMenuNeedsAShownHeaderOrARowMenu" );

    struct FOpenRowMenuOpensWithTheRightClickSwitchedOff : FAITestBase
    {
        virtual bool InstantTest() override
        {
            AITEST_TRUE( "Slate is up, so a table builds its widget", FSlateApplication::IsInitialized() );

            UObject * Vanta  = NamedRow( TEXT( "Vanta" ) );
            UObject * Keller = NamedRow( TEXT( "Keller" ) );

            USmartTableTestHarness * Table    = NamedRowTable( { Vanta, Keller } );
            const TSharedRef< SWidget > Built = Table->TakeWidget();

            FSmartTableMenuEntry Inspect;
            Inspect.Id    = TEXT( "Inspect" );
            Inspect.Label = LOCTEXT( "Inspect", "Inspect" );
            Table->SetRowMenuEntries( { Inspect } );

            Table->OnRowMenuOpening.AddDynamic( Table, &USmartTableTestHarness::HearRowMenuOpening );

            AITEST_FALSE( "A right click on a row opens nothing by default", Table->IsRowMenuEnabled() );

            Table->OpenRowMenu( 1 );

            AITEST_EQUAL( "The call still opens the menu", Table->RowMenusHeard.Num(), 1 );
            AITEST_EQUAL( "...on the row it named", Table->RowMenusHeard[ 0 ], 1 );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FOpenRowMenuOpensWithTheRightClickSwitchedOff, "SmartTables.OpenMenu.OpenRowMenuOpensWithTheRightClickSwitchedOff" );
}
UE_ENABLE_OPTIMIZATION_SHIP

#undef LOCTEXT_NAMESPACE

#endif
