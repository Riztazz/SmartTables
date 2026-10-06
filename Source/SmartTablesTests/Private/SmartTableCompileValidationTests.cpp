// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTablesTestsMacros.h"

#if SMARTTABLES_WITH_TESTS && WITH_EDITOR

#include "AITestsCommon.h"

#include "Editor/WidgetCompilerLog.h"
#include "Engine/DataTable.h"
#include "SmartTable.h"
#include "SmartTableSettings.h"
#include "SmartTableTestTypes.h"
#include "SmartTableTypes.h"

#define LOCTEXT_NAMESPACE "SmartTablesTest"

UE_DISABLE_OPTIMIZATION_SHIP

namespace SmartTable::Test
{
    struct FCollectedCompileLog : IWidgetCompilerLog
    {
        TArray< FText > Errors;

        virtual ~FCollectedCompileLog() = default;

        virtual TSubclassOf< UUserWidget > GetContextClass() const override
        {
            return nullptr;
        }

    protected:
        virtual void InternalLogMessage( TSharedRef< FTokenizedMessage > & Message ) override
        {
            if ( Message->GetSeverity() == EMessageSeverity::Error )
            {
                Errors.Add( Message->ToText() );
            }
        }
    };

    static FSmartTableColumn ColumnBinding( FName ColumnId, FName Binding )
    {
        FSmartTableColumn Column;
        Column.ColumnId    = ColumnId;
        Column.BindingName = Binding;

        return Column;
    }

    static int32 ErrorsFor( USmartTableTestHarness & Table )
    {
        FCollectedCompileLog Log;
        Table.RunCompileValidation( Log );

        return Log.Errors.Num();
    }

    struct FARenamedFieldFailsTheCompile : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTableTestHarness * Table = NewObject< USmartTableTestHarness >();
            Table->ExpectItemsOf( USmartTableSettings::StaticClass() );

            Table->Author( { ColumnBinding( TEXT( "Width" ), TEXT( "MinColumnWidth" ) ) } );
            AITEST_EQUAL( "A binding that resolves compiles clean", ErrorsFor( *Table ), 0 );

            Table->Author( { ColumnBinding( TEXT( "Width" ), TEXT( "RenamedAwayFromThis" ) ) } );
            AITEST_EQUAL( "A binding that does not resolve fails it", ErrorsFor( *Table ), 1 );

            FSmartTableColumn SortBroken = ColumnBinding( TEXT( "Width" ), TEXT( "MinColumnWidth" ) );
            SortBroken.SortBindingName   = TEXT( "AlsoNotAThing" );
            Table->Author( { SortBroken } );
            AITEST_EQUAL( "So does a broken SORT binding", ErrorsFor( *Table ), 1 );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FARenamedFieldFailsTheCompile, "SmartTables.Compile.ARenamedFieldFailsTheCompile" );

    struct FATableThatNamesNoSourceIsNotAnError : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTableTestHarness * Table = NewObject< USmartTableTestHarness >();
            Table->Author( { ColumnBinding( TEXT( "Anything" ), TEXT( "CouldBeAnything" ) ) } );

            AITEST_EQUAL( "With no class and no table, nothing is claimed", ErrorsFor( *Table ), 0 );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FATableThatNamesNoSourceIsNotAnError, "SmartTables.Compile.ATableThatNamesNoSourceIsNotAnError" );

    struct FALinkedTableThatIsGoneFailsTheCompile : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTableTestHarness * Table = NewObject< USmartTableTestHarness >();

            Table->LinkTo( TSoftObjectPtr< UDataTable >( FSoftObjectPath( TEXT( "/Game/NoSuchPath/DT_Gone.DT_Gone" ) ) ) );
            AITEST_EQUAL( "A link to a missing DataTable fails the compile", ErrorsFor( *Table ), 1 );

            Table->Author( { ColumnBinding( TEXT( "A" ), TEXT( "X" ) ), ColumnBinding( TEXT( "B" ), TEXT( "Y" ) ) } );
            AITEST_EQUAL( "...once, not once per column", ErrorsFor( *Table ), 1 );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FALinkedTableThatIsGoneFailsTheCompile, "SmartTables.Compile.ALinkedTableThatIsGoneFailsTheCompile" );

    struct FALinkedRowStructIsWhatColumnsAreCheckedAgainst : FAITestBase
    {
        virtual bool InstantTest() override
        {
            UDataTable * Linked = NewObject< UDataTable >();
            Linked->RowStruct   = FSmartTableTestRow::StaticStruct();

            USmartTableTestHarness * Table = NewObject< USmartTableTestHarness >();
            Table->LinkTo( Linked );

            Table->Author( { ColumnBinding( TEXT( "Callsign" ), TEXT( "Callsign" ) ) } );
            AITEST_EQUAL( "A field the row struct has compiles clean", ErrorsFor( *Table ), 0 );

            Table->Author( { ColumnBinding( TEXT( "Callsign" ), TEXT( "Ass" ) ) } );
            AITEST_EQUAL( "One it does not fails", ErrorsFor( *Table ), 1 );

            Table->ExpectItemsOf( USmartTableSettings::StaticClass() );
            AITEST_EQUAL( "Switching the mode switches the schema", ErrorsFor( *Table ), 1 );

            Table->Author( { ColumnBinding( TEXT( "Width" ), TEXT( "MinColumnWidth" ) ) } );
            AITEST_EQUAL( "...and a class field is the right one in class mode", ErrorsFor( *Table ), 0 );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FALinkedRowStructIsWhatColumnsAreCheckedAgainst, "SmartTables.Compile.ALinkedRowStructIsWhatColumnsAreCheckedAgainst" );

    struct FSwitchingSourceOnlyWarnsWhenSomethingWouldGo : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTableTestHarness * Table = NewObject< USmartTableTestHarness >();

            AITEST_TRUE( "An empty table loses nothing", Table->WhatSwitchingLoses( ESmartTableColumnSource::DataTable ).IsEmpty() );

            Table->ExpectItemsOf( USmartTableSettings::StaticClass() );
            AITEST_TRUE( "Staying put loses nothing", Table->WhatSwitchingLoses( ESmartTableColumnSource::ItemClass ).IsEmpty() );
            AITEST_FALSE( "Moving off a set class does", Table->WhatSwitchingLoses( ESmartTableColumnSource::DataTable ).IsEmpty() );

            Table->LinkTo( TSoftObjectPtr< UDataTable >( FSoftObjectPath( TEXT( "/Game/NoSuchPath/DT_Gone.DT_Gone" ) ) ) );
            Table->ExpectItemsOf( USmartTableSettings::StaticClass() );

            const FString Both = Table->WhatSwitchingLoses( ESmartTableColumnSource::File ).ToString();
            AITEST_TRUE( "Both are named", Both.Contains( TEXT( "SmartTableSettings" ) ) );

            AITEST_TRUE( "...including one whose asset is gone", Both.Contains( TEXT( "DT_Gone" ) ) );

            Table->PointAtFile( TEXT( "C:/Manifests/Stations.csv" ) );

            const FString WithFile = Table->WhatSwitchingLoses( ESmartTableColumnSource::DataTable ).ToString();
            AITEST_TRUE( "A file is named", WithFile.Contains( TEXT( "Stations.csv" ) ) );
            AITEST_FALSE( "...by filename rather than by path", WithFile.Contains( TEXT( "C:/Manifests" ) ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FSwitchingSourceOnlyWarnsWhenSomethingWouldGo, "SmartTables.Compile.SwitchingSourceOnlyWarnsWhenSomethingWouldGo" );

    struct FTwoColumnsCannotShareAnId : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTableTestHarness * Table = NewObject< USmartTableTestHarness >();

            Table->Author( { ColumnBinding( TEXT( "Callsign" ), NAME_None ), ColumnBinding( TEXT( "Owner" ), NAME_None ) } );
            AITEST_EQUAL( "Two distinct ids are fine", ErrorsFor( *Table ), 0 );

            Table->Author( { ColumnBinding( TEXT( "Callsign" ), NAME_None ), ColumnBinding( TEXT( "Callsign" ), NAME_None ) } );
            AITEST_EQUAL( "The same id twice is one error", ErrorsFor( *Table ), 1 );

            Table->Author( { ColumnBinding( TEXT( "Callsign" ), NAME_None ), ColumnBinding( TEXT( "Callsign" ), NAME_None ), ColumnBinding( TEXT( "Callsign" ), NAME_None ) } );
            AITEST_EQUAL( "Three sharing one id is still that one id", ErrorsFor( *Table ), 1 );

            Table->Author( { ColumnBinding( TEXT( "Callsign" ), NAME_None ), ColumnBinding( TEXT( "Callsign" ), NAME_None ), ColumnBinding( TEXT( "Owner" ), NAME_None ), ColumnBinding( TEXT( "Owner" ), NAME_None ) } );
            AITEST_EQUAL( "Two doubled ids are two things to fix", ErrorsFor( *Table ), 2 );

            Table->Author( { ColumnBinding( NAME_None, NAME_None ), ColumnBinding( NAME_None, NAME_None ) } );
            AITEST_EQUAL( "Two unnamed columns are not duplicates of each other", ErrorsFor( *Table ), 0 );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FTwoColumnsCannotShareAnId, "SmartTables.Compile.TwoColumnsCannotShareAnId" );

    struct FALinkedTableWithNoRowStructFailsTheCompile : FAITestBase
    {
        virtual bool InstantTest() override
        {
            UDataTable * Linked = NewObject< UDataTable >();

            USmartTableTestHarness * Table = NewObject< USmartTableTestHarness >();
            Table->LinkTo( Linked );
            Table->Author( { ColumnBinding( TEXT( "Callsign" ), TEXT( "Callsign" ) ) } );

            AITEST_EQUAL( "A row struct nobody set fails the compile", ErrorsFor( *Table ), 1 );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FALinkedTableWithNoRowStructFailsTheCompile, "SmartTables.Compile.ALinkedTableWithNoRowStructFailsTheCompile" );
}
UE_ENABLE_OPTIMIZATION_SHIP

#undef LOCTEXT_NAMESPACE
#endif
