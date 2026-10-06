// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "Logging/StructuredLog.h"
#include "SmartTable.h"
#include "SmartTableHelpers.h"
#include "SmartTableLayoutStore.h"
#include "SmartTableLog.h"

void USmartTable::SetTableId( FName InTableId )
{
    if ( TableId == InTableId )
    {
        return;
    }

    UE_LOGFMT( LogSmartTablesLayout, Verbose, "Table id '{From}' becomes '{To}'. Any layout on file comes back under the new one.", TableId, InTableId );

    TableId = InTableId;

    ActiveLayout = FSmartTableLayout();
    MarkColumnWidthsDirty();
    bMissingTableIdWarned = false;

    RestoreStoredLayout();
}

bool USmartTable::LoadStoredLayout()
{
    if ( !LayoutStore || TableId.IsNone() || IsDesignTime() )
    {
        return false;
    }

    FSmartTableLayout Stored;
    if ( LayoutStore->LoadLayout( TableId, Stored ) )
    {
        const int32 StoredVersion = Stored.Version;
        if ( SmartTable::MigrateStoredLayout( Stored ) )
        {
            UE_LOGFMT( LogSmartTablesLayout, Verbose, "The layout on file for '{Table}' is version {Old}. Column widths from before width resolving went, hiding and order and sort stayed.", TableId, StoredVersion );
        }

        if ( StoredVersion < 2 && Stored.Order.IsEmpty() && Stored.Columns.Num() == Columns.Num() )
        {
            Stored.Order.Reserve( Stored.Columns.Num() );
            for ( const FSmartTableColumnLayout & Column : Stored.Columns )
            {
                Stored.Order.Add( Column.ColumnId );
            }
        }

        ActiveLayout = Stored;

        UE_LOGFMT( LogSmartTablesLayout, Verbose, "The layout on file for '{Table}' is back: {Columns} column override(s), {SortLevels} sort level(s).", TableId, ActiveLayout.Columns.Num(), ActiveLayout.SortSpec.Columns.Num() );

        return true;
    }

    UE_LOGFMT( LogSmartTablesLayout, Verbose, "'{Table}' has nothing on file yet. The authored columns stand.", TableId );

    return false;
}

void USmartTable::RestoreStoredLayout()
{
    const bool bRestored = LoadStoredLayout();

    RebuildHeader();

    if ( bRestored )
    {

        SetSortSpec( ActiveLayout.SortSpec );

        return;
    }

    ActiveLayout.SortSpec = GetSortSpecRef();
}

void USmartTable::SetLayoutStore( USmartTableLayoutStore * InLayoutStore )
{
    if ( LayoutStore == InLayoutStore )
    {
        return;
    }

    LayoutStore = InLayoutStore;

    UE_LOGFMT( LogSmartTablesLayout, Verbose, "'{Table}' files its layout in {Store} from here on.", GetName(), GetNameSafe( InLayoutStore ) );

    if ( LayoutStore && !TableId.IsNone() )
    {
        RestoreStoredLayout();
    }
}

FSmartTableColumnLayout & USmartTable::LayoutFor( FName ColumnId )
{
    FSmartTableColumnLayout * Existing = SmartTable::FindByColumnId( ActiveLayout.Columns, ColumnId );

    if ( Existing )
    {
        return *Existing;
    }

    FSmartTableColumnLayout & Added = ActiveLayout.Columns.AddDefaulted_GetRef();
    Added.ColumnId                  = ColumnId;

    if ( const FSmartTableColumn * Column = FindColumn( ColumnId ) )
    {
        Added.bHidden = Column->bHiddenByDefault;
    }

    return Added;
}

void USmartTable::LayoutChangedByUser()
{
    if ( LayoutStore )
    {
        if ( TableId.IsNone() )
        {

            if ( !bMissingTableIdWarned )
            {
                bMissingTableIdWarned = true;

                UE_LOGFMT( LogSmartTablesLayout, Warning, "Table '{Table}' has a LayoutStore and no TableId, so nothing can be filed under it and no column layout is ever written. Set TableId on the widget, or clear the LayoutStore.", GetName() );
            }
        }
        else
        {
            UE_LOGFMT( LogSmartTablesLayout, Verbose, "The layout for '{Table}' goes on file: {Columns} column override(s), {SortLevels} sort level(s).", TableId, ActiveLayout.Columns.Num(), ActiveLayout.SortSpec.Columns.Num() );

            ActiveLayout.Version = FSmartTableLayout::CurrentVersion;

            LayoutStore->SaveLayout( TableId, ActiveLayout );
        }
    }

    OnLayoutChanged.Broadcast( ActiveLayout );
}

FSmartTableLayout USmartTable::GetLayout() const
{
    return ActiveLayout;
}

void USmartTable::ApplyLayout( const FSmartTableLayout & Layout )
{
    ActiveLayout = Layout;

    UE_LOGFMT( LogSmartTablesLayout, Verbose, "A layout went onto '{Table}': {Columns} column override(s), {SortLevels} sort level(s).", GetName(), Layout.Columns.Num(), Layout.SortSpec.Columns.Num() );

    RebuildHeader();
    SetSortSpec( Layout.SortSpec );
}

void USmartTable::ResetLayout()
{
    UE_LOGFMT( LogSmartTablesLayout, Verbose, "Layout cleared for '{Table}'. Back to the authored widths, visibility and sort.", GetName() );

    ActiveLayout = FSmartTableLayout();

    MarkColumnWidthsDirty();

    RebuildHeader();
    SetSortSpec( FSmartTableSortSpec() );
    LayoutChangedByUser();
}
