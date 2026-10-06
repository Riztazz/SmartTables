// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTableLazyNode.h"

#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "Rendering/DrawElements.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SLeafWidget.h"

namespace
{
    class SSmartTableLazyProbe final : public SLeafWidget
    {
    public:
        SLATE_BEGIN_ARGS( SSmartTableLazyProbe )
        {
        }
        SLATE_EVENT( FSimpleDelegate, OnSeen )
        SLATE_END_ARGS()

        void Construct( const FArguments & InArgs )
        {
            OnSeen = InArgs._OnSeen;
            SetVisibility( EVisibility::HitTestInvisible );
        }

        virtual int32 OnPaint( const FPaintArgs & Args, const FGeometry & AllottedGeometry, const FSlateRect & MyCullingRect, FSlateWindowElementList & OutDrawElements, int32 LayerId, const FWidgetStyle & InWidgetStyle, bool bParentEnabled ) const override
        {
            OnSeen.ExecuteIfBound();

            return LayerId;
        }

        virtual FVector2D ComputeDesiredSize( float LayoutScaleMultiplier ) const override
        {
            return FVector2D( 8.0f, 8.0f );
        }

    private:
        FSimpleDelegate OnSeen;
    };
}

void FSmartTableLazyNode::SetOnRebuildChildren( FSimpleDelegate InOnRegenerateChildren )
{
    OnRebuildChildren = InOnRegenerateChildren;
}

void FSmartTableLazyNode::GenerateChildContent( IDetailChildrenBuilder & ChildrenBuilder )
{
    if ( bRealized )
    {
        GenerateRealChildren( ChildrenBuilder );
        return;
    }

    ChildrenBuilder.AddCustomRow( FText::GetEmpty() ).WholeRowContent()[ SNew( SSmartTableLazyProbe ).OnSeen( FSimpleDelegate::CreateSP( this, &FSmartTableLazyNode::OnPlaceholderSeen ) ) ];
}

void FSmartTableLazyNode::OnPlaceholderSeen()
{
    bRevealRequested = true;
}

bool FSmartTableLazyNode::RequiresTick() const
{
    return true;
}

void FSmartTableLazyNode::Tick( float DeltaTime )
{
    if ( bRevealRequested && !bRealized )
    {
        bRealized = true;

        OnRebuildChildren.ExecuteIfBound();
    }
}
