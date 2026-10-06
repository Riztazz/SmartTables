// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "View/SmartTableWidgetAnimation.h"

#include "Animation/WidgetAnimation.h"
#include "Blueprint/UserWidget.h"
#include "Slate/WidgetTransform.h"
#include "UObject/UnrealType.h"

namespace SmartTable
{
    UWidgetAnimation * FindWidgetAnimation( const UUserWidget & Widget, FName AnimationName )
    {
        if ( AnimationName.IsNone() )
        {
            return nullptr;
        }

        if ( const FObjectPropertyBase * Property = FindFProperty< FObjectPropertyBase >( Widget.GetClass(), AnimationName ) )
        {
            return Cast< UWidgetAnimation >( Property->GetObjectPropertyValue_InContainer( &Widget ) );
        }

        return nullptr;
    }

    void ResetPooledLook( UUserWidget & Widget )
    {
        Widget.StopAllAnimations();
        Widget.SetRenderOpacity( 1.0f );
        Widget.SetRenderTransform( FWidgetTransform() );
    }
}
