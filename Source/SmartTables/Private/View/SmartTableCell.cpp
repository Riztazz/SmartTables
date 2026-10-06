// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTableCell.h"

#include "Animation/UMGSequencePlayer.h"
#include "Animation/WidgetAnimation.h"
#include "Components/SizeBox.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Application/SlateUser.h"
#include "Logging/StructuredLog.h"
#include "SmartTable.h"
#include "SmartTableCellDragDropOp.h"
#include "SmartTableLog.h"
#include "SmartTableModel.h"
#include "SmartTableSortKey.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "View/SmartTableItemSetter.h"
#include "View/SmartTableWidgetAnimation.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Text/STextBlock.h"

UWidgetAnimation * USmartTableCell::FindCellAnimation( FName AnimationName ) const
{
    return SmartTable::FindWidgetAnimation( *this, AnimationName );
}

UWidgetAnimation * USmartTableCell::ResolveFlash()
{
    if ( ResolvedFlashFor != ValueChangedAnimation )
    {
        ResolvedFlashFor = ValueChangedAnimation;
        ResolvedFlash    = FindCellAnimation( ValueChangedAnimation );
    }

    return ResolvedFlash;
}

void USmartTableCell::PlayValueChanged( ESmartTableAssignReason Reason )
{
    if ( Reason != ESmartTableAssignReason::ValueChanged )
    {
        return;
    }

    UWidgetAnimation * Flash = ResolveFlash();
    if ( !Flash )
    {

        UE_LOGFMT( LogSmartTables, VeryVerbose, "Cell '{Column}' holds no animation called '{Animation}'. A value change plays nothing.", ColumnId, ValueChangedAnimation );

        return;
    }

    PlayAnimation( Flash, 0.0f, 1, EUMGSequencePlayMode::Forward, 1.0f, true );
}

void USmartTableCell::AssignCell( USmartTable * InTable, USmartTableModel * InModel, int32 InNaturalRow, FName InColumnId, ESmartTableAssignReason InReason )
{
    Table        = InTable;
    Model        = InModel;
    NaturalRow   = InNaturalRow;
    ColumnId     = InColumnId;
    AssignReason = InReason;

    PlayValueChanged( InReason );

    if ( !bWantsItem.IsSet() )
    {
        bWantsItem = GetClass()->IsFunctionImplementedInScript( GET_FUNCTION_NAME_CHECKED( USmartTableCell, OnItemSet ) );
    }

    UObject * Item = ( bWantsItem.GetValue() || HasItemSetter() ) ? GetItem() : nullptr;

    HandOverItem( Item );

    NativeOnCellAssigned();
    OnCellAssigned( InReason );

    if ( bWantsItem.GetValue() )
    {
        OnItemSet( Item );
    }
}

void USmartTableCell::HandOverItem( UObject * Item )
{
    if ( Item == HandedItem || !HasItemSetter() )
    {
        return;
    }

    UFunction * Setter = FindItemSetter();
    if ( !Setter )
    {
        return;
    }

    const UClass & Taken = SmartTable::ItemSetterInputClass( *Setter );

    if ( Item && !Item->IsA( &Taken ) )
    {
        if ( IsFirstItemSetterWarning() )
        {
            UE_LOGFMT( LogSmartTablesData, Warning, "{Class} hands its row's object to Item Setter '{Setter}', which takes a {Wanted}. Column '{Column}' holds a {Held} there, so those rows are handed nothing. Give the column a model whose Get Row Item answers the class the setter takes, or pick a setter that takes the class the model holds.", GetClass()->GetName(), ItemSetter, Taken.GetName(),
                ColumnId, Item->GetClass()->GetName() );
        }

        Item = nullptr;

        if ( !HandedItem )
        {
            return;
        }
    }

    HandedItem = Item;

    SmartTable::CallItemSetter( *this, *Setter, Item );
}

UFunction * USmartTableCell::FindItemSetter()
{
    if ( !ResolvedItemSetter.IsSet() )
    {
        const TValueOrError< UFunction *, FString > Found = SmartTable::ResolveItemSetter( *GetClass(), ItemSetter );

        if ( Found.HasError() && IsFirstItemSetterWarning() )
        {
            UE_LOGFMT( LogSmartTablesData, Warning, "Cell class {Class} names Item Setter '{Setter}', which {Reason}. Its rows are handed nothing. An Item Setter is a function of the cell that takes one object and nothing else. Pick one from the Item Setter list in the cell's Class Defaults. A Manual viewmodel context adds its Set function to that list once the Widget Blueprint compiles.",
                GetClass()->GetName(), ItemSetter, Found.GetError() );
        }

        ResolvedItemSetter.Emplace( Found.HasValue() ? Found.GetValue() : nullptr );
    }

    return ResolvedItemSetter->Get();
}

bool USmartTableCell::IsFirstItemSetterWarning()
{
    USmartTable * OwningTable = Table.Get();

    return !OwningTable || OwningTable->IsFirstItemSetterWarningFor( *GetClass() );
}

TArray< FName > USmartTableCell::GetItemSetterOptions() const
{
    TArray< FName > Options = { NAME_None };

    for ( TFieldIterator< UFunction > It( GetClass() ); It; ++It )
    {
        const UClass * Owner = It->GetOwnerClass();
        if ( Owner != USmartTableCell::StaticClass() && Owner->IsChildOf< USmartTableCell >() && SmartTable::CanBeItemSetter( **It ) )
        {
            Options.Add( It->GetFName() );
        }
    }

    return Options;
}

void USmartTableCell::ReleaseCell()
{
    SmartTable::ResetPooledLook( *this );

    NativeOnCellReleased();
    OnCellReleased();

    if ( HandedItem )
    {
        HandedItem = nullptr;

        if ( UFunction * Setter = FindItemSetter() )
        {
            SmartTable::CallItemSetter( *this, *Setter, nullptr );
        }
    }

    Table        = nullptr;
    Model        = nullptr;
    NaturalRow   = INDEX_NONE;
    ColumnId     = NAME_None;
    DragRole     = ESmartTableCellDragRole::None;
    AssignReason = ESmartTableAssignReason::Scrolled;
}

void USmartTableCell::SetCellDragRole( ESmartTableCellDragRole NewRole )
{
    if ( DragRole == NewRole )
    {

        return;
    }

    DragRole = NewRole;

    OnCellDragRoleChanged( NewRole );
}

void USmartTableCell::OnCellDragRoleChanged_Implementation( ESmartTableCellDragRole NewRole )
{

}

UObject * USmartTableCell::GetItem() const
{
    return Model ? Model->GetRowItem( NaturalRow ) : nullptr;
}

USmartTableCellDragDropOp * USmartTableCell::MakeCellDragPayload_Implementation()
{
    USmartTableCellDragDropOp * Operation = NewObject< USmartTableCellDragDropOp >();

    checkf( Operation, TEXT( "A cell drag could not make its own payload object" ) );

    Operation->SourceRow      = GetRowIndex();
    Operation->SourceRowId    = Model ? Model->GetRowId( GetRowIndex() ) : NAME_None;
    Operation->SourceColumnId = GetColumnId();
    Operation->SourceItem     = GetItem();
    Operation->SourceTable    = GetTable();

    Operation->DefaultDragVisual = MakeCellDragVisual();

    Operation->Pivot = EDragPivot::MouseDown;

    return Operation;
}

UWidget * USmartTableCell::MakeCellDragVisual_Implementation()
{

    const FVector2D Painted = GetCachedGeometry().GetLocalSize();
    if ( Painted.X <= 0.0 || Painted.Y <= 0.0 )
    {

        UE_LOGFMT( LogSmartTablesInput, VeryVerbose, "Cell for '{Column}' painted at no size. Its drag carries no visual.", GetColumnId() );

        return nullptr;
    }

    TSubclassOf< USmartTableCell > VisualClass = GetClass();
    if ( const USmartTable * OwningTable = GetTable() )
    {
        if ( const FSmartTableColumn * Column = OwningTable->FindColumn( GetColumnId() ) )
        {
            if ( Column->DragVisualClass )
            {
                VisualClass = Column->DragVisualClass;
            }
        }
    }

    USmartTableCell * Ghost = CreateWidget< USmartTableCell >( this, VisualClass );
    if ( !Ghost )
    {

        UE_LOGFMT( LogSmartTablesInput, Warning, "No drag visual of class {Class} could be built for column '{Column}', and the drag carries none. Point DragVisualClass on the column at a real Smart Table Cell, or override MakeCellDragVisual.", GetNameSafe( VisualClass ), GetColumnId() );

        return nullptr;
    }

    USizeBox * Sized = NewObject< USizeBox >( this );
    checkf( Sized, TEXT( "A cell drag visual could not make its own size box" ) );

    Sized->SetWidthOverride( static_cast< float >( Painted.X ) );
    Sized->SetHeightOverride( static_cast< float >( Painted.Y ) );
    Sized->AddChild( Ghost );

    Sized->TakeWidget();

    Ghost->AssignCell( GetTable(), GetModel(), GetRowIndex(), GetColumnId(), ESmartTableAssignReason::Scrolled );

    Ghost->SetRenderOpacity( 0.85f );

    UE_LOGFMT( LogSmartTablesInput, VeryVerbose, "Drag visual for row {Row} column '{Column}': a {Class} at {Width}x{Height} px.", GetRowIndex(), GetColumnId(), GetNameSafe( VisualClass ), FMath::RoundToInt( Painted.X ), FMath::RoundToInt( Painted.Y ) );

    return Sized;
}

FReply USmartTableCell::NativeOnMouseButtonDown( const FGeometry & Geometry, const FPointerEvent & MouseEvent )
{
    const USmartTable * OwningTable = GetTable();
    if ( !OwningTable || !OwningTable->AllowsCellDragDrop() || MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton )
    {
        return Super::NativeOnMouseButtonDown( Geometry, MouseEvent );
    }

    return FReply::Handled().DetectDrag( TakeWidget(), EKeys::LeftMouseButton );
}

void USmartTableCell::NativeOnDragDetected( const FGeometry & Geometry, const FPointerEvent & MouseEvent, UDragDropOperation *& OutOperation )
{
    if ( !USmartTable::CanPointerStartDrag( MouseEvent ) )
    {
        UE_LOGFMT( LogSmartTablesInput, Verbose, "The cell on row {Row}, column '{Column}', will not drag: the pointer holds no cursor.", GetRowIndex(), GetColumnId() );
        return;
    }

    const USmartTable * OwningTable = GetTable();
    if ( !OwningTable || !OwningTable->AllowsCellDragDrop() )
    {
        return;
    }

    OutOperation = MakeCellDragPayload();

    UE_LOGFMT( LogSmartTablesInput, VeryVerbose, "A cell drag starts on row {Row}, column '{Column}'.", GetRowIndex(), GetColumnId() );

    if ( USmartTable * Marking = GetTable() )
    {
        Marking->BeginCellDrag( GetRowIndex(), GetColumnId() );
    }
}

bool USmartTableCell::NativeOnDragOver( const FGeometry & Geometry, const FDragDropEvent & DragDropEvent, UDragDropOperation * InOperation )
{

    USmartTable * OwningTable = GetTable();
    if ( OwningTable && OwningTable->AllowsCellDragDrop() && Cast< USmartTableCellDragDropOp >( InOperation ) )
    {
        OwningTable->SetCellDragTarget( GetRowIndex(), GetColumnId() );
    }

    return Super::NativeOnDragOver( Geometry, DragDropEvent, InOperation );
}

void USmartTableCell::NativeOnDragLeave( const FDragDropEvent & DragDropEvent, UDragDropOperation * InOperation )
{
    if ( USmartTable * OwningTable = GetTable() )
    {
        OwningTable->ClearCellDragTarget( GetRowIndex(), GetColumnId() );
    }

    Super::NativeOnDragLeave( DragDropEvent, InOperation );
}

bool USmartTableCell::NativeOnDrop( const FGeometry & Geometry, const FDragDropEvent & DragDropEvent, UDragDropOperation * InOperation )
{
    USmartTable * OwningTable           = GetTable();
    USmartTableCellDragDropOp * Dropped = Cast< USmartTableCellDragDropOp >( InOperation );

    if ( !OwningTable || !OwningTable->AllowsCellDragDrop() || !Dropped )
    {
        return Super::NativeOnDrop( Geometry, DragDropEvent, InOperation );
    }

    OwningTable->NotifyCellDropped( Dropped, GetRowIndex(), GetColumnId() );

    return true;
}

TSharedRef< SWidget > USmartTableTextCell::RebuildWidget()
{

    TextBlock = SNew( STextBlock );

    return TextBlock.ToSharedRef();
}

void USmartTableTextCell::NativeOnCellAssigned()
{
    Super::NativeOnCellAssigned();

    if ( !TextBlock.IsValid() )
    {

        UE_LOGFMT( LogSmartTablesData, VeryVerbose, "Text cell for '{Column}' row {Row} holds no text block.", GetColumnId(), GetRowIndex() );
        return;
    }

    FSlateColor StyleColour = FSlateColor::UseForeground();
    if ( const USmartTable * OwningTable = GetTable() )
    {
        TextBlock->SetTextStyle( &OwningTable->GetCellTextStyle(),  true );
        StyleColour = OwningTable->GetCellTextStyle().ColorAndOpacity;
    }

    USmartTableModel * CellModel = GetModel();
    if ( !CellModel )
    {
        UE_LOGFMT( LogSmartTablesData, VeryVerbose, "Text cell for '{Column}' draws nothing. No model sits under it to read.", GetColumnId() );
        TextBlock->SetText( FText::GetEmpty() );
        return;
    }

    TextBlock->SetText( CellModel->GetCellText( GetRowIndex(), GetColumnId() ) );

    const FLinearColor Colour = CellModel->GetCellColor( GetRowIndex(), GetColumnId() );
    TextBlock->SetColorAndOpacity( Colour.A > 0.0f ? FSlateColor( Colour ) : StyleColour );
}

namespace
{

    constexpr int32 ReadOnlySlot = 0;
    constexpr int32 TextSlot     = 1;
    constexpr int32 ToggleSlot   = 2;
}

TSharedRef< SWidget > USmartTableEditableCell::RebuildWidget()
{

    // clang-format off
    Switcher = SNew( SWidgetSwitcher )

        + SWidgetSwitcher::Slot()
            .HAlign( HAlign_Fill )
            .VAlign( VAlign_Fill )
            [
                SAssignNew( ReadOnlyText, STextBlock )
            ]

        + SWidgetSwitcher::Slot()
            .HAlign( HAlign_Fill )
            .VAlign( VAlign_Fill )
            [
                SAssignNew( TextBox, SEditableTextBox )
                    .OnTextCommitted( FOnTextCommitted::CreateUObject( this, &USmartTableEditableCell::CommitText ) )
            ]

        + SWidgetSwitcher::Slot()
            .HAlign( HAlign_Fill )
            .VAlign( VAlign_Fill )
            [

                SAssignNew( CheckBox, SCheckBox )
                    .HAlign( HAlign_Center )
                    .OnCheckStateChanged( FOnCheckStateChanged::CreateUObject( this, &USmartTableEditableCell::CommitToggle ) )
            ];
    // clang-format on

    return Switcher.ToSharedRef();
}

void USmartTableEditableCell::NativeOnCellAssigned()
{
    Super::NativeOnCellAssigned();

    USmartTableModel * CellModel = GetModel();
    if ( !Switcher.IsValid() || !CellModel )
    {
        EndOpenEdit( TEXT( "the cell has no model to write to" ) );

        UE_LOGFMT( LogSmartTablesData, VeryVerbose, "Editable cell for '{Column}' row {Row}: {Missing}.", GetColumnId(), GetRowIndex(), Switcher.IsValid() ? TEXT( "no model" ) : TEXT( "no Slate" ) );
        return;
    }

    Editor = CellModel->GetCellEditor( GetRowIndex(), GetColumnId() );

    if ( const USmartTable * OwningTable = GetTable() )
    {

        const FTextBlockStyle & Style = OwningTable->GetCellTextStyle();
        ReadOnlyText->SetTextStyle( &Style,  true );
        TextBox->SetTextBlockStyle( &Style );
    }

    if ( IsEditOpen() )
    {

        const ESmartTableAssignReason Reason = GetAssignReason();
        if ( Reason == ESmartTableAssignReason::Scrolled || Reason == ESmartTableAssignReason::Added )
        {
            EndOpenEdit( TEXT( "the cell now draws another row" ) );
        }
        else if ( Editor != ESmartTableCellEditor::Text && Editor != ESmartTableCellEditor::Number )
        {
            EndOpenEdit( TEXT( "the model offers no box here now" ) );
        }
        else
        {
            UE_LOGFMT( LogSmartTablesData, VeryVerbose, "'{Column}' row {Row} drew again while its box was being typed into. The typing stays for Enter to write.", GetColumnId(), GetRowIndex() );
            return;
        }
    }

    ShowModelValue();
}

void USmartTableEditableCell::NativeOnCellReleased()
{

    ShowNothing();

    Super::NativeOnCellReleased();
}

bool USmartTableEditableCell::IsEditOpen() const
{
    bool bOpen = false;

    if ( TextBox.IsValid() && FSlateApplication::IsInitialized() )
    {

        FSlateApplication::Get().ForEachUser( [ this, &bOpen ]( FSlateUser & User )
        {
            bOpen = bOpen || User.IsWidgetInFocusPath( TextBox );
        },  true );
    }

    return bOpen;
}

void USmartTableEditableCell::EndOpenEdit( const TCHAR * Why )
{
    if ( !TextBox.IsValid() || !FSlateApplication::IsInitialized() )
    {
        return;
    }

    FSlateApplication::Get().ForEachUser( [ this, Why ]( FSlateUser & User )
    {
        if ( !User.IsWidgetInFocusPath( TextBox ) )
        {
            return;
        }

        UE_LOGFMT( LogSmartTablesInput, Verbose, "The edit open in '{Column}' ends, typed text and all: {Why}.", GetColumnId(), Why );

        User.ClearFocus( EFocusCause::Cleared );
    },  true );
}

void USmartTableEditableCell::ShowNothing()
{
    if ( !Switcher.IsValid() )
    {

        return;
    }

    TextBox->SetText( FText::GetEmpty() );
    ReadOnlyText->SetText( FText::GetEmpty() );
    CheckBox->SetIsChecked( ECheckBoxState::Unchecked );

    Switcher->SetActiveWidgetIndex( ReadOnlySlot );

    Editor = ESmartTableCellEditor::None;
}

void USmartTableEditableCell::ShowModelValue()
{
    USmartTableModel * CellModel = GetModel();
    if ( !CellModel )
    {

        UE_LOGFMT( LogSmartTablesData, VeryVerbose, "An editable cell went back to the pool, then something asked it for a value. It draws nothing." );

        ShowNothing();

        return;
    }

    const FText Value = CellModel->GetCellText( GetRowIndex(), GetColumnId() );

    switch ( Editor )
    {
        case ESmartTableCellEditor::Text:
        case ESmartTableCellEditor::Number:

            TextBox->SetJustification( Editor == ESmartTableCellEditor::Number ? ETextJustify::Right : ETextJustify::Left );
            TextBox->SetText( Value );
            Switcher->SetActiveWidgetIndex( TextSlot );
            break;

        case ESmartTableCellEditor::Toggle:

            CheckBox->SetIsChecked( IsCellChecked( Value ) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked );
            Switcher->SetActiveWidgetIndex( ToggleSlot );
            break;

        default:
            ReadOnlyText->SetText( Value );
            Switcher->SetActiveWidgetIndex( ReadOnlySlot );
            break;
    }
}

void USmartTableEditableCell::CommitText( const FText & Value, ETextCommit::Type Cause )
{

    if ( Cause != ETextCommit::OnEnter && Cause != ETextCommit::OnUserMovedFocus )
    {
        ShowModelValue();
        return;
    }

    USmartTableModel * CellModel = GetModel();
    if ( !CellModel || !CellModel->SetCellText( GetRowIndex(), GetColumnId(), Value ) )
    {
        UE_LOGFMT( LogSmartTablesData, Verbose, "'{Column}' row {Row} turned down the value '{Value}'. The cell keeps showing what the model holds.", GetColumnId(), GetRowIndex(), Value.ToString() );
    }

    ShowModelValue();
}

bool USmartTableEditableCell::IsCellChecked( const FText & Drawn ) const
{
    USmartTableModel * CellModel     = GetModel();
    const USmartTable * OwningTable  = GetTable();
    const FSmartTableColumn * Column = OwningTable ? OwningTable->FindColumn( GetColumnId() ) : nullptr;

    if ( CellModel && Column && Column->SortBindingName.IsNone() && GetRowIndex() != INDEX_NONE )
    {
        const FSmartTableSortKey Key = CellModel->GetCellSortKey( GetRowIndex(), GetColumnId() );
        if ( Key.Kind == ESmartTableSortKeyKind::Numeric )
        {
            return Key.Number != 0.0;
        }
    }

    return Drawn.ToString().ToBool();
}

void USmartTableEditableCell::CommitToggle( ECheckBoxState State )
{
    USmartTableModel * CellModel = GetModel();
    if ( !CellModel )
    {
        return;
    }

    CellModel->SetCellText( GetRowIndex(), GetColumnId(), FText::AsCultureInvariant( State == ECheckBoxState::Checked ? TEXT( "true" ) : TEXT( "false" ) ) );

    ShowModelValue();
}
