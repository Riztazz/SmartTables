// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "Table/SmartTableHeight.h"

#include "Math/UnrealMathUtility.h"

float SmartTable::Height::FitRows( int32 NumPresented, int32 MaxVisibleRows, float RowHeight, float HeaderHeight )
{
    const int32 VisibleRows = MaxVisibleRows > 0 ? FMath::Min( NumPresented, MaxVisibleRows ) : NumPresented;

    return HeaderHeight + VisibleRows * RowHeight;
}

int32 SmartTable::Height::RowsWindowFits( float WindowHeight, float RowHeight )
{
    return RowHeight > 0.0f ? FMath::CeilToInt( WindowHeight / RowHeight ) : 0;
}

bool SmartTable::Height::LooksSizedByContent( float Allotted, int32 Generated, float WindowHeight, float RowHeight )
{
    return RowHeight > 0.0f && WindowHeight > 0.0f && Allotted > WindowHeight && Generated > RowsWindowFits( WindowHeight, RowHeight ) * 2;
}
