// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#pragma once

#include "Containers/Array.h"

class FProperty;
class USmartTable;
class USmartTableStyle;

namespace SmartTable::StyleMirror
{

    struct FPair
    {
        FProperty * OnStyle = nullptr;
        FProperty * OnTable = nullptr;
    };

    const TArray< FPair > & Pairs();

    void StyleToTable( const USmartTableStyle & From, USmartTable & To );

    void TableToStyle( const USmartTable & From, USmartTableStyle & To );
}
