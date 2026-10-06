// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#pragma once

#include "IDetailCustomNodeBuilder.h"
#include "Templates/SharedPointer.h"

class IDetailChildrenBuilder;

class FSmartTableLazyNode : public IDetailCustomNodeBuilder, public TSharedFromThis< FSmartTableLazyNode >
{
public:
    virtual void SetOnRebuildChildren( FSimpleDelegate InOnRegenerateChildren ) override;
    virtual void GenerateChildContent( IDetailChildrenBuilder & ChildrenBuilder ) override;
    virtual void Tick( float DeltaTime ) override;
    virtual bool RequiresTick() const override;

protected:
    virtual void GenerateRealChildren( IDetailChildrenBuilder & ChildrenBuilder ) = 0;

private:
    void OnPlaceholderSeen();

    FSimpleDelegate OnRebuildChildren;

    bool bRevealRequested = false;

    bool bRealized = false;
};
