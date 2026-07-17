#pragma once
#include "SWidget.h"

namespace grom::UI
{

// ============================================================================
// FSlateBrush - Image/Texture brush for rendering
// ============================================================================

class FSlateBrush
{
public:
    FSlateBrush() = default;

    static TSharedPtr<FSlateBrush> CreateTexture(VkImageView TextureView, VkSampler Sampler, const FVector2D& InImageSize)
    {
        auto Brush = MakeShared<FSlateBrush>();
        Brush->TextureView = TextureView;
        Brush->Sampler = Sampler;
        Brush->ImageSize = InImageSize;
        Brush->DrawAs = ESlateBrushDrawType::Image;
        return Brush;
    }

    static TSharedPtr<FSlateBrush> CreateColor(const FLinearColor& InColor)
    {
        auto Brush = MakeShared<FSlateBrush>();
        Brush->TintColor = InColor;
        Brush->DrawAs = ESlateBrushDrawType::Box;
        return Brush;
    }

    static TSharedPtr<FSlateBrush> CreateBorder(const TSharedPtr<FSlateBrush>& InBackground, const FMargin& InMargin, const FLinearColor& InColor)
    {
        auto Brush = MakeShared<FSlateBrush>();
        Brush->BackgroundBrush = InBackground;
        Brush->Margin = InMargin;
        Brush->TintColor = InColor;
        Brush->DrawAs = ESlateBrushDrawType::Border;
        return Brush;
    }

    enum class ESlateBrushDrawType : u8
    {
        NoDrawType,
        Box,
        Border,
        Image,
        RoundedBox,
    };

    // Resource
    VkImageView TextureView = VK_NULL_HANDLE;
    VkSampler Sampler = VK_NULL_HANDLE;
    VkImageLayout ImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    // Appearance
    FVector2D ImageSize = FVector2D(0, 0);
    FVector2D UVStart = FVector2D(0, 0);
    FVector2D UVSize = FVector2D(1, 1);
    FLinearColor TintColor = FLinearColor::White;
    FLinearColor Color = FLinearColor::White;
    FLinearColor BackgroundColor = FLinearColor::White;
    FLinearColor BorderBackgroundColor = FLinearColor::White;

    TSharedPtr<FSlateBrush> BackgroundBrush;
    FMargin Margin = FMargin(0, 0);

    ESlateBrushDrawType DrawAs = ESlateBrushDrawType::NoDrawType;
    float CornerRadius = 0.0f;

    // Tiling
    ESlateBrushTileType Tiling = ESlateBrushTileType::NoTile;
    FVector2D TilingScale = FVector2D(1, 1);
};

enum class ESlateBrushTileType : u8
{
    NoTile,
    Horizontal,
    Vertical,
    Both,
};

// ============================================================================
// SImage - Simple image widget
// ============================================================================

class SImage : public SWidget
{
public:
    SImage() = default;
    explicit SImage(const TSharedPtr<FSlateBrush>& InBrush) : Brush(InBrush) {}

    TSharedPtr<FSlateBrush> GetBrush() const { return Brush; }
    void SetBrush(const TSharedPtr<FSlateBrush>& InBrush) { Brush = InBrush; }
    void SetBrushFromTexture(VkImageView Texture, VkSampler Sampler, const FVector2D& Size)
    {
        Brush = FSlateBrush::CreateTexture(Texture, Sampler, Size);
    }
    void SetColorAndOpacity(const FLinearColor& InColor) { ColorAndOpacity = InColor; }
    FLinearColor GetColorAndOpacity() const { return ColorAndOpacity; }

    virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, TArray<FArrangedWidget>& OutArrangedWidgets) const override
    {
        if (Brush && Brush->DrawAs != FSlateBrush::ESlateBrushDrawType::NoDrawType)
        {
            FGeometry ChildGeometry(FVector2D(0, 0), AllottedGeometry.GetLocalSize(), AllottedGeometry.GetLocalSize());
            FArrangedWidget Arranged;
            Arranged.Widget = SharedThis(this);
            Arranged.Geometry = ChildGeometry;
            OutArrangedWidgets.Add(Arranged);
        }
    }

    virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override
    {
        if (Brush)
            return Brush->ImageSize;
        return FVector2D(0, 0);
    }

    virtual int32 OnPaint(const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
                          TArray<FDrawElement>& OutDrawElements, int32 LayerId,
                          const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override
    {
        if (!Brush || Brush->DrawAs == FSlateBrush::ESlateBrushDrawType::NoDrawType)
            return 0;

        FLinearColor FinalColor = Brush->TintColor * ColorAndOpacity * InWidgetStyle.GetColorAndOpacityTint();

        FDrawElement Element;
        Element.Geometry = FGeometry(FVector2D(0, 0), GetCachedGeometry().GetLocalSize(), GetCachedGeometry().GetLocalSize());
        Element.PrimitiveType = EPrimitiveType::TriangleStrip;
        Element.Brush = Brush;
        Element.Color = FinalColor;
        Element.LayerId = LayerId;

        OutDrawElements.Add(Element);
        return LayerId + 1;
    }

private:
    TSharedPtr<FSlateBrush> Brush;
    FLinearColor ColorAndOpacity = FLinearColor::White;
};

// ============================================================================
// STextBlock - Text rendering widget
// ============================================================================

class STextBlock : public SWidget
{
public:
    STextBlock() = default;

    void SetText(const GString& InText) { Text = InText; }
    GString GetText() const { return Text; }
    void SetFont(const TSharedPtr<class FSlateFontInfo>& InFont) { Font = InFont; }
    void SetColorAndOpacity(const FLinearColor& InColor) { ColorAndOpacity = InColor; }
    void SetShadowOffset(const FVector2D& InOffset) { ShadowOffset = InOffset; }
    void SetShadowColorAndOpacity(const FLinearColor& InColor) { ShadowColorAndOpacity = InColor; }
    void SetJustification(ETextJustify InJustification) { Justification = InJustification; }
    void SetWrapTextAt(float InWrapWidth) { WrapTextAt = InWrapWidth; }

    virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override
    {
        if (!Font)
            return FVector2D(0, 0);

        // Would measure text with font
        return FVector2D(100, 20); // Placeholder
    }

    virtual int32 OnPaint(const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
                          TArray<FDrawElement>& OutDrawElements, int32 LayerId,
                          const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override
    {
        if (Text.IsEmpty() || !Font)
            return LayerId;

        // Would render text via font atlas
        return LayerId;
    }

private:
    GString Text;
    TSharedPtr<class FSlateFontInfo> Font;
    FLinearColor ColorAndOpacity = FLinearColor::White;
    FVector2D ShadowOffset = FVector2D(0, 0);
    FLinearColor ShadowColorAndOpacity = FLinearColor::Transparent;
    ETextJustify Justification = ETextJustify::Left;
    float WrapTextAt = 0.0f;
};

// ============================================================================
// SButton - Button widget
// ============================================================================

class SButton : public SPanel
{
public:
    struct FSlot : public ISlot
    {
        FSlot() = default;
        explicit FSlot(TSharedPtr<SWidget> InWidget) : ISlot(InWidget) {}

        FSlot& HAlign(EHorizontalAlignment Align) { HAlign = Align; return *this; }
        FSlot& VAlign(EVerticalAlignment Align) { VAlign = Align; return *this; }
        FSlot& Padding(const FMargin& InPadding) { Padding = InPadding; return *this; }
    };

    SButton() = default;

    TSharedRef<FSlot> SetContent(TSharedPtr<SWidget> InContent)
    {
        ContentSlot = MakeShared<FSlot>(InContent);
        return ContentSlot;
    }

    // Button style
    struct FButtonStyle
    {
        TSharedPtr<FSlateBrush> Normal;
        TSharedPtr<FSlateBrush> Hovered;
        TSharedPtr<FSlateBrush> Pressed;
        TSharedPtr<FSlateBrush> Disabled;
        FLinearColor NormalForeground = FLinearColor::White;
        FLinearColor HoveredForeground = FLinearColor::White;
        FLinearColor PressedForeground = FLinearColor::White;
        FLinearColor DisabledForeground = FLinearColor::Gray;
        FMargin ContentPadding = FMargin(4, 2);
    };

    void SetButtonStyle(const FButtonStyle& InStyle) { Style = InStyle; }
    const FButtonStyle& GetButtonStyle() const { return Style; }

    // Events
    using FOnClicked = TFunction<void()>;
    FOnClicked OnClicked;

    virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
    {
        if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && IsEnabled())
        {
            bIsPressed = true;
            return FReply::Handled().CaptureMouse(SharedThis(this));
        }
        return FReply::Unhandled();
    }

    virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
    {
        if (bIsPressed && MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
        {
            bIsPressed = false;
            if (IsEnabled() && OnClicked)
                OnClicked();
            return FReply::Handled().ReleaseMouseCapture();
        }
        return FReply::Unhandled();
    }

    virtual FReply OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
    {
        bIsHovered = true;
        return FReply::Handled();
    }

    virtual FReply OnMouseLeave(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
    {
        bIsHovered = false;
        bIsPressed = false;
        return FReply::Handled();
    }

    virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, TArray<FArrangedWidget>& OutArrangedWidgets) const override
    {
        if (ContentSlot && ContentSlot->GetWidget() && ContentSlot->GetWidget()->IsVisible())
        {
            FGeometry ChildGeometry = AllottedGeometry.MakeChild(
                ContentSlot->GetWidget()->GetDesiredSize(),
                FVector2D(Style.ContentPadding.Left, Style.ContentPadding.Top),
                FVector2D(1, 1),
                ContentSlot->HAlign,
                ContentSlot->VAlign
            );

            FArrangedWidget Arranged;
            Arranged.Widget = ContentSlot->GetWidget();
            Arranged.Geometry = ChildGeometry;
            OutArrangedWidgets.Add(Arranged);
        }
    }

    virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override
    {
        FVector2D Size(0, 0);
        if (ContentSlot && ContentSlot->GetWidget() && ContentSlot->GetWidget()->IsVisible())
        {
            Size = ContentSlot->GetWidget()->GetDesiredSize();
            Size.X += Style.ContentPadding.Left + Style.ContentPadding.Right;
            Size.Y += Style.ContentPadding.Top + Style.ContentPadding.Bottom;
        }
        return Size;
    }

    virtual int32 OnPaint(const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
                          TArray<FDrawElement>& OutDrawElements, int32 LayerId,
                          const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override
    {
        if (!IsEnabled())
        {
            // Draw disabled
        }
        else if (bIsPressed && bIsHovered)
        {
            // Draw pressed
        }
        else if (bIsHovered)
        {
            // Draw hovered
        }
        else
        {
            // Draw normal
        }

        // Paint children
        return SPanel::OnPaint(AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
    }

private:
    FButtonStyle Style;
    TSharedPtr<FSlot> ContentSlot;
    bool bIsPressed = false;
    bool bIsHovered = false;
};

// ============================================================================
// SEditableText - Single line editable text
// ============================================================================

class SEditableText : public SWidget
{
public:
    SEditableText() = default;

    void SetText(const GString& InText) { Text = InText; }
    GString GetText() const { return Text; }
    void SetHintText(const GString& InHint) { HintText = InHint; }
    void SetFont(const TSharedPtr<class FSlateFontInfo>& InFont) { Font = InFont; }
    void SetColorAndOpacity(const FLinearColor& InColor) { ColorAndOpacity = InColor; }
    void SetReadOnly(bool bInReadOnly) { bIsReadOnly = bInReadOnly; }
    void SetIsPassword(bool bInPassword) { bIsPassword = bInPassword; }
    void SetOnTextChanged(TFunction<void(const GString&)> InCallback) { OnTextChanged = InCallback; }
    void SetOnTextCommitted(TFunction<void(const GString&, ETextCommit)> InCallback) { OnTextCommitted = InCallback; }

    virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override
    {
        return FVector2D(200, 24);
    }

    virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent) override
    {
        // Handle text input
        return FReply::Unhandled();
    }

    virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
    {
        // Focus on click
        return FReply::Handled().CaptureMouse(SharedThis(this));
    }

private:
    GString Text;
    GString HintText;
    TSharedPtr<class FSlateFontInfo> Font;
    FLinearColor ColorAndOpacity = FLinearColor::White;
    bool bIsReadOnly = false;
    bool bIsPassword = false;
    TFunction<void(const GString&)> OnTextChanged;
    TFunction<void(const GString&, ETextCommit)> OnTextCommitted;
};

} // namespace grom::UI