// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTablesTestsMacros.h"

#if SMARTTABLES_WITH_TESTS

#include "AITestsCommon.h"

#include "Internationalization/Culture.h"
#include "Internationalization/Internationalization.h"

#include "SmartTableBinding.h"
#include "SmartTableCell.h"
#include "SmartTableDataTableModel.h"
#include "SmartTableObjectModel.h"
#include "SmartTableSettings.h"
#include "SmartTableTestTypes.h"
#include "SmartTableTypes.h"

#define LOCTEXT_NAMESPACE "SmartTablesTest"

UE_DISABLE_OPTIMIZATION_SHIP

namespace SmartTable::Test
{
    static FSmartTableColumn FormatWith( int32 Decimals )
    {
        FSmartTableColumn Column;
        Column.MaxFractionalDigits = Decimals;

        return Column;
    }

    static FText ReadColumnField( const FSmartTableColumn & Row, FName Field, int32 Decimals = 2 )
    {
        return ReadStructAsText( ResolveBinding( FSmartTableColumn::StaticStruct(), Field ), &Row, FormatWith( Decimals ) );
    }

    static FSmartTableSortKey KeyForColumnField( const FSmartTableColumn & Row, FName Field )
    {
        return ReadStructAsSortKey( ResolveBinding( FSmartTableColumn::StaticStruct(), Field ), &Row );
    }

    struct FBindingResolvesPropertiesThenFunctions : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const FBinding Property = ResolveBinding( USmartTableSettings::StaticClass(), TEXT( "MinColumnWidth" ) );
            AITEST_TRUE( "A property resolves", Property.IsValid() );
            AITEST_TRUE( "...as a property", Property.Property.IsValid() );
            AITEST_FALSE( "...and not as a function", Property.Function.IsValid() );

            AITEST_TRUE( "Case does not matter", ResolveBinding( USmartTableSettings::StaticClass(), TEXT( "mincolumnwidth" ) ).IsValid() );

            const FBinding Function = ResolveBinding( USmartTableObjectModel::StaticClass(), TEXT( "IsBusy" ) );
            AITEST_TRUE( "A parameterless pure function resolves", Function.IsValid() );
            AITEST_TRUE( "...as a function", Function.Function.IsValid() );

            AITEST_FALSE( "A function with parameters does not", ResolveBinding( USmartTableObjectModel::StaticClass(), TEXT( "GetCellText" ) ).IsValid() );
            AITEST_FALSE( "Nor does a name nothing answers to", ResolveBinding( USmartTableSettings::StaticClass(), TEXT( "NotAThing" ) ).IsValid() );
            AITEST_FALSE( "Nor an unset binding name", ResolveBinding( USmartTableSettings::StaticClass(), NAME_None ).IsValid() );
            AITEST_FALSE( "Nor a null class", ResolveBinding( nullptr, TEXT( "MinColumnWidth" ) ).IsValid() );

            AITEST_TRUE( "Struct fields resolve", ResolveBinding( FSmartTableColumn::StaticStruct(), TEXT( "Width" ) ).IsValid() );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FBindingResolvesPropertiesThenFunctions, "SmartTables.Binding.ResolvesPropertiesThenFunctions" );

    struct FBindableNamesListWhatAColumnCouldSay : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TArray< FName > ClassNames = GetBindableNames( USmartTableObjectModel::StaticClass() );
            AITEST_TRUE( "Properties are offered", ClassNames.Contains( TEXT( "ActiveFilterText" ) ) );
            AITEST_TRUE( "...and so are bindable functions", ClassNames.Contains( TEXT( "IsBusy" ) ) );
            AITEST_FALSE( "...but not the ones that take arguments", ClassNames.Contains( TEXT( "GetCellText" ) ) );

            const TArray< FName > StructNames = GetBindableNames( FSmartTableColumn::StaticStruct() );
            AITEST_TRUE( "A struct offers its fields", StructNames.Contains( TEXT( "ColumnId" ) ) );
            AITEST_TRUE( "...all of them", StructNames.Contains( TEXT( "MaxFractionalDigits" ) ) );

            AITEST_TRUE( "Nothing has nothing to offer", GetBindableNames( nullptr ).IsEmpty() );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FBindableNamesListWhatAColumnCouldSay, "SmartTables.Binding.BindableNamesListWhatAColumnCouldSay" );

    struct FStructReadsCoverEveryTypeAColumnClaims : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FSmartTableColumn Row;
            Row.ColumnId            = TEXT( "Callsign" );
            Row.Header              = LOCTEXT( "HeaderValue", "callsign" );
            Row.Width               = 12.5f;
            Row.MaxFractionalDigits = 3;
            Row.bSortable           = false;
            Row.Sizing              = ESmartTableColumnSizing::Fixed;
            Row.HAlign              = HAlign_Center;

            AITEST_EQUAL( "A name reads as itself", ReadColumnField( Row, TEXT( "ColumnId" ) ).ToString(), FString( TEXT( "Callsign" ) ) );
            AITEST_EQUAL( "Text passes straight through", ReadColumnField( Row, TEXT( "Header" ) ).ToString(), FString( TEXT( "callsign" ) ) );
            AITEST_EQUAL( "An int reads as a number", ReadColumnField( Row, TEXT( "MaxFractionalDigits" ) ).ToString(), FString( TEXT( "3" ) ) );
            AITEST_EQUAL( "A bool reads as a word", ReadColumnField( Row, TEXT( "bSortable" ) ).ToString(), FString( TEXT( "false" ) ) );

            AITEST_EQUAL( "A scoped enum reads as its name", ReadColumnField( Row, TEXT( "Sizing" ) ).ToString(), FString( TEXT( "Fixed" ) ) );

            AITEST_EQUAL( "A TEnumAsByte reads as its name too", ReadColumnField( Row, TEXT( "HAlign" ) ).ToString(), FString( TEXT( "Center" ) ) );

            AITEST_EQUAL( "An unreadable type says so", ReadColumnField( Row, TEXT( "CellPadding" ) ).ToString(), FString( TEXT( "<unsupported>" ) ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FStructReadsCoverEveryTypeAColumnClaims, "SmartTables.Binding.StructReadsCoverEveryTypeAColumnClaims" );

    struct FDecimalsAreTheColumnsChoice : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FSmartTableColumn Row;
            Row.Width = 12.5f;

            const FString Coarse = ReadColumnField( Row, TEXT( "Width" ), 0 ).ToString();
            const FString Fine   = ReadColumnField( Row, TEXT( "Width" ), 2 ).ToString();

            AITEST_NOT_EQUAL( "Decimal places change the text", Coarse, Fine );
            AITEST_TRUE( "More places is more text", Fine.Len() > Coarse.Len() );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FDecimalsAreTheColumnsChoice, "SmartTables.Binding.DecimalsAreTheColumnsChoice" );

    struct FSortKeysOrderByValueNotByLabel : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FSmartTableColumn Fill;
            Fill.Sizing    = ESmartTableColumnSizing::Fill;
            Fill.Width     = 1.5f;
            Fill.bSortable = false;
            Fill.ColumnId  = TEXT( "Vanta" );

            FSmartTableColumn Fixed;
            Fixed.Sizing    = ESmartTableColumnSizing::Fixed;
            Fixed.Width     = 1.25f;
            Fixed.bSortable = true;
            Fixed.ColumnId  = TEXT( "Asteroid #2" );

            const FSmartTableSortKey SmallWidth = KeyForColumnField( Fixed, TEXT( "Width" ) );
            AITEST_TRUE( "A float sorts as a number", SmallWidth.Kind == ESmartTableSortKeyKind::Numeric );
            AITEST_TRUE( "...and 1.25 is below 1.5", SmallWidth.Compare( KeyForColumnField( Fill, TEXT( "Width" ) ) ) < 0 );

            const FSmartTableSortKey FillKey = KeyForColumnField( Fill, TEXT( "Sizing" ) );
            AITEST_TRUE( "An enum sorts as a number", FillKey.Kind == ESmartTableSortKeyKind::Numeric );
            AITEST_TRUE( "...in declaration order", FillKey.Compare( KeyForColumnField( Fixed, TEXT( "Sizing" ) ) ) < 0 );

            AITEST_TRUE( "A bool sorts false-first", KeyForColumnField( Fill, TEXT( "bSortable" ) ).Compare( KeyForColumnField( Fixed, TEXT( "bSortable" ) ) ) < 0 );

            const FSmartTableSortKey Name = KeyForColumnField( Fill, TEXT( "ColumnId" ) );
            AITEST_TRUE( "A name sorts as a string", Name.Kind == ESmartTableSortKeyKind::String );
            AITEST_TRUE( "...naturally", KeyForColumnField( Fixed, TEXT( "ColumnId" ) ).Compare( Name ) < 0 );

            AITEST_TRUE( "An unreadable type sorts as empty", KeyForColumnField( Fill, TEXT( "CellPadding" ) ).IsEmpty() );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FSortKeysOrderByValueNotByLabel, "SmartTables.Binding.SortKeysOrderByValueNotByLabel" );

    struct FObjectReadsGoThroughPropertiesAndCalls : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTableObjectModel * Model = NewObject< USmartTableObjectModel >();
            Model->ApplyTextFilter( FText::FromString( TEXT( "vanta" ) ), TArray< FName >() );

            const FSmartTableColumn Column = FormatWith( 2 );

            AITEST_EQUAL( "A property on an object reads", ReadAsText( ResolveBinding( Model->GetClass(), TEXT( "ActiveFilterText" ) ), Model, Column ).ToString(), FString( TEXT( "vanta" ) ) );

            AITEST_EQUAL( "A pure function is called and read", ReadAsText( ResolveBinding( Model->GetClass(), TEXT( "GetActiveFilterText" ) ), Model, Column ).ToString(), FString( TEXT( "vanta" ) ) );

            AITEST_EQUAL( "...including one returning a bool", ReadAsText( ResolveBinding( Model->GetClass(), TEXT( "IsBusy" ) ), Model, Column ).ToString(), FString( TEXT( "false" ) ) );

            AITEST_TRUE( "...and its sort key", ReadAsSortKey( ResolveBinding( Model->GetClass(), TEXT( "IsBusy" ) ), Model ).Kind == ESmartTableSortKeyKind::Numeric );

            AITEST_EQUAL( "A struct return with no text form says so", ReadAsText( ResolveBinding( Model->GetClass(), TEXT( "GetActiveSortSpec" ) ), Model, Column ).ToString(), FString( TEXT( "<unsupported>" ) ) );

            USmartTableDataTableRow * Row = NewObject< USmartTableDataTableRow >();
            Row->RowName                  = TEXT( "Row_7" );
            AITEST_EQUAL( "A name property reads", ReadAsText( ResolveBinding( Row->GetClass(), TEXT( "RowName" ) ), Row, Column ).ToString(), FString( TEXT( "Row_7" ) ) );
            AITEST_TRUE( "An unset object reads as nothing", ReadAsText( ResolveBinding( Row->GetClass(), TEXT( "DataTable" ) ), Row, Column ).IsEmpty() );
            AITEST_TRUE( "...and sorts as empty", ReadAsSortKey( ResolveBinding( Row->GetClass(), TEXT( "DataTable" ) ), Row ).IsEmpty() );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FObjectReadsGoThroughPropertiesAndCalls, "SmartTables.Binding.ObjectReadsGoThroughPropertiesAndCalls" );

    struct FNothingToReadReadsAsNothing : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const FSmartTableColumn Column = FormatWith( 2 );
            const FBinding Unresolved;
            const FBinding Resolved = ResolveBinding( FSmartTableColumn::StaticStruct(), TEXT( "Width" ) );

            USmartTableDataTableRow * Row = NewObject< USmartTableDataTableRow >();

            AITEST_TRUE( "An unresolved binding reads as nothing", ReadAsText( Unresolved, Row, Column ).IsEmpty() );
            AITEST_TRUE( "...and sorts as empty", ReadAsSortKey( Unresolved, Row ).IsEmpty() );
            AITEST_TRUE( "A null item reads as nothing", ReadAsText( Resolved, nullptr, Column ).IsEmpty() );
            AITEST_TRUE( "...and sorts as empty", ReadAsSortKey( Resolved, nullptr ).IsEmpty() );
            AITEST_TRUE( "Null row memory reads as nothing", ReadStructAsText( Resolved, nullptr, Column ).IsEmpty() );
            AITEST_TRUE( "...and sorts as empty", ReadStructAsSortKey( Resolved, nullptr ).IsEmpty() );

            const FBinding Function = ResolveBinding( USmartTableObjectModel::StaticClass(), TEXT( "IsBusy" ) );
            const FSmartTableColumn Data;
            AITEST_TRUE( "A function binding cannot read a struct", ReadStructAsText( Function, &Data, Column ).IsEmpty() );
            AITEST_TRUE( "...and sorts as empty", ReadStructAsSortKey( Function, &Data ).IsEmpty() );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FNothingToReadReadsAsNothing, "SmartTables.Binding.NothingToReadReadsAsNothing" );

    struct FAWidgetColumnThatAskedForNothingDoesNotWarn : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const FBinding Unresolved;
            AITEST_FALSE( "Precondition: the binding really did fail", Unresolved.IsValid() );

            FSmartTableColumn WidgetOnly;
            WidgetOnly.ColumnId  = TEXT( "Play" );
            WidgetOnly.CellClass = USmartTableTextCell::StaticClass();
            AITEST_FALSE( "A widget column that named no binding is silent", ShouldWarnForUnresolvedBinding( WidgetOnly, Unresolved ) );

            FSmartTableColumn WidgetAndBinding = WidgetOnly;
            WidgetAndBinding.BindingName       = TEXT( "Nonexistent" );
            AITEST_TRUE( "...but one that named a binding still warns", ShouldWarnForUnresolvedBinding( WidgetAndBinding, Unresolved ) );

            FSmartTableColumn PlainText;
            PlainText.ColumnId = TEXT( "Price" );
            AITEST_TRUE( "A plain column always warns", ShouldWarnForUnresolvedBinding( PlainText, Unresolved ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAWidgetColumnThatAskedForNothingDoesNotWarn, "SmartTables.Binding.AWidgetColumnThatAskedForNothingDoesNotWarn" );

    struct FANamedSortBindingThatFailsIsWorthSaying : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const FBinding Unresolved;

            FSmartTableColumn FallsBack;
            FallsBack.ColumnId = TEXT( "Price" );
            AITEST_FALSE( "A column that named no sort binding is silent", ShouldWarnForUnresolvedSortBinding( FallsBack, Unresolved ) );

            FSmartTableColumn Typo;
            Typo.ColumnId        = TEXT( "Price" );
            Typo.SortBindingName = TEXT( "PriceSortKye" );
            AITEST_TRUE( "A named sort binding that fails is worth saying", ShouldWarnForUnresolvedSortBinding( Typo, Unresolved ) );

            FBinding Resolved;
            Resolved.Property = FindFProperty< FProperty >( FSmartTableTestRow::StaticStruct(), TEXT( "Integrity" ) );
            AITEST_TRUE( "Precondition: that one really did resolve", Resolved.IsValid() );
            AITEST_FALSE( "A named sort binding that works is silent", ShouldWarnForUnresolvedSortBinding( Typo, Resolved ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FANamedSortBindingThatFailsIsWorthSaying, "SmartTables.Binding.ANamedSortBindingThatFailsIsWorthSaying" );

    struct FAResolvedBindingNeverWarns : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const FBinding Resolved = ResolveBinding( FSmartTableColumn::StaticStruct(), TEXT( "ColumnId" ) );
            AITEST_TRUE( "Precondition: this one really did resolve", Resolved.IsValid() );

            FSmartTableColumn PlainText;
            PlainText.ColumnId = TEXT( "ColumnId" );
            AITEST_FALSE( "A resolved plain column is silent", ShouldWarnForUnresolvedBinding( PlainText, Resolved ) );

            FSmartTableColumn WidgetAndBinding;
            WidgetAndBinding.ColumnId    = TEXT( "ColumnId" );
            WidgetAndBinding.CellClass   = USmartTableTextCell::StaticClass();
            WidgetAndBinding.BindingName = TEXT( "ColumnId" );
            AITEST_FALSE( "So is a resolved widget column", ShouldWarnForUnresolvedBinding( WidgetAndBinding, Resolved ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAResolvedBindingNeverWarns, "SmartTables.Binding.AResolvedBindingNeverWarns" );

    static ESmartTableCellEditor EditorForColumnField( FName Field )
    {
        return EditorFor( ResolveBinding( FSmartTableColumn::StaticStruct(), Field ) );
    }

    static bool WriteColumnField( FSmartTableColumn & Row, FName Field, const FString & Value )
    {
        return WriteStructAsText( ResolveBinding( FSmartTableColumn::StaticStruct(), Field ), &Row, FText::FromString( Value ) );
    }

    struct FEditorFollowsThePropertyType : FAITestBase
    {
        virtual bool InstantTest() override
        {
            AITEST_EQUAL( "A name is typed", EditorForColumnField( TEXT( "ColumnId" ) ), ESmartTableCellEditor::Text );
            AITEST_EQUAL( "So is a text", EditorForColumnField( TEXT( "Header" ) ), ESmartTableCellEditor::Text );
            AITEST_EQUAL( "A float is a number", EditorForColumnField( TEXT( "Width" ) ), ESmartTableCellEditor::Number );
            AITEST_EQUAL( "So is an int", EditorForColumnField( TEXT( "MaxFractionalDigits" ) ), ESmartTableCellEditor::Number );
            AITEST_EQUAL( "A bool is a toggle", EditorForColumnField( TEXT( "bSortable" ) ), ESmartTableCellEditor::Toggle );

            AITEST_EQUAL( "A scoped enum is nobody's text box", EditorForColumnField( TEXT( "Sizing" ) ), ESmartTableCellEditor::None );
            AITEST_EQUAL( "Nor is a TEnumAsByte", EditorForColumnField( TEXT( "HAlign" ) ), ESmartTableCellEditor::None );

            AITEST_EQUAL( "A struct has no text form to edit", EditorForColumnField( TEXT( "CellPadding" ) ), ESmartTableCellEditor::None );
            AITEST_EQUAL( "An unresolved name offers nothing", EditorForColumnField( TEXT( "NotAThing" ) ), ESmartTableCellEditor::None );

            AITEST_EQUAL( "Nor does a function binding", EditorFor( ResolveBinding( USmartTableObjectModel::StaticClass(), TEXT( "IsBusy" ) ) ), ESmartTableCellEditor::None );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FEditorFollowsThePropertyType, "SmartTables.Binding.EditorFollowsThePropertyType" );

    struct FAWrittenValueReadsBackAsItself : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FSmartTableColumn Row;

            AITEST_TRUE( "A name takes a write", WriteColumnField( Row, TEXT( "ColumnId" ), TEXT( "Callsign" ) ) );
            AITEST_EQUAL( "...and reads back", Row.ColumnId, FName( TEXT( "Callsign" ) ) );

            AITEST_TRUE( "A float takes a write", WriteColumnField( Row, TEXT( "Width" ), TEXT( "12.5" ) ) );
            AITEST_EQUAL( "...and reads back", Row.Width, 12.5f );

            AITEST_TRUE( "An int takes a write", WriteColumnField( Row, TEXT( "MaxFractionalDigits" ), TEXT( "4" ) ) );
            AITEST_EQUAL( "...and reads back", Row.MaxFractionalDigits, 4 );

            AITEST_TRUE( "A bool takes false", WriteColumnField( Row, TEXT( "bSortable" ), TEXT( "false" ) ) );
            AITEST_FALSE( "...and reads back", Row.bSortable );
            AITEST_TRUE( "A bool takes true", WriteColumnField( Row, TEXT( "bSortable" ), TEXT( "true" ) ) );
            AITEST_TRUE( "...and reads back", Row.bSortable );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAWrittenValueReadsBackAsItself, "SmartTables.Binding.AWrittenValueReadsBackAsItself" );

    struct FATypoIsRefusedRatherThanRounded : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FSmartTableColumn Row;
            Row.Width = 7.0f;

            AITEST_FALSE( "A typo is refused", WriteColumnField( Row, TEXT( "Width" ), TEXT( "12abc" ) ) );
            AITEST_EQUAL( "...and changes nothing", Row.Width, 7.0f );

            AITEST_FALSE( "So is empty text", WriteColumnField( Row, TEXT( "Width" ), FString() ) );
            AITEST_EQUAL( "...and changes nothing", Row.Width, 7.0f );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FATypoIsRefusedRatherThanRounded, "SmartTables.Binding.ATypoIsRefusedRatherThanRounded" );

    struct FAWholeNumberFieldRefusesADot : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FSmartTableColumn Row;
            Row.MaxFractionalDigits = 4;

            AITEST_FALSE( "A fraction is refused", WriteColumnField( Row, TEXT( "MaxFractionalDigits" ), TEXT( "3.5" ) ) );
            AITEST_EQUAL( "...and changes nothing", Row.MaxFractionalDigits, 4 );

            AITEST_FALSE( "A trailing dot is refused too", WriteColumnField( Row, TEXT( "MaxFractionalDigits" ), TEXT( "3." ) ) );
            AITEST_EQUAL( "...and changes nothing", Row.MaxFractionalDigits, 4 );

            AITEST_TRUE( "A whole number is taken", WriteColumnField( Row, TEXT( "MaxFractionalDigits" ), TEXT( "6" ) ) );
            AITEST_EQUAL( "...as itself", Row.MaxFractionalDigits, 6 );

            AITEST_TRUE( "And a float field still takes a fraction", WriteColumnField( Row, TEXT( "Width" ), TEXT( "2.5" ) ) );
            AITEST_EQUAL( "...as itself", Row.Width, 2.5f );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAWholeNumberFieldRefusesADot, "SmartTables.Binding.AWholeNumberFieldRefusesADot" );

    struct FANumberSurvivesTheStringItWasShownAs : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FSmartTableColumn Row;
            Row.Width = 1144.36f;

            const FText Shown = ReadColumnField( Row, TEXT( "Width" ) );
            AITEST_TRUE( "The display string is accepted back", WriteColumnField( Row, TEXT( "Width" ), Shown.ToString() ) );
            AITEST_EQUAL( "...as the value it was", Row.Width, 1144.36f );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FANumberSurvivesTheStringItWasShownAs, "SmartTables.Binding.ANumberSurvivesTheStringItWasShownAs" );

    struct FWhatOffersNoEditorRefusesEveryWrite : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FSmartTableColumn Row;
            Row.Sizing = ESmartTableColumnSizing::Fill;
            Row.HAlign = HAlign_Left;

            AITEST_FALSE( "A scoped enum refuses", WriteColumnField( Row, TEXT( "Sizing" ), TEXT( "Fixed" ) ) );
            AITEST_EQUAL( "...and is unchanged", Row.Sizing, ESmartTableColumnSizing::Fill );

            AITEST_FALSE( "A TEnumAsByte refuses", WriteColumnField( Row, TEXT( "HAlign" ), TEXT( "2" ) ) );
            AITEST_EQUAL( "...and is unchanged", Row.HAlign.GetValue(), HAlign_Left );

            AITEST_FALSE( "A struct refuses", WriteColumnField( Row, TEXT( "CellPadding" ), TEXT( "4" ) ) );
            AITEST_FALSE( "An unresolved name refuses", WriteColumnField( Row, TEXT( "NotAThing" ), TEXT( "x" ) ) );
            AITEST_FALSE( "A null container refuses", WriteStructAsText( ResolveBinding( FSmartTableColumn::StaticStruct(), TEXT( "Width" ) ), nullptr, FText::FromString( TEXT( "1" ) ) ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FWhatOffersNoEditorRefusesEveryWrite, "SmartTables.Binding.WhatOffersNoEditorRefusesEveryWrite" );

    struct FAModelIsReadOnlyUntilItSaysOtherwise : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTableTestStructModel * Model = NewObject< USmartTableTestStructModel >();
            Model->SetRows( { FSmartTableTestRow() } );

            AITEST_EQUAL( "A model that says nothing offers no editor", Model->GetCellEditor( 0, TEXT( "Callsign" ) ), ESmartTableCellEditor::None );
            AITEST_FALSE( "...and refuses a write", Model->SetCellText( 0, TEXT( "Callsign" ), FText::FromString( TEXT( "x" ) ) ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAModelIsReadOnlyUntilItSaysOtherwise, "SmartTables.Model.AModelIsReadOnlyUntilItSaysOtherwise" );
    struct FANumberSurvivesACommaForADecimalPoint : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FInternationalization & I18n = FInternationalization::Get();

            const FString Was    = I18n.GetCurrentCulture()->GetName();
            const bool bSwitched = I18n.SetCurrentCulture( TEXT( "de" ) );

            const FString Drawn = FText::AsNumber( 1234.5 ).ToString();

            FSmartTableColumn Row;
            Row.Width         = 1.0f;
            const bool bTaken = WriteColumnField( Row, TEXT( "Width" ), *Drawn );
            const float Took  = Row.Width;

            const bool bRestored = I18n.SetCurrentCulture( Was );

            AITEST_TRUE( "The settings this test needs are here", bSwitched );
            AITEST_TRUE( FString::Printf( TEXT( "A comma is the point now, and the reader drew '%s'" ), *Drawn ), Drawn.EndsWith( TEXT( ",5" ) ) );
            AITEST_TRUE( "The number the reader drew is taken back", bTaken );
            AITEST_EQUAL( "As the number it drew", Took, 1234.5f );
            AITEST_TRUE( "And the setting is put back", bRestored );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FANumberSurvivesACommaForADecimalPoint, "SmartTables.Binding.ANumberSurvivesACommaForADecimalPoint" );

}
UE_ENABLE_OPTIMIZATION_SHIP

#undef LOCTEXT_NAMESPACE
#endif
