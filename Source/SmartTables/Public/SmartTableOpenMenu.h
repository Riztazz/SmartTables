// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#pragma once

#include "HAL/Platform.h"
#include "Math/MathFwd.h"
#include "Templates/SharedPointer.h"
#include "UObject/NameTypes.h"

class FSlateStyleSet;
class FWidgetPath;
class IMenu;
class ISlateStyle;
class SWidget;
struct FSmartTableMenuStyle;

struct SMARTTABLES_API FSmartTableOpenMenu
{
    FSmartTableOpenMenu() = default;

    ~FSmartTableOpenMenu();

    FSmartTableOpenMenu( const FSmartTableOpenMenu & )             = delete;
    FSmartTableOpenMenu & operator=( const FSmartTableOpenMenu & ) = delete;

    const ISlateStyle & Style( const FSmartTableMenuStyle & Named, FName SetName );

    void DropStyle();

    void Push( TSharedRef< SWidget > Anchor, const FWidgetPath & AnchorPath, TSharedRef< SWidget > Content, FVector2D ScreenPosition, TWeakPtr< const SWidget > TableRoot, int32 DrivingUserIndex );

    void Dismiss();

    int32 GetDrivingUser() const
    {
        return DrivingUser;
    }

private:
    TWeakPtr< IMenu > Menu;
    TSharedPtr< FSlateStyleSet > StyleSet;

    int32 DrivingUser = INDEX_NONE;

    static void WatchAnchor( TWeakPtr< const SWidget > Root, TWeakPtr< IMenu > Watched );

    static void RestoreFocusNextFrame( int32 UserIndex, TWeakPtr< SWidget > FocusBefore );
};
