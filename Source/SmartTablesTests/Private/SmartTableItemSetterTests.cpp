// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTablesTestsMacros.h"

#if SMARTTABLES_WITH_TESTS

#include "AITestsCommon.h"

#include "Algo/Transform.h"
#include "SmartTable.h"
#include "SmartTableCell.h"
#include "SmartTableDataTableModel.h"
#include "SmartTableObjectModel.h"
#include "SmartTableTestTypes.h"

UE_DISABLE_OPTIMIZATION_SHIP

namespace SmartTable::Test
{
    namespace
    {
        const FName ItemSetterColumn = TEXT( "RowName" );

        FString HandedItemNames( const TArray< const UObject * > & Handed )
        {
            TArray< FString > Names;
            Algo::Transform( Handed, Names, []( const UObject * Item )
            {
                return Item ? Item->GetName() : FString( TEXT( "null" ) );
            } );

            return FString::Join( Names, TEXT( " " ) );
        }
    }

    struct FTheCellHandsEachRowsItemAndNullOnRelease : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TArray< UObject * > Items = { NamedRow( TEXT( "Vanta" ) ), NamedRow( TEXT( "Garris" ) ), NamedRow( TEXT( "Keller" ) ), NamedRow( TEXT( "Osei" ) ) };

            USmartTableObjectModel * Model   = NamedRowModel( Items );
            USmartTable * Table              = NewObject< USmartTable >();
            USmartTableTestHandedCell * Cell = NewObject< USmartTableTestHandedCell >();
            const FString Name0              = Items[ 0 ]->GetName();
            const FString Name1              = Items[ 1 ]->GetName();
            const FString Name3              = Items[ 3 ]->GetName();

            Cell->AssignCell( Table, Model, 0, ItemSetterColumn, ESmartTableAssignReason::Scrolled );
            AITEST_EQUAL( "The first row hands its item over", HandedItemNames( Cell->Handed ), Name0 );

            Cell->AssignCell( Table, Model, 1, ItemSetterColumn, ESmartTableAssignReason::Refreshed );
            AITEST_EQUAL( "A sort that moves the cell to another row hands that row's item", HandedItemNames( Cell->Handed ), Name0 + TEXT( " " ) + Name1 );

            Cell->ReleaseCell();
            AITEST_EQUAL( "The release hands null", HandedItemNames( Cell->Handed ), Name0 + TEXT( " " ) + Name1 + TEXT( " null" ) );

            Cell->AssignCell( Table, Model, 3, ItemSetterColumn, ESmartTableAssignReason::Scrolled );
            AITEST_EQUAL( "The same cell, reused by a scroll, hands the next row's item", HandedItemNames( Cell->Handed ), Name0 + TEXT( " " ) + Name1 + TEXT( " null " ) + Name3 );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FTheCellHandsEachRowsItemAndNullOnRelease, "SmartTables.Cell.ItemSetter.TheCellHandsEachRowsItemAndNullOnRelease" );

    struct FTheSameItemIsHandedOnce : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTableObjectModel * Model   = NamedRowModel( { NamedRow( TEXT( "Vanta" ) ) } );
            USmartTable * Table              = NewObject< USmartTable >();
            USmartTableTestHandedCell * Cell = NewObject< USmartTableTestHandedCell >();

            Cell->AssignCell( Table, Model, 0, ItemSetterColumn, ESmartTableAssignReason::Added );
            Cell->AssignCell( Table, Model, 0, ItemSetterColumn, ESmartTableAssignReason::ValueChanged );
            Cell->AssignCell( Table, Model, 0, ItemSetterColumn, ESmartTableAssignReason::Refreshed );

            AITEST_EQUAL( "A value change and a refresh on the row already drawn hand nothing more", Cell->Handed.Num(), 1 );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FTheSameItemIsHandedOnce, "SmartTables.Cell.ItemSetter.TheSameItemIsHandedOnce" );

    struct FAReleasedCellHandsTheSameRowAgain : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTableObjectModel * Model   = NamedRowModel( { NamedRow( TEXT( "Vanta" ) ) } );
            USmartTable * Table              = NewObject< USmartTable >();
            USmartTableTestHandedCell * Cell = NewObject< USmartTableTestHandedCell >();

            Cell->AssignCell( Table, Model, 0, ItemSetterColumn, ESmartTableAssignReason::Scrolled );
            Cell->ReleaseCell();
            Cell->AssignCell( Table, Model, 0, ItemSetterColumn, ESmartTableAssignReason::Scrolled );

            AITEST_EQUAL( "The item, null, then the item again", Cell->Handed.Num(), 3 );
            AITEST_NULL( "...with null in the middle", Cell->Handed[ 1 ] );
            AITEST_TRUE( "...and the same item at both ends", Cell->Handed[ 0 ] && Cell->Handed[ 0 ] == Cell->Handed[ 2 ] );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAReleasedCellHandsTheSameRowAgain, "SmartTables.Cell.ItemSetter.AReleasedCellHandsTheSameRowAgain" );

    struct FAModelWithNoItemsHandsNothing : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTableTestCountingModel * Model = NewObject< USmartTableTestCountingModel >();
            USmartTable * Table                  = NewObject< USmartTable >();
            USmartTableTestHandedCell * Cell     = NewObject< USmartTableTestHandedCell >();

            for ( int32 Row = 0; Row < 4; ++Row )
            {
                Cell->AssignCell( Table, Model, Row, ItemSetterColumn, ESmartTableAssignReason::Scrolled );
                Cell->ReleaseCell();
            }

            AITEST_EQUAL( "A cell with an Item Setter asks for the item once per row", Model->ItemsAsked, 4 );
            AITEST_EQUAL( "...and a model with none hands nothing, not even a release", Cell->Handed.Num(), 0 );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAModelWithNoItemsHandsNothing, "SmartTables.Cell.ItemSetter.AModelWithNoItemsHandsNothing" );

    struct FAnItemOfTheWrongClassIsHandedAsNull : FAITestBase
    {
        virtual bool InstantTest() override
        {
            UObject * First  = NewObject< USmartTableTestLayoutStore >();
            UObject * Second = NewObject< USmartTableTestLayoutStore >();

            USmartTableObjectModel * Model  = NamedRowModel( { First, NamedRow( TEXT( "Garris" ) ), Second, NamedRow( TEXT( "Keller" ) ) } );
            USmartTable * Table             = NewObject< USmartTable >();
            USmartTableTestTypedCell * Cell = NewObject< USmartTableTestTypedCell >();

            GetTestRunner().AddExpectedMessagePlain( TEXT( "which takes a SmartTableTestLayoutStore" ), ELogVerbosity::Warning, EAutomationExpectedMessageFlags::Contains, 1 );

            Cell->AssignCell( Table, Model, 0, ItemSetterColumn, ESmartTableAssignReason::Scrolled );
            Cell->AssignCell( Table, Model, 1, ItemSetterColumn, ESmartTableAssignReason::Refreshed );
            Cell->AssignCell( Table, Model, 2, ItemSetterColumn, ESmartTableAssignReason::Refreshed );
            Cell->AssignCell( Table, Model, 3, ItemSetterColumn, ESmartTableAssignReason::Refreshed );

            AITEST_EQUAL( "Each wrong item takes the right one back off the cell", Cell->Handed.Num(), 4 );
            AITEST_EQUAL( "...so the first store", Cell->Handed[ 0 ], static_cast< const UObject * >( First ) );
            AITEST_NULL( "...then null for a named row", Cell->Handed[ 1 ] );
            AITEST_EQUAL( "...then the second store", Cell->Handed[ 2 ], static_cast< const UObject * >( Second ) );
            AITEST_NULL( "...then null again, with one warning for both", Cell->Handed[ 3 ] );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAnItemOfTheWrongClassIsHandedAsNull, "SmartTables.Cell.ItemSetter.AnItemOfTheWrongClassIsHandedAsNull" );

    struct FAMisnamedSetterWarnsOnceAndTheCellStillDraws : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTableObjectModel * Model = NamedRowModel( { NamedRow( TEXT( "Vanta" ) ), NamedRow( TEXT( "Garris" ) ) } );
            USmartTable * Table            = NewObject< USmartTable >();

            USmartTableTestHandedCell * First  = NewObject< USmartTableTestHandedCell >();
            USmartTableTestHandedCell * Second = NewObject< USmartTableTestHandedCell >();
            First->NameItemSetter( TEXT( "SetNoSuchContext" ) );
            Second->NameItemSetter( TEXT( "SetNoSuchContext" ) );

            GetTestRunner().AddExpectedMessagePlain( TEXT( "which is no function of that class" ), ELogVerbosity::Warning, EAutomationExpectedMessageFlags::Contains, 1 );

            First->AssignCell( Table, Model, 0, ItemSetterColumn, ESmartTableAssignReason::Scrolled );
            Second->AssignCell( Table, Model, 1, ItemSetterColumn, ESmartTableAssignReason::Scrolled );

            AITEST_EQUAL( "Two cells of the class, and nothing handed to either", First->Handed.Num() + Second->Handed.Num(), 0 );
            AITEST_EQUAL( "The cell still holds its table", First->GetTable(), Table );
            AITEST_EQUAL( "...and its row", Second->GetRowIndex(), 1 );

            First->ReleaseCell();
            AITEST_EQUAL( "A release hands nothing either", First->Handed.Num(), 0 );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAMisnamedSetterWarnsOnceAndTheCellStillDraws, "SmartTables.Cell.ItemSetter.AMisnamedSetterWarnsOnceAndTheCellStillDraws" );

    struct FASetterWithTwoInputsIsTurnedDown : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTableObjectModel * Model   = NamedRowModel( { NamedRow( TEXT( "Vanta" ) ) } );
            USmartTable * Table              = NewObject< USmartTable >();
            USmartTableTestHandedCell * Cell = NewObject< USmartTableTestHandedCell >();
            Cell->NameItemSetter( GET_FUNCTION_NAME_CHECKED( USmartTableTestHandedCell, TakeTwo ) );

            GetTestRunner().AddExpectedMessagePlain( TEXT( "which takes 2 inputs and not one" ), ELogVerbosity::Warning, EAutomationExpectedMessageFlags::Contains, 1 );

            Cell->AssignCell( Table, Model, 0, ItemSetterColumn, ESmartTableAssignReason::Scrolled );

            AITEST_EQUAL( "The function with two inputs never runs", Cell->Handed.Num(), 0 );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FASetterWithTwoInputsIsTurnedDown, "SmartTables.Cell.ItemSetter.ASetterWithTwoInputsIsTurnedDown" );

    struct FTheSetterListHoldsOnlyWhatCanTakeAnItem : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TArray< FName > Options = GetDefault< USmartTableTestHandedCell >()->ItemSetterOptions();

            AITEST_EQUAL( "None and the one function that takes an object, and nothing from UUserWidget or the cell base", Options.Num(), 2 );
            AITEST_TRUE( "...None first, so the list can clear the field", Options[ 0 ].IsNone() );
            AITEST_TRUE( "...then TakeItem", Options[ 1 ] == GET_FUNCTION_NAME_CHECKED( USmartTableTestHandedCell, TakeItem ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FTheSetterListHoldsOnlyWhatCanTakeAnItem, "SmartTables.Cell.ItemSetter.TheSetterListHoldsOnlyWhatCanTakeAnItem" );
}

UE_ENABLE_OPTIMIZATION_SHIP

#endif
