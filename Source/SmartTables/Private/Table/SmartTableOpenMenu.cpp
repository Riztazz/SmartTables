// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTableOpenMenu.h"

#include "Containers/Ticker.h"
#include "Framework/Application/IMenu.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Application/SlateUser.h"
#include "Layout/WidgetPath.h"
#include "Logging/StructuredLog.h"
#include "SmartTableLog.h"
#include "Style/SmartTableMenuStyleSet.h"
#include "Styling/SlateStyle.h"
#include "Widgets/SWidget.h"

namespace
{
    int32 UserPointingAtTable( FSlateApplication & Slate, const TWeakPtr< const SWidget > & TableRoot )
    {
        const TSharedPtr< const SWidget > Root = TableRoot.Pin();

        int32 Pointing = INDEX_NONE;
        if ( Root.IsValid() )
        {

            Slate.ForEachUser( [ &Pointing, &Root ]( FSlateUser & User )
            {
                if ( Pointing == INDEX_NONE && User.IsWidgetUnderAnyPointer( Root ) )
                {
                    Pointing = User.GetUserIndex();
                }
            }, true );
        }

        return Pointing != INDEX_NONE ? Pointing : Slate.GetUserIndexForKeyboard();
    }
}

FSmartTableOpenMenu::~FSmartTableOpenMenu()
{
    if ( FSlateApplication::IsInitialized() )
    {
        Dismiss();
    }
}

const ISlateStyle & FSmartTableOpenMenu::Style( const FSmartTableMenuStyle & Named, FName SetName )
{
    if ( !StyleSet.IsValid() )
    {
        StyleSet = MakeSmartTableMenuStyleSet( SetName, Named );
    }

    return *StyleSet;
}

void FSmartTableOpenMenu::DropStyle()
{
    Dismiss();

    StyleSet.Reset();
}

void FSmartTableOpenMenu::Dismiss()
{
    if ( const TSharedPtr< IMenu > Live = Menu.Pin() )
    {
        Live->Dismiss();
    }

    Menu.Reset();
}

void FSmartTableOpenMenu::Push( TSharedRef< SWidget > Anchor, const FWidgetPath & AnchorPath, TSharedRef< SWidget > Content, FVector2D ScreenPosition, TWeakPtr< const SWidget > TableRoot, int32 DrivingUserIndex )
{
    FSlateApplication & Slate = FSlateApplication::Get();

    Slate.DismissAllMenus();

    const int32 UserIndex                 = DrivingUserIndex != INDEX_NONE ? DrivingUserIndex : UserPointingAtTable( Slate, TableRoot );
    const TWeakPtr< SWidget > FocusBefore = Slate.GetUserFocusedWidget( static_cast< uint32 >( UserIndex ) );

    DrivingUser = UserIndex;

    const TSharedPtr< IMenu > Opened = Slate.PushMenu( Anchor, AnchorPath, Content, ScreenPosition, FPopupTransitionEffect( FPopupTransitionEffect::ContextMenu ), true, FVector2f::ZeroVector, TOptional< EPopupMethod >(), true, UserIndex );

    if ( !Opened.IsValid() )
    {
        return;
    }

    Menu = Opened;

    WatchAnchor( TableRoot, Opened );

    Opened->GetOnMenuDismissed().AddLambda( [ UserIndex, FocusBefore ]( TSharedRef< IMenu > )
    {
        UE_LOGFMT( LogSmartTablesInput, Verbose, "A table menu shut. Focus comes back on the next frame." );

        RestoreFocusNextFrame( UserIndex, FocusBefore );
    } );
}

void FSmartTableOpenMenu::WatchAnchor( TWeakPtr< const SWidget > Root, TWeakPtr< IMenu > Watched )
{
    const TSharedRef< int32 > MissedFrames    = MakeShared< int32 >( 0 );
    const TSharedRef< FDelegateHandle > Watch = MakeShared< FDelegateHandle >();

    *Watch = FSlateApplication::Get().OnPreTick().AddLambda( [ Root, Watched, MissedFrames, Watch ]( float )
    {
        if ( !FSlateApplication::IsInitialized() )
        {
            return;
        }

        FSlateApplication & Ticking = FSlateApplication::Get();

        if ( !Watched.IsValid() )
        {
            Ticking.OnPreTick().Remove( *Watch );
            return;
        }

        FWidgetPath Unused;

        const TSharedPtr< const SWidget > Drawn = Root.Pin();
        if ( Drawn.IsValid() && Ticking.GeneratePathToWidgetUnchecked( Drawn.ToSharedRef(), Unused ) )
        {
            *MissedFrames = 0;
            return;
        }

        if ( ++( *MissedFrames ) < 2 )
        {
            return;
        }

        UE_LOGFMT( LogSmartTablesInput, Verbose, "A table menu shut itself. The table it belongs to is not drawn any more." );

        if ( const TSharedPtr< IMenu > Live = Watched.Pin() )
        {
            Live->Dismiss();
        }
    } );
}

void FSmartTableOpenMenu::RestoreFocusNextFrame( int32 UserIndex, TWeakPtr< SWidget > FocusBefore )
{

    FTSTicker::GetCoreTicker().AddTicker( FTickerDelegate::CreateLambda(
                                              [ UserIndex, FocusBefore ]( float )
    {

        if ( !FSlateApplication::IsInitialized() )
        {
            return false;
        }

        FSlateApplication & Restoring = FSlateApplication::Get();

        const TSharedPtr< SWidget > Restore = FocusBefore.Pin();
        if ( !Restore.IsValid() || !Restore->SupportsKeyboardFocus() )
        {
            UE_LOGFMT( LogSmartTablesInput, Verbose, "Focus stays where it is after a menu: {Reason}.", Restore.IsValid() ? TEXT( "the widget that had it cannot take keyboard focus" ) : TEXT( "the widget that had it is gone" ) );
            return false;
        }

        const TSharedPtr< SWidget > Holder = Restoring.GetUserFocusedWidget( static_cast< uint32 >( UserIndex ) );
        if ( Holder.IsValid() )
        {
            UE_LOGFMT( LogSmartTablesInput, Verbose, "Focus stays where it is after a menu. '{Widget}' took it later than this did.", Holder->GetType() );
            return false;
        }

        const bool bRestored = Restoring.SetUserFocus( static_cast< uint32 >( UserIndex ), Restore, EFocusCause::SetDirectly );

        UE_LOGFMT( LogSmartTablesInput, Verbose, "Focus once a menu shut: '{Widget}' ({Result}).", Restore->GetType(), bRestored ? TEXT( "restored" ) : TEXT( "refused by Slate" ) );

        return false;
    } ),
        0.0f );
}
