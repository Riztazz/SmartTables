// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "Style/SmartTableStyleRefresh.h"

#if WITH_EDITOR

#include "Algo/Transform.h"
#include "Containers/Set.h"
#include "Logging/StructuredLog.h"
#include "Misc/AssertionMacros.h"
#include "SmartTable.h"
#include "SmartTableLog.h"
#include "SmartTableStyle.h"
#include "Style/SmartTableStyleMirror.h"
#include "UObject/Class.h"
#include "UObject/UObjectIterator.h"
#include "UObject/UnrealType.h"

namespace
{

    int32 ArchetypeDepth( const UObject & Object )
    {
        int32 Depth = 0;
        for ( const UObject * Archetype = Object.GetArchetype(); Archetype; Archetype = Archetype->GetArchetype() )
        {
            ++Depth;
        }

        return Depth;
    }

    struct FStyleFollower
    {
        TWeakObjectPtr< UObject > Object;
        TArray< FProperty * > Properties;
        int32 Depth = 0;
    };

    void MarkFollowers( UObject & Object, const TArray< FProperty * > & Candidates, TArray< FStyleFollower > & Out )
    {
        const UObject * Archetype = Object.GetArchetype();
        if ( !Archetype )
        {
            return;
        }

        FStyleFollower Follower;
        for ( FProperty * Property : Candidates )
        {
            if ( Property->Identical_InContainer( &Object, Archetype ) )
            {
                Follower.Properties.Add( Property );
            }
        }

        if ( Follower.Properties.IsEmpty() )
        {
            return;
        }

        Follower.Object = &Object;
        Follower.Depth  = ArchetypeDepth( Object );

        Out.Add( MoveTemp( Follower ) );
    }
}

void SmartTable::StyleRefresh::FromSettings()
{
    const TArray< SmartTable::StyleMirror::FPair > & Mirrored = SmartTable::StyleMirror::Pairs();

    TArray< FProperty * > StyleSide;
    TArray< FProperty * > TableSide;
    StyleSide.Reserve( Mirrored.Num() );
    TableSide.Reserve( Mirrored.Num() );
    Algo::Transform( Mirrored, StyleSide, &SmartTable::StyleMirror::FPair::OnStyle );
    Algo::Transform( Mirrored, TableSide, &SmartTable::StyleMirror::FPair::OnTable );

    USmartTableStyle * StyleDefaults = GetMutableDefault< USmartTableStyle >();
    USmartTable * TableDefaults      = GetMutableDefault< USmartTable >();

    TArray< FStyleFollower > Followers;

    for ( TObjectIterator< USmartTableStyle > It( RF_NoFlags ); It; ++It )
    {
        if ( *It != StyleDefaults )
        {
            MarkFollowers( **It, StyleSide, Followers );
        }
    }

    for ( TObjectIterator< USmartTable > It( RF_NoFlags ); It; ++It )
    {
        if ( *It != TableDefaults )
        {
            MarkFollowers( **It, TableSide, Followers );
        }
    }

    StyleDefaults->BuildDefaultsFromSettings();
    TableDefaults->CopyStyleClassDefaults( *StyleDefaults );

    Followers.Sort( []( const FStyleFollower & A, const FStyleFollower & B )
    {
        return A.Depth < B.Depth;
    } );

    TSet< const UObject * > Refreshed;
    int32 RefreshedAssets = 0;

    for ( const FStyleFollower & Follower : Followers )
    {
        UObject * Object = Follower.Object.Get();
        if ( !Object )
        {
            continue;
        }

        UObject * Archetype = Object->GetArchetype();

        checkf( Archetype, TEXT( "'%s' lost its archetype between marking and applying" ), *Object->GetName() );

        for ( FProperty * Property : Follower.Properties )
        {
            Property->CopyCompleteValue_InContainer( Object, Archetype );
        }

        Refreshed.Add( Object );
        RefreshedAssets += Object->IsA< USmartTableStyle >() ? 1 : 0;
    }

    UE_LOGFMT( LogSmartTables, Verbose, "Palette changed. Both class defaults went up fresh, then {Assets} style asset(s) and {Tables} table(s) still on theirs.", RefreshedAssets, Refreshed.Num() - RefreshedAssets );

    for ( TObjectIterator< USmartTable > It; It; ++It )
    {
        It->ReapplyAfterPaletteChange( Refreshed );
    }
}

#endif
