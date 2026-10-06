// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#pragma once

#include "HAL/Platform.h"

namespace SmartTable::Height
{

    SMARTTABLES_API float FitRows( int32 NumPresented, int32 MaxVisibleRows, float RowHeight, float HeaderHeight );

    SMARTTABLES_API int32 RowsWindowFits( float WindowHeight, float RowHeight );

    SMARTTABLES_API bool LooksSizedByContent( float Allotted, int32 Generated, float WindowHeight, float RowHeight );
}
