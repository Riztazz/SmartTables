// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTableRowWidget.h"

#include "Animation/UMGSequencePlayer.h"
#include "Animation/WidgetAnimation.h"
#include "Blueprint/WidgetTree.h"
#include "Components/NamedSlot.h"
#include "Components/NativeWidgetHost.h"
#include "Logging/StructuredLog.h"
#include "SmartTable.h"
#include "SmartTableLog.h"
#include "View/SmartTableWidgetAnimation.h"

const FName USmartTableRowWidget::ColumnsSlotName( TEXT( "Columns" ) );

FName USmartTableRowWidget::ResolveColumnsSlot()
{
    if ( bColumnsSlotResolved )
    {
        return ColumnsSlot;
    }

    bColumnsSlotResolved = true;

    if ( !WidgetTree )
    {
        return ColumnsSlot;
    }

    if ( Cast< UNamedSlot >( WidgetTree->FindWidget( ColumnsSlotName ) ) )
    {
        ColumnsSlot = ColumnsSlotName;

        return ColumnsSlot;
    }

    FName OnlySlot = NAME_None;
    int32 Count    = 0;

    WidgetTree->ForEachWidget( [ &OnlySlot, &Count ]( UWidget * Widget )
    {
        if ( const UNamedSlot * Named = Cast< UNamedSlot >( Widget ) )
        {
            ++Count;
            OnlySlot = Named->GetFName();
        }
    } );

    ColumnsSlot = ( Count == 1 ) ? OnlySlot : NAME_None;

    return ColumnsSlot;
}

bool USmartTableRowWidget::HostColumns( const TSharedRef< SWidget > & Columns )
{
    const FName SlotName = ResolveColumnsSlot();
    if ( SlotName.IsNone() )
    {
        return false;
    }

    if ( !ColumnsHost )
    {

        ColumnsHost = WidgetTree->ConstructWidget< UNativeWidgetHost >();
    }

    ColumnsHost->SetContent( Columns );

    if ( GetContentForSlot( SlotName ) != ColumnsHost )
    {
        SetContentForSlot( SlotName, ColumnsHost );
    }

    return GetContentForSlot( SlotName ) == ColumnsHost;
}

UWidgetAnimation * USmartTableRowWidget::FindRowAnimation( FName AnimationName ) const
{
    return SmartTable::FindWidgetAnimation( *this, AnimationName );
}

UWidgetAnimation * USmartTableRowWidget::ResolveArrival()
{
    if ( ResolvedArrivalFor != ArrivalAnimation )
    {
        ResolvedArrivalFor = ArrivalAnimation;
        ResolvedArrival    = FindRowAnimation( ArrivalAnimation );
    }

    return ResolvedArrival;
}

void USmartTableRowWidget::ArmArrival()
{
    if ( !ResolveArrival() )
    {

        return;
    }

    bArrivalArmed = true;

    SetRenderOpacity( 0.0f );
}

bool USmartTableRowWidget::PlayArrivalNow()
{
    if ( !bArrivalArmed )
    {
        return false;
    }

    bArrivalArmed = false;

    UWidgetAnimation * Arrival = ResolveArrival();
    if ( !Arrival )
    {
        SetRenderOpacity( 1.0f );

        return true;
    }

    PlayAnimation( Arrival, 0.0f, 1, EUMGSequencePlayMode::Forward, 1.0f, true );

    FlushAnimations();

    SetRenderOpacity( 1.0f );

    return true;
}

void USmartTableRowWidget::CancelArrival()
{
    if ( !bArrivalArmed )
    {
        return;
    }

    bArrivalArmed = false;

    SetRenderOpacity( 1.0f );
}

void USmartTableRowWidget::AssignRow( USmartTable * InTable, int32 InNaturalRow, ESmartTableAssignReason InReason )
{
    Table        = InTable;
    NaturalRow   = InNaturalRow;
    AssignReason = InReason;

    NativeOnRowAssigned();
    OnRowAssigned( InReason );
}

void USmartTableRowWidget::ReleaseRow()
{
    SmartTable::ResetPooledLook( *this );

    bArrivalArmed = false;

    NativeOnRowReleased();
    OnRowReleased();

    Table        = nullptr;
    NaturalRow   = INDEX_NONE;
    AssignReason = ESmartTableAssignReason::Scrolled;
}
