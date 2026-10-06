// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTableDispatcher.h"

#include "Async/Async.h"
#include "Async/TaskGraphInterfaces.h"
#include "Tasks/Task.h"

FSmartTableWorkDispatcher SmartTable::Dispatchers::TaskGraph()
{
    return []( TUniqueFunction< void() > Work )
    {
        UE::Tasks::Launch( UE_SOURCE_LOCATION, MoveTemp( Work ) );
    };
}

FSmartTableWorkDispatcher SmartTable::Dispatchers::Synchronous()
{
    return []( TUniqueFunction< void() > Work )
    {
        Work();
    };
}

void SmartTable::Dispatchers::RunOnGameThread( TUniqueFunction< void() > && Work )
{
    if ( IsInGameThread() )
    {
        Work();
        return;
    }

    AsyncTask( ENamedThreads::GameThread, MoveTemp( Work ) );
}
