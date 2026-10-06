// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "Style/SmartTableMenuStyleSet.h"

#include "Logging/StructuredLog.h"
#include "Math/Color.h"
#include "SmartTableLog.h"
#include "SmartTableStyle.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateBrush.h"
#include "Styling/SlateColor.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateTypes.h"

namespace
{

    class FSmartTableMenuStyleSet : public FSlateStyleSet
    {
    public:
        explicit FSmartTableMenuStyleSet( const FName & InName )
            : FSlateStyleSet( InName )
        {
        }

        virtual ~FSmartTableMenuStyleSet() override
        {
            for ( FSlateBrush * Brush : TextureBrushes )
            {
                delete Brush;
            }
        }

        FSlateBrush * Own( const FSlateBrush & Brush )
        {
            FSlateBrush * Copy = new FSlateBrush( Brush );

            if ( Copy->HasUObject() )
            {
                TextureBrushes.Add( Copy );
            }

            return Copy;
        }

    private:
        TArray< FSlateBrush * > TextureBrushes;
    };

    FTextBlockStyle MarkedLabelStyle( const FSmartTableMenuStyle & Style, FName SetName )
    {
        FTextBlockStyle Label = Style.LabelTextStyle;

        if ( !Label.ColorAndOpacity.IsColorSpecified() )
        {
            UE_LOGFMT( LogSmartTables, Verbose, "Menu style '{Set}': the label text colour is set to Inherit, so the search highlight has no colour to draw with and the label style's own Highlight Shape draws instead. Give the label text a colour of its own to use Fill and Outline.", SetName );
            return Label;
        }

        const FSmartTableSearchHighlight & Highlight = Style.SearchHighlight;
        const FLinearColor Colour                    = Label.ColorAndOpacity.GetSpecifiedColor();

        const float Radius         = 3.0f;
        const float FillOpacity    = Highlight.bOutline ? 0.18f : 0.28f;
        const float OutlineOpacity = Highlight.bFill ? 0.6f : 0.75f;

        FSlateBrush Mark;
        Mark.DrawAs          = Highlight.bFill || Highlight.bOutline ? ESlateBrushDrawType::RoundedBox : ESlateBrushDrawType::NoDrawType;
        Mark.TintColor       = FSlateColor( Highlight.bFill ? Colour.CopyWithNewOpacity( Colour.A * FillOpacity ) : FLinearColor::Transparent );
        Mark.OutlineSettings = Highlight.bOutline ? FSlateBrushOutlineSettings( Radius, FSlateColor( Colour.CopyWithNewOpacity( Colour.A * OutlineOpacity ) ), 1.0f ) : FSlateBrushOutlineSettings( Radius );

        Label.SetHighlightShape( Mark );

        return Label;
    }
}

TSharedRef< FSlateStyleSet > MakeSmartTableMenuStyleSet( FName SetName, const FSmartTableMenuStyle & Style )
{
    TSharedRef< FSmartTableMenuStyleSet > Set = MakeShared< FSmartTableMenuStyleSet >( SetName );

    Set->SetParentStyleName( FCoreStyle::Get().GetStyleSetName() );

    Set->Set( "Menu.Background", Set->Own( Style.BackgroundBrush ) );
    Set->Set( "Menu.Separator", Set->Own( Style.SeparatorBrush ) );

    const FTextBlockStyle Label = MarkedLabelStyle( Style, SetName );

    Set->Set( "Menu.Button", Style.EntryStyle );
    Set->Set( "Menu.Label", Label );
    Set->Set( "Menu.Heading", Style.HeadingTextStyle );

    Set->Set( "Menu.Keybinding", Label );

    Set->Set( "Menu.Check", Style.CheckStyle );
    Set->Set( "Menu.CheckBox", Style.CheckStyle );

    return Set;
}
