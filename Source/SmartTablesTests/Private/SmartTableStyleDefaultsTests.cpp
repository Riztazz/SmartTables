// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTablesTestsMacros.h"

#if SMARTTABLES_WITH_TESTS

#include "AITestsCommon.h"

#include "SmartTable.h"
#include "SmartTableColour.h"
#include "SmartTableSettings.h"
#include "SmartTableStyle.h"
#include "Styling/DefaultStyleCache.h"
#include "UObject/UnrealType.h"

#define LOCTEXT_NAMESPACE "SmartTablesTest"

UE_DISABLE_OPTIMIZATION_SHIP

namespace SmartTable::Test
{
    struct FEveryWarmMarkWearsTheAccent : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const USmartTableSettings * Settings = GetDefault< USmartTableSettings >();
            const USmartTableStyle * Style       = GetDefault< USmartTableStyle >();

            AITEST_NOT_NULL( "The settings exist", Settings );
            AITEST_NOT_NULL( "The style exists", Style );

            AITEST_EQUAL( "The caret wears the accent", Style->ColumnCaretBrush.TintColor.GetSpecifiedColor(), Settings->AccentColor );
            AITEST_EQUAL( "So does the focus ring", Style->FocusBorderBrush.OutlineSettings.Color.GetSpecifiedColor(), Settings->AccentColor );
            AITEST_EQUAL( "So does the drop target", Style->DragTargetBrush.OutlineSettings.Color.GetSpecifiedColor(), Settings->AccentColor );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FEveryWarmMarkWearsTheAccent, "SmartTables.Style.EveryWarmMarkWearsTheAccent" );

    struct FTheScrollbarThumbIsDarkerThanTheEngineDefault : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const FScrollBarStyle Engine = UE::Slate::Private::FDefaultStyleCache::GetRuntime().GetScrollBarStyle();

            const FLinearColor Thumb = Engine.NormalThumbImage.TintColor.GetSpecifiedColor();
            const int32 Grey         = static_cast< int32 >( Thumb.ToFColor( true ).R );

            AITEST_EQUAL( "The engine thumb tint is plain white", Grey, 255 );

            const USmartTableStyle * Style = GetDefault< USmartTableStyle >();
            AITEST_NOT_NULL( "The style exists", Style );

            const int32 Ours = static_cast< int32 >( Style->ScrollBarStyle.NormalThumbImage.TintColor.GetSpecifiedColor().ToFColor( true ).R );
            AITEST_TRUE( "And ours is darker than it", Ours < Grey );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FTheScrollbarThumbIsDarkerThanTheEngineDefault, "SmartTables.Style.TheScrollbarThumbIsDarkerThanTheEngineDefault" );

    struct FAHeaderNeverInheritsTheEnginesBlackForeground : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const USmartTableStyle * Style = GetDefault< USmartTableStyle >();
            AITEST_NOT_NULL( "The style exists", Style );

            AITEST_NOT_EQUAL( "The header foreground is not the engine's own black", Style->HeaderStyle.ForegroundColor.GetSpecifiedColor(), FLinearColor::Black );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAHeaderNeverInheritsTheEnginesBlackForeground, "SmartTables.Style.AHeaderNeverInheritsTheEnginesBlackForeground" );

    struct FTheSortArrowIsDrawnInTheHeaderLabelColour : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTable * Table = NewObject< USmartTable >();
            AITEST_NOT_NULL( "A table builds", Table );

            USmartTableStyle * Asset = NewObject< USmartTableStyle >();
            AITEST_NOT_NULL( "A style asset builds", Asset );

            const FLinearColor Authored( 0.25f, 0.5f, 0.75f );
            Asset->HeaderTextStyle.SetColorAndOpacity( FSlateColor( Authored ) );

            Table->SetStyleAsset( Asset );

            AITEST_EQUAL( "The arrow wears what the label wears", Table->GetSortTint().GetSpecifiedColor(), Authored );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FTheSortArrowIsDrawnInTheHeaderLabelColour, "SmartTables.Style.TheSortArrowIsDrawnInTheHeaderLabelColour" );

    struct FAStyleAssetGreysOutEveryPropertyItOverwrites : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const UClass * TableClass = USmartTable::StaticClass();
            AITEST_NOT_NULL( "The table class exists", TableClass );

            int32 Mirrored = 0;

            for ( TFieldIterator< FProperty > It( USmartTableStyle::StaticClass() ); It; ++It )
            {
                const FProperty * OnTable = TableClass->FindPropertyByName( It->GetFName() );
                if ( !OnTable )
                {
                    continue;
                }

                AITEST_TRUE( FString::Printf( TEXT( "%s is the same type on both, so the mirror copies it" ), *It->GetName() ), OnTable->SameType( *It ) );
                AITEST_EQUAL( FString::Printf( TEXT( "%s is the same array size on both" ), *It->GetName() ), OnTable->ArrayDim, It->ArrayDim );

                ++Mirrored;

                AITEST_EQUAL( FString::Printf( TEXT( "%s greys out while an asset is set" ), *It->GetName() ), OnTable->GetMetaData( TEXT( "EditCondition" ) ), FString( TEXT( "StyleAsset == nullptr" ) ) );
            }

            AITEST_EQUAL( "Every property the asset writes was reached", Mirrored, 18 );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAStyleAssetGreysOutEveryPropertyItOverwrites, "SmartTables.Style.AStyleAssetGreysOutEveryPropertyItOverwrites" );

    struct FClearingAStyleAssetPutsEveryAuthoredValueBack : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTable * Untouched = NewObject< USmartTable >();
            USmartTable * Cycled    = NewObject< USmartTable >();
            AITEST_NOT_NULL( "Two tables build", Cycled );

            USmartTableStyle * Loud = NewObject< USmartTableStyle >();
            AITEST_NOT_NULL( "A style builds", Loud );

            const FLinearColor Odd( 0.9f, 0.1f, 0.7f, 1.0f );

            Loud->BackgroundBrush.TintColor                 = Odd;
            Loud->RowStyle.EvenRowBackgroundBrush.TintColor = Odd;
            Loud->HeaderStyle.ForegroundColor               = Odd;
            Loud->CellTextStyle.ColorAndOpacity             = Odd;
            Loud->HeaderHeight                              = Untouched->GetHeaderHeight() + 11.0f;

            Cycled->SetStyleAsset( Loud );

            AITEST_NOT_EQUAL( "The asset reached the table", Cycled->GetHeaderHeight(), Untouched->GetHeaderHeight() );

            Cycled->SetStyleAsset( nullptr );

            const UClass * TableClass = USmartTable::StaticClass();
            int32 Checked             = 0;

            for ( TFieldIterator< FProperty > It( USmartTableStyle::StaticClass() ); It; ++It )
            {
                const FProperty * OnTable = TableClass->FindPropertyByName( It->GetFName() );
                if ( !OnTable || !OnTable->SameType( *It ) )
                {
                    continue;
                }

                ++Checked;

                AITEST_TRUE( FString::Printf( TEXT( "%s came back exactly as it was authored" ), *OnTable->GetName() ), OnTable->Identical_InContainer( Cycled, Untouched ) );
            }

            AITEST_EQUAL( "Every member the asset can write was checked", Checked, 18 );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FClearingAStyleAssetPutsEveryAuthoredValueBack, "SmartTables.Style.ClearingAStyleAssetPutsEveryAuthoredValueBack" );

    struct FTheLastColumnAlwaysWearsTheColumnStyle : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTable * Table = NewObject< USmartTable >();
            AITEST_NOT_NULL( "A table builds", Table );

            AITEST_TRUE( "A fresh table's last column matches every other", Table->GetHeaderStyle().LastColumnStyle.NormalBrush == Table->GetHeaderStyle().ColumnStyle.NormalBrush );

            USmartTableStyle * Split                                 = NewObject< USmartTableStyle >();
            Split->HeaderStyle.ColumnStyle.NormalBrush.TintColor     = FLinearColor( 0.2f, 0.8f, 0.3f, 1.0f );
            Split->HeaderStyle.LastColumnStyle.NormalBrush.TintColor = FLinearColor( 1.0f, 0.0f, 0.0f, 1.0f );

            Table->SetStyleAsset( Split );

            AITEST_TRUE( "An asset that styles the two apart is overruled", Table->GetHeaderStyle().LastColumnStyle.NormalBrush == Table->GetHeaderStyle().ColumnStyle.NormalBrush );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FTheLastColumnAlwaysWearsTheColumnStyle, "SmartTables.Style.TheLastColumnAlwaysWearsTheColumnStyle" );
    struct FAStyleAssetKeepsASortArrowItTurnedOff : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTable * Table = NewObject< USmartTable >();
            AITEST_NOT_NULL( "A table builds", Table );

            USmartTableStyle * Asset = NewObject< USmartTableStyle >();
            AITEST_NOT_NULL( "A style asset builds", Asset );

            AITEST_TRUE( "The asset starts with an arrow to turn off", Asset->HeaderStyle.ColumnStyle.SortPrimaryAscendingImage.DrawAs != ESlateBrushDrawType::NoDrawType );

            FSlateBrush Off;
            Off.DrawAs = ESlateBrushDrawType::NoDrawType;

            AITEST_NULL( "The brush being installed names no texture", Off.GetResourceObject() );
            AITEST_TRUE( "Nor any resource by name", Off.GetResourceName().IsNone() );

            Asset->HeaderStyle.ColumnStyle.SortPrimaryAscendingImage  = Off;
            Asset->HeaderStyle.ColumnStyle.SortPrimaryDescendingImage = Off;

            Table->SetStyleAsset( Asset );

            const FSlateBrush & Applied = Table->GetHeaderStyle().ColumnStyle.SortPrimaryAscendingImage;
            AITEST_TRUE( "The table keeps the arrow the asset turned off", Applied.DrawAs == ESlateBrushDrawType::NoDrawType );
            AITEST_NULL( "And puts no texture back behind it", Applied.GetResourceObject() );
            AITEST_TRUE( "Nor one by name", Applied.GetResourceName().IsNone() );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAStyleAssetKeepsASortArrowItTurnedOff, "SmartTables.Style.AStyleAssetKeepsASortArrowItTurnedOff" );
    struct FNothingAStyleAssetWritesStaysEditable : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTableStyle * Fresh = NewObject< USmartTableStyle >();
            USmartTableStyle * Loud  = NewObject< USmartTableStyle >();
            AITEST_NOT_NULL( "Two style assets build", Loud );

            const FSlateColor Odd( FLinearColor( 0.13f, 0.29f, 0.71f ) );

            Loud->BackgroundBrush.TintColor           = Odd;
            Loud->DragSourceBrush.TintColor           = Odd;
            Loud->DragTargetBrush.TintColor           = Odd;
            Loud->InsertMarkerBrush.TintColor         = Odd;
            Loud->RowInsertMarkerBrush.TintColor      = Odd;
            Loud->ColumnDividerBrush.TintColor        = Odd;
            Loud->ColumnCaretBrush.TintColor          = Odd;
            Loud->FocusBorderBrush.TintColor          = Odd;
            Loud->BusyOverlayBrush.TintColor          = Odd;
            Loud->MenuStyle.BackgroundBrush.TintColor = Odd;

            Loud->RowStyle.EvenRowBackgroundBrush.TintColor = Odd;
            Loud->HeaderStyle.ForegroundColor               = Odd;
            Loud->ScrollBarStyle.NormalThumbImage.TintColor = Odd;
            Loud->HeaderTextStyle.ColorAndOpacity           = Odd;
            Loud->CellTextStyle.ColorAndOpacity             = Odd;
            Loud->RowNumberTextStyle.ColorAndOpacity        = Odd;
            Loud->EmptyTextStyle.ColorAndOpacity            = Odd;

            Loud->HeaderHeight = Fresh->HeaderHeight + 7.0f;

            for ( TFieldIterator< FProperty > It( USmartTableStyle::StaticClass() ); It; ++It )
            {
                AITEST_FALSE( FString::Printf( TEXT( "%s was moved, so this test still covers it" ), *It->GetName() ), It->Identical_InContainer( Fresh, Loud ) );
            }

            USmartTable * Plain  = NewObject< USmartTable >();
            USmartTable * Styled = NewObject< USmartTable >();
            AITEST_NOT_NULL( "Two tables build", Styled );

            Styled->SetStyleAsset( Loud );

            for ( TFieldIterator< FProperty > It( USmartTable::StaticClass() ); It; ++It )
            {
                const bool bShowsInThePanel  = It->HasAnyPropertyFlags( CPF_Edit );
                const bool bIsTheAssetItself = It->GetFName() == TEXT( "StyleAsset" );

                if ( !bShowsInThePanel || bIsTheAssetItself || It->Identical_InContainer( Plain, Styled ) )
                {
                    continue;
                }

                AITEST_EQUAL( FString::Printf( TEXT( "%s moved when the asset was set, so it greys out" ), *It->GetName() ), It->GetMetaData( TEXT( "EditCondition" ) ), FString( TEXT( "StyleAsset == nullptr" ) ) );
            }

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FNothingAStyleAssetWritesStaysEditable, "SmartTables.Style.NothingAStyleAssetWritesStaysEditable" );

    struct FCloseMenuIsReachableWithNoMouse : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const UFunction * Close = USmartTable::StaticClass()->FindFunctionByName( TEXT( "CloseMenu" ) );
            AITEST_NOT_NULL( "A table can be told to close its menu", Close );
            AITEST_TRUE( "And a graph can tell it", Close->HasAnyFunctionFlags( FUNC_BlueprintCallable ) );

            USmartTable * Table = NewObject< USmartTable >();
            AITEST_NOT_NULL( "A table builds", Table );

            Table->CloseMenu();

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FCloseMenuIsReachableWithNoMouse, "SmartTables.Input.CloseMenuIsReachableWithNoMouse" );

}
UE_ENABLE_OPTIMIZATION_SHIP

#undef LOCTEXT_NAMESPACE

#endif
