// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTableColumnSchema.h"

#include "SmartTableConstants.h"

#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Engine/DataTable.h"
#include "Logging/StructuredLog.h"
#include "Misc/AssertionMacros.h"
#include "Misc/Paths.h"
#include "Serialization/Csv/CsvParser.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "SmartTableBinding.h"
#include "SmartTableLog.h"
#include "UObject/UnrealType.h"

#define LOCTEXT_NAMESPACE "SmartTable"

namespace
{

    FSmartTableColumn TextColumn( FName ColumnId, const FText & Header )
    {
        FSmartTableColumn Column;
        Column.ColumnId = ColumnId;
        Column.Header   = Header;

        Column.Sizing = ESmartTableColumnSizing::Fill;
        Column.Width  = 1.0f;

        return Column;
    }

    void MakeNumeric( FSmartTableColumn & Column )
    {
        Column.Sizing = ESmartTableColumnSizing::Fixed;
        Column.Width  = SmartTable::Metrics::NumericColumnWidth;
        Column.HAlign = HAlign_Right;
    }

    void MakeToggle( FSmartTableColumn & Column )
    {
        Column.Sizing = ESmartTableColumnSizing::Fixed;
        Column.Width  = SmartTable::Metrics::ToggleColumnWidth;
        Column.HAlign = HAlign_Center;
    }
}

TArray< FSmartTableColumn > SmartTable::ColumnsFromStruct( const UStruct * Struct )
{
    TArray< FSmartTableColumn > Columns;
    if ( !Struct )
    {
        return Columns;
    }

    for ( TFieldIterator< FProperty > It( Struct ); It; ++It )
    {
        const FProperty * Property = *It;

        const FString AuthoredName = Property->GetAuthoredName();

#if WITH_EDITORONLY_DATA
        const FText Header = Property->GetDisplayNameText();
#else
        const FText Header = FText::FromString( FName::NameToDisplayString( AuthoredName, Property->IsA< FBoolProperty >() ) );
#endif

        FSmartTableColumn Column = TextColumn( FName( *AuthoredName ), Header );

        switch ( SmartTable::Classify( Property ) )
        {
            case SmartTable::EBoundKind::Bool:
                MakeToggle( Column );
                break;

            case SmartTable::EBoundKind::Number:
                MakeNumeric( Column );
                break;

            default:
                break;
        }

        Columns.Add( Column );
    }

    return Columns;
}

TArray< FSmartTableColumn > SmartTable::ColumnsFromCsv( const FString & Csv )
{
    TArray< FSmartTableColumn > Columns;

    const FCsvParser Parser( Csv );
    const FCsvParser::FRows & Rows = Parser.GetRows();
    if ( Rows.IsEmpty() )
    {
        UE_LOGFMT( LogSmartTablesData, Warning, "That CSV holds no rows, so there is no header to take columns from. The FIRST row is the header, one column name per cell: Callsign,Mass,Speed." );
        return Columns;
    }

    for ( const TCHAR * Cell : Rows[ 0 ] )
    {
        const FString Header = FString( Cell ).TrimStartAndEnd();
        if ( Header.IsEmpty() )
        {

            continue;
        }

        Columns.Add( TextColumn( FName( *Header ), FText::FromString( Header ) ) );
    }

    return Columns;
}

TArray< FSmartTableColumn > SmartTable::ColumnsFromJson( const FString & Json )
{
    TArray< FSmartTableColumn > Columns;

    const TSharedRef< TJsonReader<> > Reader = TJsonReaderFactory<>::Create( Json );

    TSharedPtr< FJsonValue > Parsed;
    if ( !FJsonSerializer::Deserialize( Reader, Parsed ) || !Parsed.IsValid() )
    {
        UE_LOGFMT( LogSmartTablesData, Warning, "That JSON would not parse, and gave no columns: {Error}", Reader->GetErrorMessage() );
        return Columns;
    }

    const TSharedPtr< FJsonObject > * First = nullptr;
    if ( const TArray< TSharedPtr< FJsonValue > > * AsArray = nullptr; Parsed->TryGetArray( AsArray ) )
    {
        if ( AsArray->IsEmpty() || !( *AsArray )[ 0 ]->TryGetObject( First ) )
        {
            UE_LOGFMT( LogSmartTablesData, Warning, "That JSON is an array with no object in it to take columns from. Its first entry should be one row, an object whose field names are the columns, the way a writer writes a row." );
            return Columns;
        }
    }
    else if ( !Parsed->TryGetObject( First ) )
    {
        UE_LOGFMT( LogSmartTablesData, Warning, "That JSON is neither an object nor an array of them, and holds no columns. Expect one row as an object whose field names are columns, or an array of such rows." );
        return Columns;
    }

    for ( const TPair< FString, TSharedPtr< FJsonValue > > & Field : ( *First )->Values )
    {
        Columns.Add( TextColumn( FName( *Field.Key ), FText::FromString( Field.Key ) ) );
    }

    return Columns;
}

int32 SmartTable::AppendMissingColumns( TArray< FSmartTableColumn > & Columns, TArrayView< const FSmartTableColumn > Candidates )
{
    int32 Added = 0;

    for ( const FSmartTableColumn & Candidate : Candidates )
    {
        const bool bAlreadyDeclared = Columns.ContainsByPredicate( [ &Candidate ]( const FSmartTableColumn & Existing )
        {
            return Existing.ColumnId == Candidate.ColumnId;
        } );

        if ( bAlreadyDeclared )
        {
            continue;
        }

        Columns.Add( Candidate );
        ++Added;
    }

    return Added;
}

TSet< FName > SmartTable::DuplicatedColumnIds( TArrayView< const FSmartTableColumn > Columns )
{
    TSet< FName > Seen;
    Seen.Reserve( Columns.Num() );

    TSet< FName > Duplicated;

    for ( const FSmartTableColumn & Column : Columns )
    {
        if ( Column.ColumnId.IsNone() )
        {
            continue;
        }

        bool bAlreadySeen = false;
        Seen.Add( Column.ColumnId, &bAlreadySeen );

        if ( bAlreadySeen )
        {
            Duplicated.Add( Column.ColumnId );
        }
    }

    return Duplicated;
}

const UStruct * SmartTable::ResolveColumnSchema( ESmartTableColumnSource Source, const TSubclassOf< UObject > & ItemClass, const TSoftObjectPtr< UDataTable > & Table, FText & OutSchemaName, FText & OutProblem )
{
    switch ( Source )
    {
        case ESmartTableColumnSource::ItemClass:
            if ( !ItemClass )
            {
                OutProblem = LOCTEXT( "NoItemClass", "the mode is Item Class but no class is set" );
                return nullptr;
            }

            OutSchemaName = FText::FromString( ItemClass->GetName() );
            return ItemClass;

        case ESmartTableColumnSource::DataTable:
        {
            if ( Table.IsNull() )
            {
                OutProblem = LOCTEXT( "NoSourceTable", "the mode is Data Table but no table is set" );
                return nullptr;
            }

            const UDataTable * Linked = Table.LoadSynchronous();
            if ( !Linked )
            {

                OutProblem = FText::Format( LOCTEXT( "SourceTableMissing", "the DataTable '{0}' no longer exists" ), FText::FromString( Table.ToSoftObjectPath().ToString() ) );

                return nullptr;
            }

            if ( !Linked->RowStruct )
            {
                OutProblem = FText::Format( LOCTEXT( "SourceTableNoRowStruct", "the DataTable '{0}' has no row struct, so no column can bind to anything" ), FText::FromString( Linked->GetName() ) );

                return nullptr;
            }

            OutSchemaName = FText::FromString( Linked->RowStruct->GetName() );
            return Linked->RowStruct;
        }

        default:
            return nullptr;
    }
}

#if WITH_EDITOR
FText SmartTable::DescribeSourcesLost( ESmartTableColumnSource NewSource, const TSubclassOf< UObject > & ItemClass, const TSoftObjectPtr< UDataTable > & SourceTable, const FFilePath & SourceFile )
{
    TArray< FText > Lost;

    if ( NewSource != ESmartTableColumnSource::ItemClass && ItemClass )
    {
        Lost.Add( FText::Format( NSLOCTEXT( "SmartTables", "LostItemClass", "the Item Class ({0})" ), FText::FromString( ItemClass->GetName() ) ) );
    }

    if ( NewSource != ESmartTableColumnSource::DataTable && !SourceTable.IsNull() )
    {
        Lost.Add( FText::Format( NSLOCTEXT( "SmartTables", "LostSourceTable", "the Data Table ({0})" ), FText::FromString( SourceTable.ToSoftObjectPath().GetAssetName() ) ) );
    }

    if ( NewSource != ESmartTableColumnSource::File && !SourceFile.FilePath.IsEmpty() )
    {
        Lost.Add( FText::Format( NSLOCTEXT( "SmartTables", "LostSourceFile", "the File ({0})" ), FText::FromString( FPaths::GetCleanFilename( SourceFile.FilePath ) ) ) );
    }

    return Lost.IsEmpty() ? FText::GetEmpty() : FText::Join( NSLOCTEXT( "SmartTables", "LostJoin", " and " ), Lost );
}

FString SmartTable::ResolveSourceFile( const FFilePath & SourceFile )
{
    if ( SourceFile.FilePath.IsEmpty() || !FPaths::IsRelative( SourceFile.FilePath ) )
    {
        return SourceFile.FilePath;
    }

    return FPaths::ConvertRelativePathToFull( FPaths::ProjectDir() / SourceFile.FilePath );
}

TArray< SmartTable::FUnresolvedBinding > SmartTable::FindUnresolvedBindings( const UStruct * Schema, TArrayView< const FSmartTableColumn > Columns )
{
    checkf( Schema, TEXT( "FindUnresolvedBindings was given no schema to check the bindings against" ) );

    TArray< FUnresolvedBinding > Unresolved;

    for ( const FSmartTableColumn & Column : Columns )
    {
        TArray< FName, TInlineAllocator< 2 > > Bindings;
        Bindings.Add( Column.GetValueBinding() );
        Bindings.AddUnique( Column.GetSortBinding() );

        for ( const FName Binding : Bindings )
        {
            if ( !Binding.IsNone() && !ResolveBinding( Schema, Binding ).IsValid() )
            {
                Unresolved.Add( { Column.ColumnId, Binding } );
            }
        }
    }

    return Unresolved;
}
#endif

#undef LOCTEXT_NAMESPACE
