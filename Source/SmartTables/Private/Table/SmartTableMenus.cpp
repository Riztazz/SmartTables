// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "Framework/Application/SlateApplication.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Layout/WidgetPath.h"
#include "Logging/StructuredLog.h"
#include "SmartTable.h"
#include "SmartTableLog.h"
#include "SmartTableModel.h"
#include "SmartTableOpenMenu.h"
#include "Styling/SlateBrush.h"
#include "Table/SmartTableColumnView.h"
#include "View/SmartTableListView.h"
#include "Widgets/Input/SMenuAnchor.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SWindow.h"
#include "Widgets/Views/ITableRow.h"
#include "Widgets/Views/SHeaderRow.h"

#define LOCTEXT_NAMESPACE "SmartTables"

namespace
{

    class SSmartTableMenuHost : public SCompoundWidget
    {
    public:
        SLATE_BEGIN_ARGS( SSmartTableMenuHost )
            : _Background( nullptr )
        {
        }
        SLATE_ARGUMENT( const FSlateBrush *, Background )
        SLATE_ARGUMENT( TWeakObjectPtr< USmartTable >, Table )
        SLATE_DEFAULT_SLOT( FArguments, Content )
        SLATE_END_ARGS()

        void Construct( const FArguments & InArgs )
        {
            Table = InArgs._Table;

            // clang-format off
            ChildSlot
            [
                SNew( SBorder )
                    .BorderImage( InArgs._Background )
                    .Padding( 0.0f )
                    [
                        InArgs._Content.Widget
                    ]
            ];
            // clang-format on
        }

        virtual FReply OnMouseMove( const FGeometry & Geometry, const FPointerEvent & MouseEvent ) override
        {
            OpenHoveredFlyouts();

            return SCompoundWidget::OnMouseMove( Geometry, MouseEvent );
        }

        virtual FPopupMethodReply OnQueryPopupMethod() const override
        {

            return FPopupMethodReply::UseMethod( EPopupMethod::UseCurrentWindow );
        }

    private:

        void OpenHoveredFlyouts()
        {
            if ( Anchors.IsEmpty() )
            {

                GatherAnchors( AsShared() );
            }

            const USmartTable * Owner = Table.Get();
            const int32 User          = Owner ? Owner->GetMenuDrivingUser() : INDEX_NONE;

            for ( const TWeakPtr< SMenuAnchor > & Weak : Anchors )
            {
                const TSharedPtr< SMenuAnchor > Anchor = Weak.Pin();

                if ( Anchor.IsValid() && Anchor->IsHovered() && !Anchor->IsOpen() )
                {
                    Anchor->SetIsOpen( true, true, User );
                }
            }
        }

        void GatherAnchors( const TSharedRef< SWidget > & Widget )
        {
            static const FName MenuAnchorType( TEXT( "SMenuAnchor" ) );

            if ( Widget->GetType() == MenuAnchorType )
            {

                Anchors.Add( StaticCastSharedRef< SMenuAnchor >( Widget ) );

                return;
            }

            FChildren * Children = Widget->GetChildren();
            for ( int32 Index = 0; Index < Children->Num(); ++Index )
            {
                GatherAnchors( Children->GetChildAt( Index ) );
            }
        }

        TWeakObjectPtr< USmartTable > Table;

        TArray< TWeakPtr< SMenuAnchor > > Anchors;
    };

    FVector2D InMenuStackSpace( const TSharedPtr< SWindow > & Window, FVector2D Absolute )
    {
        if ( !Window.IsValid() )
        {
            return Absolute;
        }

        const FGeometry & Drawn = Window->GetTickSpaceGeometry();
        const FVector2D Size    = FVector2D( Drawn.GetLocalSize() );

        if ( Size.X <= 0.0f || Size.Y <= 0.0f )
        {

            return Absolute;
        }

        return FVector2D( Window->GetLocalToScreenTransform().TransformPoint( FVector2f( Drawn.AbsoluteToLocal( Absolute ) ) ) );
    }
}

FMenuBuilder USmartTable::MakeMenuBuilder()
{

    return FMenuBuilder( true, nullptr, nullptr, false, &MenuHost.Style( MenuStyle, FName( *( TEXT( "SmartTableMenu." ) + GetPathName() ) ) ) );
}

TSharedRef< SWidget > USmartTable::HostMenu( FMenuBuilder & Menu )
{

    return SNew( SSmartTableMenuHost ).Background( &MenuStyle.BackgroundBrush ).Table( this )[ Menu.MakeWidget() ];
}

void USmartTable::AppendTableWideEntries( FMenuBuilder & Menu )
{

    if ( HeaderMenuOptions.bOfferVisibility )
    {

        Menu.AddWrapperSubMenu( LOCTEXT( "ShowColumns", "Show Columns" ), FText::GetEmpty(), FOnGetContent::CreateUObject( this, &USmartTable::MakeColumnVisibilityMenu ), FSlateIcon() );
    }

    if ( HeaderMenuOptions.bOfferResetLayout )
    {
        Menu.AddMenuEntry( LOCTEXT( "ResetColumns", "Reset Columns" ), LOCTEXT( "ResetColumnsTip", "Widths, hidden columns and sort, back to how the table was authored." ), FSlateIcon(), FUIAction( FExecuteAction::CreateUObject( this, &USmartTable::ResetLayout ) ) );
    }
}

TSharedRef< SWidget > USmartTable::MakeTableMenu()
{

    FMenuBuilder Menu = MakeMenuBuilder();

    Menu.BeginSection( NAME_None, LOCTEXT( "MenuColumns", "Columns" ) );

    AppendTableWideEntries( Menu );

    Menu.EndSection();

    OnExtendHeaderMenu.ExecuteIfBound( NAME_None, Menu );

    return HostMenu( Menu );
}

TSharedRef< SWidget > USmartTable::MakeHeaderMenu( FName ColumnId )
{
    const FSmartTableColumn * Column = FindColumn( ColumnId );
    if ( !Column )
    {

        return MakeTableMenu();
    }

    FMenuBuilder Menu = MakeMenuBuilder();

    if ( Column->bSortable && HeaderMenuOptions.bOfferSort )
    {
        Menu.BeginSection( NAME_None, LOCTEXT( "MenuSort", "Sort" ) );

        Menu.AddMenuEntry( LOCTEXT( "SortAscending", "Sort Ascending" ), FText::GetEmpty(), FSlateIcon(), FUIAction( FExecuteAction::CreateUObject( this, &USmartTable::SortFromMenu, ColumnId, ESmartTableSortMode::Ascending, false ) ) );

        Menu.AddMenuEntry( LOCTEXT( "SortDescending", "Sort Descending" ), FText::GetEmpty(), FSlateIcon(), FUIAction( FExecuteAction::CreateUObject( this, &USmartTable::SortFromMenu, ColumnId, ESmartTableSortMode::Descending, false ) ) );

        RefreshDeadSecondaryColumns();

        Menu.AddMenuEntry( LOCTEXT( "SortThenBy", "Then Sort By This" ), LOCTEXT( "SortThenByTip", "Keeps the current sort and orders its ties by this column. Shift-click a header does the same." ), FSlateIcon(),
            FUIAction( FExecuteAction::CreateUObject( this, &USmartTable::SortFromMenu, ColumnId, ESmartTableSortMode::Ascending, true ), FCanExecuteAction::CreateUObject( this, &USmartTable::CanSortBySecondaryColumn, ColumnId ) ) );

        Menu.EndSection();
    }

    const FSmartTableSortSpec Spec = HeaderMenuOptions.bOfferSort ? GetSortSpec() : FSmartTableSortSpec();
    for ( const FSmartTableSortColumn & Sorted : Spec.Columns )
    {
        const FSmartTableColumn * SortedColumn = FindColumn( Sorted.ColumnId );
        if ( SortedColumn && !ColumnView().IsShown( *SortedColumn ) )
        {
            const FText Name = SortedColumn->GetLabel();

            Menu.BeginSection( NAME_None, FText::Format( LOCTEXT( "SortedByHidden", "Sorted by: {0} (hidden)" ), Name ) );
            Menu.AddMenuEntry( LOCTEXT( "ClearHiddenSort", "Clear That Sort" ), FText::GetEmpty(), FSlateIcon(), FUIAction( FExecuteAction::CreateUObject( this, &USmartTable::SortFromMenu, Sorted.ColumnId, ESmartTableSortMode::None, false ) ) );
            Menu.EndSection();
        }
    }

    Menu.BeginSection( NAME_None, LOCTEXT( "MenuColumns", "Columns" ) );

    if ( HeaderMenuOptions.bOfferSizeToContent )
    {
        Menu.AddMenuEntry( LOCTEXT( "SizeToContent", "Size to Content" ), LOCTEXT( "SizeToContentTip", "Fits this column to the widest thing in it." ), FSlateIcon(), FUIAction( FExecuteAction::CreateUObject( this, &USmartTable::SizeColumnToContent, ColumnId ) ) );
    }

    const TArray< FName > Order = GetColumnOrder();
    const int32 Position        = Order.IndexOfByKey( ColumnId );

    if ( HeaderMenuOptions.bOfferMove && Position > 0 )
    {
        Menu.AddMenuEntry( LOCTEXT( "MoveLeft", "Move Left" ), FText::GetEmpty(), FSlateIcon(), FUIAction( FExecuteAction::CreateUObject( this, &USmartTable::MoveColumn, ColumnId, -1 ) ) );
    }

    if ( HeaderMenuOptions.bOfferMove && Position != INDEX_NONE && Position < Order.Num() - 1 )
    {
        Menu.AddMenuEntry( LOCTEXT( "MoveRight", "Move Right" ), FText::GetEmpty(), FSlateIcon(), FUIAction( FExecuteAction::CreateUObject( this, &USmartTable::MoveColumn, ColumnId, 1 ) ) );
    }

    if ( HeaderMenuOptions.bOfferVisibility && Column->bHideable )
    {

        Menu.AddMenuEntry( LOCTEXT( "HideColumn", "Hide This Column" ), FText::GetEmpty(), FSlateIcon(), FUIAction( FExecuteAction::CreateUObject( this, &USmartTable::SetColumnVisible, ColumnId, false ) ) );
    }

    AppendTableWideEntries( Menu );

    Menu.EndSection();

    OnExtendHeaderMenu.ExecuteIfBound( ColumnId, Menu );

    return HostMenu( Menu );
}

TSharedRef< SWidget > USmartTable::MakeColumnVisibilityMenu()
{

    FMenuBuilder Menu = MakeMenuBuilder();

    for ( const FSmartTableColumn & Column : Columns )
    {
        if ( !Column.bHideable )
        {
            continue;
        }

        const FName ColumnId = Column.ColumnId;
        const FText Label    = Column.GetLabel();

        Menu.AddMenuEntry( Label, FText::GetEmpty(), FSlateIcon(), FUIAction( FExecuteAction::CreateUObject( this, &USmartTable::ToggleColumnVisible, ColumnId ), FCanExecuteAction(), FIsActionChecked::CreateUObject( this, &USmartTable::IsColumnVisible, ColumnId ) ), NAME_None, EUserInterfaceActionType::ToggleButton );
    }

    return HostMenu( Menu );
}

void USmartTable::OpenHeaderMenu( FName ColumnId )
{
    ColumnId = ColumnForIntent( ColumnId );

    if ( !HeaderMenuOptions.bEnabled )
    {
        UE_LOGFMT( LogSmartTablesInput, Verbose, "OpenHeaderMenu('{Column}') opened nothing. This table has its header menu turned off. Every entry it would offer is also a function: SetColumnVisible, MoveColumn, ResetLayout.", ColumnId );
        return;
    }

    if ( !HeaderRow.IsValid() || !bShowHeader )
    {
        UE_LOGFMT( LogSmartTablesInput, Verbose, "OpenHeaderMenu('{Column}') opened nothing. There is no header to anchor to. OpenTableMenu is the route with no header.", ColumnId );
        return;
    }

    const int32 ColumnIndex = HeaderRow->FindColumnIndex( ColumnId );
    if ( ColumnIndex == INDEX_NONE )
    {
        if ( FindColumn( ColumnId ) )
        {
            UE_LOGFMT( LogSmartTablesInput, Verbose, "OpenHeaderMenu('{Column}') opened nothing. That column is hidden. Open the menu on a shown column to bring it back.", ColumnId );
        }
        else
        {
            WarnUnknownColumn( TEXT( "OpenHeaderMenu" ), ColumnId );
        }

        return;
    }

    PushMenuAt( HeaderRow->GetColumns()[ ColumnIndex ].HeaderContent.Widget, MakeHeaderMenu( ColumnId ), true );
}

TSharedRef< SWidget > USmartTable::MakeRowMenu( const FSmartTableKeptRow & Row )
{
    FMenuBuilder Menu = MakeMenuBuilder();

    Menu.BeginSection( NAME_None, RowMenuOptions.Heading );

    bool bFirst = true;
    for ( int32 EntryIndex = 0; EntryIndex < RowMenuEntries.Num(); ++EntryIndex )
    {
        const FSmartTableMenuEntry & Entry = RowMenuEntries[ EntryIndex ];
        if ( !Entry.bVisible )
        {
            continue;
        }

        if ( Entry.bSeparatorAbove && !bFirst )
        {
            Menu.AddMenuSeparator();
        }

        bFirst = false;

        const FExecuteAction Pick       = FExecuteAction::CreateUObject( this, &USmartTable::ChooseRowMenuEntry, Entry.Id, EntryIndex, Row );
        const FCanExecuteAction Allowed = FCanExecuteAction::CreateLambda( [ bEnabled = Entry.bEnabled ]()
        {
            return bEnabled;
        } );

        Menu.AddMenuEntry( Entry.Label, Entry.ToolTip, FSlateIcon(), FUIAction( Pick, Allowed ) );
    }

    Menu.EndSection();

    OnExtendRowMenu.ExecuteIfBound( Row, Menu );

    return HostMenu( Menu );
}

void USmartTable::ChooseRowMenuEntry( FName EntryId, int32 EntryIndex, FSmartTableKeptRow Row )
{
    const int32 NaturalNow = Model ? Model->FindKeptRow( Row ) : INDEX_NONE;
    if ( NaturalNow == INDEX_NONE )
    {
        UE_LOGFMT( LogSmartTablesInput, Verbose, "Row menu entry '{Entry}' on '{Table}' did nothing. Row '{Row}' left the table while the menu was open.", EntryId, GetName(), Row.RowId );
        return;
    }

    OnRowMenuEntryChosen.Broadcast( EntryId, EntryIndex, ItemForRow( NaturalNow ), NaturalNow );
}

void USmartTable::SetRowMenuEntries( const TArray< FSmartTableMenuEntry > & Entries )
{
    RowMenuEntries = Entries;

    bWarnedEmptyRowMenu = false;
}

void USmartTable::OpenRowMenu( int32 NaturalRow )
{
    OpenRowMenuInternal( NaturalRow, TOptional< FVector2D >(), INDEX_NONE );
}

void USmartTable::OpenRowMenuFromPointer( int32 NaturalRow, const FVector2D & AbsolutePosition, int32 DrivingUserIndex )
{
    OpenRowMenuInternal( NaturalRow, AbsolutePosition, DrivingUserIndex );
}

void USmartTable::OpenRowMenuInternal( int32 NaturalRow, const TOptional< FVector2D > & AbsolutePosition, int32 DrivingUserIndex )
{

    if ( !ListView.IsValid() || !IsRowDrawn( NaturalRow ) )
    {
        UE_LOGFMT( LogSmartTablesInput, Verbose, "OpenRowMenu({Row}) opened nothing: {Reason}.", NaturalRow, ListView.IsValid() ? TEXT( "that row is not currently drawn - a filter may be hiding it" ) : TEXT( "the widget is not built yet" ) );
        return;
    }

    FSmartTableKeptRow Kept = Model->KeepRow( NaturalRow );

    OnRowMenuOpening.Broadcast( ItemForRow( NaturalRow ), NaturalRow );

    Kept.NaturalRow = Model ? Model->FindKeptRow( Kept ) : INDEX_NONE;
    if ( Kept.NaturalRow == INDEX_NONE )
    {
        UE_LOGFMT( LogSmartTablesInput, Verbose, "OpenRowMenu({Row}) opened nothing. A handler of OnRowMenuOpening took row '{Id}' out of the table.", NaturalRow, Kept.RowId );
        return;
    }

    if ( RowMenuEntries.IsEmpty() && !OnExtendRowMenu.IsBound() )
    {

        if ( !bWarnedEmptyRowMenu )
        {
            bWarnedEmptyRowMenu = true;

            UE_LOGFMT( LogSmartTablesInput, Warning, "'{Table}' opened its row menu with nothing to show. Fill RowMenuEntries in the Details panel, or call SetRowMenuEntries from OnRowMenuOpening.", GetName() );
        }

        return;
    }

    if ( RowMenuOptions.bSelectRowFirst )
    {
        SetRowSelected( Kept.NaturalRow, true );
    }

    const TSharedPtr< ITableRow > Row  = ListView->WidgetFromItem( Kept.NaturalRow );
    const TSharedRef< SWidget > Anchor = Row.IsValid() ? Row->AsWidget() : StaticCastSharedRef< SWidget >( ListView.ToSharedRef() );

    if ( AbsolutePosition.IsSet() )
    {
        PushMenuAtPoint( Anchor, MakeRowMenu( Kept ), AbsolutePosition.GetValue(), DrivingUserIndex );
    }
    else
    {
        PushMenuAt( Anchor, MakeRowMenu( Kept ), true );
    }
}

void USmartTable::OpenTableMenu()
{
    if ( !HeaderMenuOptions.bEnabled )
    {
        UE_LOGFMT( LogSmartTablesInput, Verbose, "OpenTableMenu opened nothing. This table has its header menu turned off." );
        return;
    }

    const bool bUnderHeader            = HeaderRow.IsValid() && bShowHeader;
    const TSharedPtr< SWidget > Anchor = bUnderHeader ? StaticCastSharedPtr< SWidget >( HeaderRow ) : StaticCastSharedPtr< SWidget >( ListView );

    if ( !Anchor.IsValid() )
    {
        UE_LOGFMT( LogSmartTablesInput, Verbose, "OpenTableMenu opened nothing. '{Table}' is not up yet.", GetName() );
        return;
    }

    PushMenuAt( Anchor.ToSharedRef(), MakeTableMenu(), bUnderHeader );
}

void USmartTable::PushMenuAt( TSharedRef< SWidget > Anchor, TSharedRef< SWidget > Menu, bool bBelow )
{
    const FGeometry & Geometry = Anchor->GetTickSpaceGeometry();
    const FVector2D Corner     = bBelow ? FVector2D( 0.0f, Geometry.GetLocalSize().Y ) : FVector2D::ZeroVector;

    PushMenuAtPoint( Anchor, Menu, Geometry.LocalToAbsolute( Corner ), INDEX_NONE );
}

void USmartTable::CloseMenu()
{
    MenuHost.Dismiss();
}

void USmartTable::PushMenuAtPoint( TSharedRef< SWidget > Anchor, TSharedRef< SWidget > Menu, FVector2D AbsolutePosition, int32 DrivingUserIndex )
{
    FSlateApplication & Slate = FSlateApplication::Get();

    FWidgetPath AnchorPath;
    if ( !Slate.GeneratePathToWidgetUnchecked( Anchor, AnchorPath ) )
    {

        UE_LOGFMT( LogSmartTablesInput, Verbose, "A menu on '{Table}' stayed shut. The table is not on screen.", GetName() );
        return;
    }

    PushMenuOnPath( Anchor, AnchorPath, Menu, AbsolutePosition, DrivingUserIndex );
}

void USmartTable::PushMenuOnPath( TSharedRef< SWidget > Anchor, const FWidgetPath & AnchorPath, TSharedRef< SWidget > Menu, FVector2D AbsolutePosition, int32 DrivingUserIndex )
{

    const TSharedPtr< SWindow > Window = AnchorPath.IsValid() ? AnchorPath.GetWindow() : FSlateApplication::Get().FindWidgetWindow( Anchor );

    MenuHost.Push( Anchor, AnchorPath, Menu, InMenuStackSpace( Window, AbsolutePosition ), GetCachedWidget(), DrivingUserIndex );
}

#undef LOCTEXT_NAMESPACE
