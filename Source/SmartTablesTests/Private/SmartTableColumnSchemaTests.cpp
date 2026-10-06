// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTablesTestsMacros.h"

#if SMARTTABLES_WITH_TESTS

#include "AITestsCommon.h"

#include "Containers/Set.h"
#include "Engine/DataTable.h"
#include "Misc/Paths.h"
#include "SmartTableBinding.h"
#include "SmartTableColumnSchema.h"
#include "SmartTableTestTypes.h"
#include "SmartTableTypes.h"

#define LOCTEXT_NAMESPACE "SmartTablesTest"

UE_DISABLE_OPTIMIZATION_SHIP

namespace SmartTable::Test
{
    static const FSmartTableColumn * FindColumn( const TArray< FSmartTableColumn > & Columns, FName ColumnId )
    {
        return Columns.FindByPredicate( [ ColumnId ]( const FSmartTableColumn & Column )
        {
            return Column.ColumnId == ColumnId;
        } );
    }

    struct FAStructBecomesOneColumnPerField : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TArray< FSmartTableColumn > Columns = ColumnsFromStruct( FSmartTableTestRow::StaticStruct() );

            AITEST_TRUE( "A struct yields columns", Columns.Num() > 0 );
            AITEST_NOT_NULL( "A text field is one of them", FindColumn( Columns, TEXT( "Callsign" ) ) );

            AITEST_TRUE( "The binding is left to fall back", FindColumn( Columns, TEXT( "Callsign" ) )->BindingName.IsNone() );
            AITEST_EQUAL( "...and resolves to the field", FindColumn( Columns, TEXT( "Callsign" ) )->GetValueBinding(), FName( TEXT( "Callsign" ) ) );

            for ( const FSmartTableColumn & Column : Columns )
            {
                AITEST_TRUE( "Every generated binding resolves", ResolveBinding( FSmartTableTestRow::StaticStruct(), Column.GetValueBinding() ).IsValid() );
            }

            AITEST_EQUAL( "A null struct yields nothing", ColumnsFromStruct( nullptr ).Num(), 0 );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAStructBecomesOneColumnPerField, "SmartTables.Schema.AStructBecomesOneColumnPerField" );

    struct FGeneratedColumnsTakeTheirShapeFromTheType : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TArray< FSmartTableColumn > Columns = ColumnsFromStruct( FSmartTableColumn::StaticStruct() );

            const FSmartTableColumn * Number = FindColumn( Columns, TEXT( "Width" ) );
            AITEST_NOT_NULL( "A float becomes a column", Number );
            AITEST_EQUAL( "...right-aligned", Number->HAlign.GetValue(), HAlign_Right );
            AITEST_EQUAL( "...and fixed", Number->Sizing, ESmartTableColumnSizing::Fixed );

            const FSmartTableColumn * Toggle = FindColumn( Columns, TEXT( "bSortable" ) );
            AITEST_NOT_NULL( "A bool becomes a column", Toggle );
            AITEST_EQUAL( "...centred", Toggle->HAlign.GetValue(), HAlign_Center );

            const FSmartTableColumn * Text = FindColumn( Columns, TEXT( "ColumnId" ) );
            AITEST_NOT_NULL( "A name becomes a column", Text );
            AITEST_EQUAL( "...left-aligned", Text->HAlign.GetValue(), HAlign_Left );
            AITEST_EQUAL( "...and takes the leftover", Text->Sizing, ESmartTableColumnSizing::Fill );

            const FSmartTableColumn * Enum = FindColumn( Columns, TEXT( "HAlign" ) );
            AITEST_NOT_NULL( "A TEnumAsByte becomes a column", Enum );
            AITEST_EQUAL( "...that is not treated as a number", Enum->HAlign.GetValue(), HAlign_Left );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FGeneratedColumnsTakeTheirShapeFromTheType, "SmartTables.Schema.GeneratedColumnsTakeTheirShapeFromTheType" );

    struct FACsvGivesUpOnlyItsHeaderRow : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TArray< FSmartTableColumn > Columns = ColumnsFromCsv( TEXT( "Callsign,Mass,Visited\nVanta-1,120,true\nGarris-2,173,false\n" ) );

            AITEST_EQUAL( "Three headings, three columns", Columns.Num(), 3 );
            AITEST_EQUAL( "In the order they appear", Columns[ 0 ].ColumnId, FName( TEXT( "Callsign" ) ) );

            AITEST_EQUAL( "Nothing is guessed to be a number", Columns[ 1 ].HAlign.GetValue(), HAlign_Left );
            AITEST_EQUAL( "Nor a toggle", Columns[ 2 ].HAlign.GetValue(), HAlign_Left );

            AITEST_EQUAL( "A trailing comma adds no column", ColumnsFromCsv( TEXT( "A,B,\n1,2,3\n" ) ).Num(), 2 );
            AITEST_EQUAL( "An empty file yields nothing", ColumnsFromCsv( FString() ).Num(), 0 );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FACsvGivesUpOnlyItsHeaderRow, "SmartTables.Schema.ACsvGivesUpOnlyItsHeaderRow" );

    struct FAJsonGivesUpTheKeysOfItsFirstObject : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TArray< FSmartTableColumn > FromArray = ColumnsFromJson( TEXT( "[{\"Callsign\":\"Vanta-1\",\"Mass\":120},{\"Callsign\":\"Garris-2\",\"Mass\":173}]" ) );
            AITEST_EQUAL( "An array of objects yields the first one's keys", FromArray.Num(), 2 );
            AITEST_NOT_NULL( "...by name", FindColumn( FromArray, TEXT( "Callsign" ) ) );

            AITEST_EQUAL( "A lone object works too", ColumnsFromJson( TEXT( "{\"A\":1,\"B\":2}" ) ).Num(), 2 );

            const TArray< FSmartTableColumn > Ragged = ColumnsFromJson( TEXT( "[{\"A\":1},{\"B\":2}]" ) );
            AITEST_EQUAL( "A ragged document follows the first row", Ragged.Num(), 1 );
            AITEST_NOT_NULL( "...and only that one", FindColumn( Ragged, TEXT( "A" ) ) );

            AITEST_EQUAL( "Malformed JSON yields nothing", ColumnsFromJson( TEXT( "{not json" ) ).Num(), 0 );
            AITEST_EQUAL( "An empty array yields nothing", ColumnsFromJson( TEXT( "[]" ) ).Num(), 0 );
            AITEST_EQUAL( "A bare number is not a row", ColumnsFromJson( TEXT( "42" ) ).Num(), 0 );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAJsonGivesUpTheKeysOfItsFirstObject, "SmartTables.Schema.AJsonGivesUpTheKeysOfItsFirstObject" );

    struct FCopyingTwiceDisturbsNothingAuthored : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TArray< FSmartTableColumn > Candidates = ColumnsFromCsv( TEXT( "A,B,C\n" ) );

            TArray< FSmartTableColumn > Columns;
            AITEST_EQUAL( "The first copy brings everything", AppendMissingColumns( Columns, Candidates ), 3 );

            Columns.RemoveAll( []( const FSmartTableColumn & Column )
            {
                return Column.ColumnId == FName( TEXT( "B" ) );
            } );
            Columns[ 0 ].Width = 480.0f;

            AITEST_EQUAL( "The second copy brings back only what is missing", AppendMissingColumns( Columns, Candidates ), 1 );
            AITEST_EQUAL( "...leaving three", Columns.Num(), 3 );
            AITEST_EQUAL( "An authored width survives", Columns[ 0 ].Width, 480.0f );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FCopyingTwiceDisturbsNothingAuthored, "SmartTables.Schema.CopyingTwiceDisturbsNothingAuthored" );

    struct FASecondColumnSharingAnIdIsFoundOnce : FAITestBase
    {
        virtual bool InstantTest() override
        {
            TArray< FSmartTableColumn > Columns;
            Columns.AddDefaulted_GetRef().ColumnId = FName( TEXT( "Callsign" ) );
            Columns.AddDefaulted_GetRef().ColumnId = FName( TEXT( "Shield" ) );
            Columns.AddDefaulted_GetRef().ColumnId = FName( TEXT( "Callsign" ) );

            const TSet< FName > Doubled = DuplicatedColumnIds( Columns );

            AITEST_EQUAL( "One id is doubled", Doubled.Num(), 1 );
            AITEST_TRUE( "...and it is the one that repeats", Doubled.Contains( FName( TEXT( "Callsign" ) ) ) );

            Columns.AddDefaulted_GetRef().ColumnId = FName( TEXT( "Callsign" ) );
            AITEST_EQUAL( "A third copy is still one id to fix", DuplicatedColumnIds( Columns ).Num(), 1 );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FASecondColumnSharingAnIdIsFoundOnce, "SmartTables.Schema.ASecondColumnSharingAnIdIsFoundOnce" );

    struct FUnnamedColumnsAreNotDuplicatesOfEachOther : FAITestBase
    {
        virtual bool InstantTest() override
        {
            TArray< FSmartTableColumn > Columns;
            Columns.AddDefaulted();
            Columns.AddDefaulted();
            Columns.AddDefaulted_GetRef().ColumnId = FName( TEXT( "Shield" ) );

            AITEST_EQUAL( "Two columns with no id are two unfinished columns, not a clash", DuplicatedColumnIds( Columns ).Num(), 0 );
            AITEST_EQUAL( "A well-formed set reports nothing", DuplicatedColumnIds( TArray< FSmartTableColumn >() ).Num(), 0 );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FUnnamedColumnsAreNotDuplicatesOfEachOther, "SmartTables.Schema.UnnamedColumnsAreNotDuplicatesOfEachOther" );

    struct FAnItemClassSourceResolvesToTheClass : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FText Name;
            FText Problem;

            const UStruct * Schema = ResolveColumnSchema( ESmartTableColumnSource::ItemClass, USmartTableTestCountingModel::StaticClass(), nullptr, Name, Problem );

            AITEST_TRUE( "The class is the schema", Schema == USmartTableTestCountingModel::StaticClass() );
            AITEST_TRUE( "...and nothing is wrong", Problem.IsEmpty() );
            AITEST_FALSE( "...and it is named for the panel", Name.IsEmpty() );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAnItemClassSourceResolvesToTheClass, "SmartTables.ColumnSchema.AnItemClassSourceResolvesToTheClass" );

    struct FASourceNamingNothingSaysWhichOneIsUnset : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FText Name;
            FText NoClass;
            FText NoTable;

            AITEST_NULL( "Item Class with no class resolves to nothing", ResolveColumnSchema( ESmartTableColumnSource::ItemClass, nullptr, nullptr, Name, NoClass ) );
            AITEST_FALSE( "...and says which value is unset", NoClass.IsEmpty() );

            AITEST_NULL( "Data Table with no table resolves to nothing", ResolveColumnSchema( ESmartTableColumnSource::DataTable, nullptr, nullptr, Name, NoTable ) );
            AITEST_FALSE( "...and says which value is unset", NoTable.IsEmpty() );

            AITEST_TRUE( "The two problems are told apart", !NoClass.EqualTo( NoTable ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FASourceNamingNothingSaysWhichOneIsUnset, "SmartTables.ColumnSchema.ASourceNamingNothingSaysWhichOneIsUnset" );

    struct FAMissingDataTableIsToldApartFromOneNeverSet : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FText Name;
            FText NeverSet;
            FText Missing;

            ResolveColumnSchema( ESmartTableColumnSource::DataTable, nullptr, nullptr, Name, NeverSet );

            const TSoftObjectPtr< UDataTable > Gone( FSoftObjectPath( TEXT( "/Game/NoSuchPackage.NoSuchTable" ) ) );

            AITEST_NULL( "A table that no longer exists resolves to nothing", ResolveColumnSchema( ESmartTableColumnSource::DataTable, nullptr, Gone, Name, Missing ) );
            AITEST_FALSE( "...and says so", Missing.IsEmpty() );
            AITEST_TRUE( "...in words a table never set does not use", !Missing.EqualTo( NeverSet ) );
            AITEST_TRUE( "...naming the path, which is all that is left of it", Missing.ToString().Contains( TEXT( "NoSuchTable" ) ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAMissingDataTableIsToldApartFromOneNeverSet, "SmartTables.ColumnSchema.AMissingDataTableIsToldApartFromOneNeverSet" );

    struct FAFileSourceResolvesToNothingAndIsNotAProblem : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FText Name;
            FText Problem;

            AITEST_NULL( "A file has headings and not a struct", ResolveColumnSchema( ESmartTableColumnSource::File, nullptr, nullptr, Name, Problem ) );
            AITEST_TRUE( "...which is a supported table and not a fault", Problem.IsEmpty() );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAFileSourceResolvesToNothingAndIsNotAProblem, "SmartTables.ColumnSchema.AFileSourceResolvesToNothingAndIsNotAProblem" );

#if WITH_EDITOR
    struct FSwitchingSourceNamesEveryFieldItWouldClear : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TSubclassOf< UObject > ItemClass = UObject::StaticClass();
            const TSoftObjectPtr< UDataTable > Table( FSoftObjectPath( TEXT( "/Game/Data/DT_Ships.DT_Ships" ) ) );

            FFilePath File;
            File.FilePath = TEXT( "C:/Data/ships.csv" );

            const FString ToFile = DescribeSourcesLost( ESmartTableColumnSource::File, ItemClass, Table, File ).ToString();
            AITEST_TRUE( "Switching to a file names the item class it clears", ToFile.Contains( TEXT( "the Item Class (Object)" ) ) );
            AITEST_TRUE( "...and the data table, by the name in its path", ToFile.Contains( TEXT( "the Data Table (DT_Ships)" ) ) );
            AITEST_FALSE( "...and never the file it switches to", ToFile.Contains( TEXT( "the File" ) ) );

            const FString ToClass = DescribeSourcesLost( ESmartTableColumnSource::ItemClass, ItemClass, Table, File ).ToString();
            AITEST_EQUAL( "Switching to an item class names the other two in one line", ToClass, FString( TEXT( "the Data Table (DT_Ships) and the File (ships.csv)" ) ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FSwitchingSourceNamesEveryFieldItWouldClear, "SmartTables.ColumnSchema.SwitchingSourceNamesEveryFieldItWouldClear" );

    struct FSwitchingToTheSourceInUseLosesNothing : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const FFilePath NoFile;

            AITEST_TRUE( "Switching to the source in use clears nothing", DescribeSourcesLost( ESmartTableColumnSource::ItemClass, UObject::StaticClass(), TSoftObjectPtr< UDataTable >(), NoFile ).IsEmpty() );
            AITEST_TRUE( "Fields left unset clear nothing either", DescribeSourcesLost( ESmartTableColumnSource::File, TSubclassOf< UObject >(), TSoftObjectPtr< UDataTable >(), NoFile ).IsEmpty() );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FSwitchingToTheSourceInUseLosesNothing, "SmartTables.ColumnSchema.SwitchingToTheSourceInUseLosesNothing" );

    struct FARelativeSourceFileIsReadFromTheProjectFolder : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FFilePath Relative;
            Relative.FilePath = TEXT( "Content/Data/ships.csv" );

            const FString ProjectFolder = FPaths::ConvertRelativePathToFull( FPaths::ProjectDir() );
            const FString Resolved      = ResolveSourceFile( Relative );

            AITEST_FALSE( "A relative path comes back full", FPaths::IsRelative( Resolved ) );
            AITEST_TRUE( "...under the project folder and not the engine's binaries", FPaths::IsUnderDirectory( Resolved, ProjectFolder ) );
            AITEST_EQUAL( "...naming the file the path names", Resolved, ProjectFolder / TEXT( "Content/Data/ships.csv" ) );

            FFilePath Absolute;
            Absolute.FilePath = TEXT( "C:/Data/ships.csv" );

            AITEST_EQUAL( "An absolute path is read as it is", ResolveSourceFile( Absolute ), Absolute.FilePath );
            AITEST_TRUE( "No file set resolves to nothing", ResolveSourceFile( FFilePath() ).IsEmpty() );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FARelativeSourceFileIsReadFromTheProjectFolder, "SmartTables.ColumnSchema.ARelativeSourceFileIsReadFromTheProjectFolder" );

    struct FABindingThatResolvesIsNotReported : FAITestBase
    {
        virtual bool InstantTest() override
        {
            TArray< FSmartTableColumn > Columns;
            Columns.AddDefaulted_GetRef().ColumnId = TEXT( "Width" );
            Columns.AddDefaulted_GetRef().ColumnId = TEXT( "Nope" );

            const TArray< FUnresolvedBinding > Unresolved = FindUnresolvedBindings( FSmartTableColumn::StaticStruct(), Columns );

            AITEST_EQUAL( "Only the binding with nothing behind it is reported", Unresolved.Num(), 1 );
            AITEST_TRUE( "...under its own column", Unresolved[ 0 ].ColumnId == TEXT( "Nope" ) && Unresolved[ 0 ].Binding == TEXT( "Nope" ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FABindingThatResolvesIsNotReported, "SmartTables.ColumnSchema.ABindingThatResolvesIsNotReported" );

    struct FASortBindingThatFallsBackIsReportedOnce : FAITestBase
    {
        virtual bool InstantTest() override
        {
            TArray< FSmartTableColumn > Columns;
            Columns.AddDefaulted_GetRef().ColumnId = TEXT( "Nope" );

            FSmartTableColumn & Sorted = Columns.AddDefaulted_GetRef();
            Sorted.ColumnId            = TEXT( "Width" );
            Sorted.SortBindingName     = TEXT( "Missing" );

            const TArray< FUnresolvedBinding > Unresolved = FindUnresolvedBindings( FSmartTableColumn::StaticStruct(), Columns );

            AITEST_EQUAL( "A sort binding that falls back is reported once, and a sort binding of its own once", Unresolved.Num(), 2 );
            AITEST_TRUE( "...the first under the column whose one binding is missing", Unresolved[ 0 ].ColumnId == TEXT( "Nope" ) );
            AITEST_TRUE( "...the second naming the sort binding that is missing", Unresolved[ 1 ].ColumnId == TEXT( "Width" ) && Unresolved[ 1 ].Binding == TEXT( "Missing" ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FASortBindingThatFallsBackIsReportedOnce, "SmartTables.ColumnSchema.ASortBindingThatFallsBackIsReportedOnce" );
#endif
}
UE_ENABLE_OPTIMIZATION_SHIP

#undef LOCTEXT_NAMESPACE
#endif
