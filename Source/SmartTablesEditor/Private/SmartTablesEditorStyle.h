// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#pragma once

#include "Templates/SharedPointer.h"

class FSlateStyleSet;

class FSmartTablesEditorStyle
{
public:
    static void Register();

    static void Unregister();

private:
    static TSharedPtr< FSlateStyleSet > StyleSet;
};
