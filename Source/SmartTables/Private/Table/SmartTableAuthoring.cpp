// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "Editor/WidgetCompilerLog.h"
#include "Engine/DataTable.h"
#include "Logging/StructuredLog.h"
#include "Misc/FileHelper.h"
#include "Model/SmartTablePreviewModel.h"
#include "SmartTable.h"
#include "SmartTableBinding.h"
#include "SmartTableColumnSchema.h"
#include "SmartTableLog.h"

#define LOCTEXT_NAMESPACE "SmartTables"

void USmartTable::ApplyDesignTimePreview()
{
    if ( !IsDesignTime() )
    {
        return;
    }

    if ( !PreviewModel )
    {
        PreviewModel = NewObject< USmartTablePreviewModel >( this );
    }

    FText Unused;
    FText Ignored;
    const UStruct * Schema = ResolveColumnSchema( Unused, Ignored );

    const UDataTable * Rows = ColumnSource == ESmartTableColumnSource::DataTable ? SourceTable.LoadSynchronous() : nullptr;

    PreviewModel->SetPreview( Columns, Schema, Rows, PreviewRowCount );
    SetModel( PreviewModel );
}

TArray< FName > USmartTable::GetBindableNames() const
{

    FText Unused;
    FText Ignored;
    const UStruct * Schema = ResolveColumnSchema( Unused, Ignored );

    return Schema ? SmartTable::GetBindableNames( Schema ) : TArray< FName >();
}

const UStruct * USmartTable::ResolveColumnSchema( FText & OutSchemaName, FText & OutProblem ) const
{
    return SmartTable::ResolveColumnSchema( ColumnSource, ExpectedItemClass, SourceTable, OutSchemaName, OutProblem );
}

#if WITH_EDITOR
const FText USmartTable::GetPaletteCategory()
{
    return LOCTEXT( "PaletteCategory", "Smart Tables" );
}

FText USmartTable::DescribeSourcesLostBySwitchingTo( ESmartTableColumnSource NewSource ) const
{
    return SmartTable::DescribeSourcesLost( NewSource, ExpectedItemClass, SourceTable, SourceFile );
}

void USmartTable::PostEditChangeProperty( FPropertyChangedEvent & PropertyChangedEvent )
{
    Super::PostEditChangeProperty( PropertyChangedEvent );

    if ( LayoutStore && TableId.IsNone() )
    {
        TableId = GetFName();
    }
}

TArray< FSmartTableColumn > USmartTable::MergedColumnsFromSource() const
{
    TArray< FSmartTableColumn > Candidates;

    if ( ColumnSource == ESmartTableColumnSource::File )
    {
        const FString Path = SmartTable::ResolveSourceFile( SourceFile );
        FString Contents;
        if ( Path.IsEmpty() || !FFileHelper::LoadFileToString( Contents, *Path ) )
        {
            UE_LOGFMT( LogSmartTablesLayout, Warning, "No columns to copy: {Reason}. Point SourceFile at a file that exists.", Path.IsEmpty() ? TEXT( "the mode is File but no file is set" ) : *FString::Printf( TEXT( "'%s' could not be read" ), *Path ) );

            return Columns;
        }

        Candidates = SourceFile.FilePath.EndsWith( TEXT( ".json" ) ) ? SmartTable::ColumnsFromJson( Contents ) : SmartTable::ColumnsFromCsv( Contents );
    }
    else
    {
        FText SourceName;
        FText Problem;
        const UStruct * Schema = ResolveColumnSchema( SourceName, Problem );
        if ( !Schema )
        {
            UE_LOGFMT( LogSmartTablesLayout, Warning, "Copy of columns failed: {Problem}.", Problem.IsEmpty() ? TEXT( "this table names no source - set ColumnSource first" ) : *Problem.ToString() );

            return Columns;
        }

        Candidates = SmartTable::ColumnsFromStruct( Schema );
    }

    TArray< FSmartTableColumn > Merged = Columns;
    SmartTable::AppendMissingColumns( Merged, Candidates );

    return Merged;
}

void USmartTable::ValidateCompiledDefaults( IWidgetCompilerLog & CompileLog ) const
{

    for ( const FName ColumnId : SmartTable::DuplicatedColumnIds( Columns ) )
    {
        CompileLog.Error( FText::Format( LOCTEXT( "DuplicateColumnId", "Smart Table has more than one column with the ColumnId '{0}'. Every lookup takes the FIRST, so the later one is invisible - it never draws, sorts or hides, however it is authored. Give it its own ColumnId, or delete it." ), FText::FromName( ColumnId ) ) );
    }

    FText SchemaName;
    FText Problem;
    const UStruct * Schema = ResolveColumnSchema( SchemaName, Problem );

    if ( !Problem.IsEmpty() )
    {
        CompileLog.Error( FText::Format( LOCTEXT( "ColumnSourceUnusable", "Smart Table '{0}' cannot be checked: {1}." ), FText::FromString( GetName() ), Problem ) );

        return;
    }

    if ( !Schema )
    {
        return;
    }

    const FText Available = FText::FromString( SmartTable::DescribeBindableNames( Schema ) );

    for ( const SmartTable::FUnresolvedBinding & Unresolved : SmartTable::FindUnresolvedBindings( Schema, Columns ) )
    {
        CompileLog.Error( FText::Format( LOCTEXT( "BindingDoesNotResolve", "Smart Table column '{0}' binds '{1}', which is not a property or a pure parameterless function on {2}. Available: {3}" ), FText::FromName( Unresolved.ColumnId ), FText::FromName( Unresolved.Binding ), SchemaName, Available ) );
    }
}
#endif

#undef LOCTEXT_NAMESPACE
