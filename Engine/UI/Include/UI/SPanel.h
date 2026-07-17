#pragma once
#include "SPanel.h"

namespace grom::UI
{

// ============================================================================
// SVerticalBox - Vertical layout container
// ============================================================================

class SVerticalBox : public SPanel
{
public:
    struct FSlot : public ISlot
    {
        FSlot() = default;
        explicit FSlot(TSharedPtr<SWidget> InWidget) : ISlot(InWidget) {}

        FSlot& AutoHeight() { bAutoHeight = true; return *this; }
        FSlot& FillHeight(float InHeight) { bAutoHeight = false; FillHeight = InHeight; return *this; }
        FSlot& HAlign(EHorizontalAlignment Align) { HAlign = Align; return *this; }
        FSlot& VAlign(EVerticalAlignment Align) { VAlign = Align; return *this; }
        FSlot& Padding(const FMargin& InPadding) { Padding = InPadding; return *this; }

        bool bAutoHeight = true;
        float FillHeight = 1.0f;
    };

    SVerticalBox() = default;

    TSharedRef<FSlot> AddSlot()
    {
        auto Slot = MakeShared<FSlot>();
        Slots.Add(Slot);
        return Slot;
    }

    TSharedRef<FSlot> AddSlot(TSharedPtr<SWidget> Widget)
    {
        auto Slot = AddSlot();
        Slot->SetWidget(Widget);
        return Slot;
    }

    virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, TArray<FArrangedWidget>& OutArrangedWidgets) const override
    {
        float Y = 0;
        float TotalFillWeight = 0;
        int32 AutoHeightCount = 0;

        // First pass: calculate total fill weight and auto heights
        for (const auto& Slot : Slots)
        {
            if (!Slot || !Slot->GetWidget() || !Slot->GetWidget()->IsVisible())
                continue;

            if (Slot->bAutoHeight)
            {
                AutoHeightCount++;
            }
            else
            {
                TotalFillWeight += Slot->FillHeight;
            }
        }

        // Calculate available space for fill slots
        float AvailableHeight = AllottedGeometry.GetLocalSize().Y;
        float UsedHeight = 0;

        for (const auto& Slot : Slots)
        {
            if (!Slot || !Slot->GetWidget() || !Slot->GetWidget()->IsVisible())
                continue;

            FVector2D ChildDesiredSize = Slot->GetWidget()->GetDesiredSize();
            float SlotHeight = 0;

            if (Slot->bAutoHeight)
            {
                SlotHeight = ChildDesiredSize.Y + Slot->Padding.Top + Slot->Padding.Bottom;
            }
            else
            {
                float RemainingHeight = AllottedGeometry.GetLocalSize().Y - UsedHeight;
                int32 RemainingAuto = 0;
                for (int32 i = Slots.IndexOf(Slot) + 1; i < Slots.Size(); ++i)
                {
                    if (Slots[i] && Slots[i]->bAutoHeight && Slots[i]->GetWidget() && Slots[i]->GetWidget()->IsVisible())
                        RemainingAuto++;
                }
                float RemainingAutoHeight = 0;
                for (int32 i = Slots.IndexOf(Slot) + 1; i < Slots.Size(); ++i)
                {
                    if (Slots[i] && Slots[i]->bAutoHeight && Slots[i]->GetWidget() && Slots[i]->GetWidget()->IsVisible())
                    {
                        RemainingAutoHeight += Slots[i]->GetWidget()->GetDesiredSize().Y + Slots[i]->Padding.Top + Slots[i]->Padding.Bottom;
                    }
                }
                float FillSpace = AllottedGeometry.GetLocalSize().Y - UsedHeight - RemainingAutoHeight;
                SlotHeight = FillSpace * (Slot->FillHeight / TotalFillWeight);
            }

            FVector2D SlotSize(AllottedGeometry.GetLocalSize().X - Slot->Padding.Left - Slot->Padding.Right,
                               SlotHeight - Slot->Padding.Top - Slot->Padding.Bottom);

            FVector2D SlotPosition(Slot->Padding.Left, Y + Slot->Padding.Top);

            // Apply horizontal alignment
            float WidgetWidth = Slot->GetWidget()->GetDesiredSize().X;
            float AvailableWidth = AllottedGeometry.GetLocalSize().X - Slot->Padding.Left - Slot->Padding.Right;
            float OffsetX = 0;
            switch (Slot->HAlign)
            {
            case EHorizontalAlignment::Left: OffsetX = 0; break;
            case EHorizontalAlignment::Center: OffsetX = (AvailableWidth - Slot->GetWidget()->GetDesiredSize().X) * 0.5f; break;
            case EHorizontalAlignment::Right: OffsetX = AvailableWidth - Slot->GetWidget()->GetDesiredSize().X; break;
            case EHorizontalAlignment::Fill: break;
            }
            SlotPosition.X += OffsetX;

            FVector2D SlotPos = AllottedGeometry.AbsoluteToLocal(FVector2D(SlotPosition.X, SlotPosition.Y));
            FGeometry ChildGeometry(FVector2D(SlotPosition.X, SlotPosition.Y), SlotSize, SlotSize);

            FArrangedWidget Arranged;
            Arranged.Widget = Slot->GetWidget();
            Arranged.Geometry = ChildGeometry;
            OutArrangedWidgets.Add(Arranged);

            Y += SlotHeight;
            UsedHeight += SlotHeight;
        }
    }

    virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override
    {
        float Width = 0, Height = 0;
        for (const auto& Slot : Slots)
        {
            if (!Slot || !Slot->GetWidget() || !Slot->GetWidget()->IsVisible())
                continue;

            FVector2D ChildSize = Slot->GetWidget()->GetDesiredSize();
            if (Slot->bAutoHeight)
            {
                Height += ChildSize.Y + Slot->Padding.Top + Slot->Padding.Bottom;
            }
            else
            {
                Height += Slot->FillHeight; // Will be distributed
            }
            Width = FMath::Max(Width, ChildSize.X + Slot->Padding.Left + Slot->Padding.Right);
        }
        return FVector2D(Width, Height);
    }

private:
    TArray<TSharedRef<FSlot>> Slots;
};

// ============================================================================
// SHorizontalBox - Horizontal layout container
// ============================================================================

class SHorizontalBox : public SPanel
{
public:
    struct FSlot : public ISlot
    {
        FSlot() = default;
        explicit FSlot(TSharedPtr<SWidget> InWidget) : ISlot(InWidget) {}

        FSlot& AutoWidth() { bAutoWidth = true; return *this; }
        FSlot& FillWidth(float InWidth) { bAutoWidth = false; FillWidth = InWidth; return *this; }
        FSlot& HAlign(EHorizontalAlignment Align) { HAlign = Align; return *this; }
        FSlot& VAlign(EVerticalAlignment Align) { VAlign = Align; return *this; }
        FSlot& Padding(const FMargin& InPadding) { Padding = InPadding; return *this; }

        bool bAutoWidth = true;
        float FillWidth = 1.0f;
    };

    SHorizontalBox() = default;

    TSharedRef<FSlot> AddSlot()
    {
        auto Slot = MakeShared<FSlot>();
        Slots.Add(Slot);
        return Slot;
    }

    TSharedRef<FSlot> AddSlot(TSharedPtr<SWidget> Widget)
    {
        auto Slot = AddSlot();
        Slot->SetWidget(Widget);
        return Slot;
    }

    virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, TArray<FArrangedWidget>& OutArrangedWidgets) const override
    {
        // Similar to SVerticalBox but horizontal
        float X = 0;
        float TotalFillWeight = 0;

        for (const auto& Slot : Slots)
        {
            if (!Slot || !Slot->GetWidget() || !Slot->GetWidget()->IsVisible())
                continue;

            if (!Slot->bAutoWidth)
                TotalFillWeight += Slot->FillWidth;
        }

        float UsedWidth = 0;

        for (const auto& Slot : Slots)
        {
            if (!Slot || !Slot->GetWidget() || !Slot->GetWidget()->IsVisible())
                continue;

            FVector2D ChildDesiredSize = Slot->GetWidget()->GetDesiredSize();
            float SlotWidth = 0;

            if (Slot->bAutoWidth)
            {
                SlotWidth = ChildDesiredSize.X + Slot->Padding.Left + Slot->Padding.Right;
            }
            else
            {
                float RemainingWidth = AllottedGeometry.GetLocalSize().X - UsedWidth;
                float FillSpace = AllottedGeometry.GetLocalSize().X - UsedWidth; // Simplified
                SlotWidth = FillSpace * (Slot->FillWidth / TotalFillWeight);
            }

            FVector2D SlotSize(SlotWidth - Slot->Padding.Left - Slot->Padding.Right,
                               AllottedGeometry.GetLocalSize().Y - Slot->Padding.Top - Slot->Padding.Bottom);

            FVector2D SlotPosition(X + Slot->Padding.Left, Slot->Padding.Top);

            FGeometry ChildGeometry(FVector2D(SlotPosition.X, SlotPosition.Y), SlotSize, SlotSize);

            FArrangedWidget Arranged;
            Arranged.Widget = Slot->GetWidget();
            Arranged.Geometry = ChildGeometry;
            OutArrangedWidgets.Add(Arranged);

            X += SlotWidth;
            UsedWidth += SlotWidth;
        }
    }

    virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override
    {
        float Width = 0, Height = 0;
        for (const auto& Slot : Slots)
        {
            if (!Slot || !Slot->GetWidget() || !Slot->GetWidget()->IsVisible())
                continue;

            FVector2D ChildSize = Slot->GetWidget()->GetDesiredSize();
            if (Slot->bAutoWidth)
            {
                Width += ChildSize.X + Slot->Padding.Left + Slot->Padding.Right;
            }
            else
            {
                Width += Slot->FillWidth;
            }
            Height = FMath::Max(Height, ChildSize.Y + Slot->Padding.Top + Slot->Padding.Bottom);
        }
        return FVector2D(Width, Height);
    }

private:
    TArray<TSharedRef<FSlot>> Slots;
};

// ============================================================================
// SOverlay - Overlay layout (z-order stacking)
// ============================================================================

class SOverlay : public SPanel
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

    SOverlay() = default;

    TSharedRef<FSlot> AddSlot()
    {
        auto Slot = MakeShared<FSlot>();
        Slots.Add(Slot);
        return Slot;
    }

    TSharedRef<FSlot> AddSlot(TSharedPtr<SWidget> Widget)
    {
        auto Slot = AddSlot();
        Slot->SetWidget(Widget);
        return Slot;
    }

    virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, TArray<FArrangedWidget>& OutArrangedWidgets) const override
    {
        for (const auto& Slot : Slots)
        {
            if (!Slot || !Slot->GetWidget() || !Slot->GetWidget()->IsVisible())
                continue;

            FVector2D ChildDesiredSize = Slot->GetWidget()->GetDesiredSize();
            FVector2D SlotSize(AllottedGeometry.GetLocalSize().X - Slot->Padding.Left - Slot->Padding.Right,
                               AllottedGeometry.GetLocalSize().Y - Slot->Padding.Top - Slot->Padding.Bottom);

            FVector2D SlotPosition(Slot->Padding.Left, Slot->Padding.Top);

            // Horizontal alignment
            switch (Slot->HAlign)
            {
            case EHorizontalAlignment::Left: break;
            case EHorizontalAlignment::Center: SlotPosition.X += (AllottedGeometry.GetLocalSize().X - ChildDesiredSize.X - Slot->Padding.Left - Slot->Padding.Right) * 0.5f; break;
            case EHorizontalAlignment::Right: SlotPosition.X += AllottedGeometry.GetLocalSize().X - ChildDesiredSize.X - Slot->Padding.Left - Slot->Padding.Right; break;
            case EHorizontalAlignment::Fill: break;
            }

            // Vertical alignment
            switch (Slot->VAlign)
            {
            case EVerticalAlignment::Top: break;
            case EVerticalAlignment::Center: SlotPosition.Y += (AllottedGeometry.GetLocalSize().Y - ChildDesiredSize.Y - Slot->Padding.Top - Slot->Padding.Bottom) * 0.5f; break;
            case EVerticalAlignment::Bottom: SlotPosition.Y += AllottedGeometry.GetLocalSize().Y - ChildDesiredSize.Y - Slot->Padding.Top - Slot->Padding.Bottom; break;
            case EVerticalAlignment::Fill: break;
            }

            FVector2D SlotSize(AllottedGeometry.GetLocalSize().X - Slot->Padding.Left - Slot->Padding.Right,
                               AllottedGeometry.GetLocalSize().Y - Slot->Padding.Top - Slot->Padding.Bottom);

            FGeometry ChildGeometry(FVector2D(SlotPosition.X, SlotPosition.Y), SlotSize, SlotSize);

            FArrangedWidget Arranged;
            Arranged.Widget = Slot->GetWidget();
            Arranged.Geometry = ChildGeometry;
            OutArrangedWidgets.Add(Arranged);
        }
    }

    virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override
    {
        FVector2D MaxSize(0, 0);
        for (const auto& Slot : Slots)
        {
            if (!Slot || !Slot->GetWidget() || !Slot->GetWidget()->IsVisible())
                continue;

            FVector2D ChildSize = Slot->GetWidget()->GetDesiredSize();
            MaxSize.X = FMath::Max(MaxSize.X, ChildSize.X + Slot->Padding.Left + Slot->Padding.Right);
            MaxSize.Y = FMath::Max(MaxSize.Y, ChildSize.Y + Slot->Padding.Top + Slot->Padding.Bottom);
        }
        return MaxSize;
    }

private:
    TArray<TSharedRef<FSlot>> Slots;
};

// ============================================================================
// SBorder - Border with background
// ============================================================================

class SBorder : public SPanel
{
public:
    struct FSlot : public ISlot
    {
        FSlot() = default;
        explicit FSlot(TSharedPtr<SWidget> InWidget) : ISlot(InWidget) {}

        FSlot& BorderImage(const TSharedPtr<class FSlateBrush>& InBrush) { BorderBrush = InBrush; return *this; }
        FSlot& BorderBackgroundColor(const FLinearColor& InColor) { BorderBackgroundColor = InColor; return *this; }
        FSlot& HAlign(EHorizontalAlignment Align) { HAlign = Align; return *this; }
        FSlot& VAlign(EVerticalAlignment Align) { VAlign = Align; return *this; }
        FSlot& Padding(const FMargin& InPadding) { Padding = InPadding; return *this; }

        TSharedPtr<class FSlateBrush> BorderBrush;
        FLinearColor BorderBackgroundColor = FLinearColor::White;
    };

    SBorder() = default;

    TSharedRef<FSlot> SetContent(TSharedPtr<SWidget> InContent)
    {
        ContentSlot = MakeShared<FSlot>(InContent);
        return ContentSlot;
    }

    void SetBorderImage(const TSharedPtr<class FSlateBrush>& InBrush)
    {
        if (ContentSlot)
            ContentSlot->BorderBrush = InBrush;
    }

    void SetBorderBackgroundColor(const FLinearColor& InColor)
    {
        if (ContentSlot)
            ContentSlot->BorderBackgroundColor = InColor;
    }

    virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, TArray<FArrangedWidget>& OutArrangedWidgets) const override
    {
        if (ContentSlot && ContentSlot->GetWidget() && ContentSlot->GetWidget()->IsVisible())
        {
            FVector2D ChildDesiredSize = ContentSlot->GetWidget()->GetDesiredSize();
            FVector2D SlotSize(AllottedGeometry.GetLocalSize().X - ContentSlot->Padding.Left - ContentSlot->Padding.Right,
                               AllottedGeometry.GetLocalSize().Y - ContentSlot->Padding.Top - ContentSlot->Padding.Bottom);

            FVector2D SlotPosition(ContentSlot->Padding.Left, ContentSlot->Padding.Top);

            // Alignment
            switch (ContentSlot->HAlign)
            {
            case EHorizontalAlignment::Center: SlotPosition.X += (AllottedGeometry.GetLocalSize().X - ContentSlot->GetWidget()->GetDesiredSize().X - ContentSlot->Padding.Left - ContentSlot->Padding.Right) * 0.5f; break;
            case EHorizontalAlignment::Right: SlotPosition.X += AllottedGeometry.GetLocalSize().X - ContentSlot->GetWidget()->GetDesiredSize().X - ContentSlot->Padding.Left - ContentSlot->Padding.Right; break;
            case EHorizontalAlignment::Fill: break;
            }

            FVector2D SlotSize(AllottedGeometry.GetLocalSize().X - ContentSlot->Padding.Left - ContentSlot->Padding.Right,
                               AllottedGeometry.GetLocalSize().Y - ContentSlot->Padding.Top - ContentSlot->Padding.Bottom);

            FGeometry ChildGeometry(FVector2D(SlotPosition.X, SlotPosition.Y), SlotSize, SlotSize);

            FArrangedWidget Arranged;
            Arranged.Widget = ContentSlot->GetWidget();
            Arranged.Geometry = ChildGeometry;
            OutArrangedWidgets.Add(Arranged);
        }
    }

    virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override
    {
        if (ContentSlot && ContentSlot->GetWidget() && ContentSlot->GetWidget()->IsVisible())
        {
            FVector2D ChildSize = ContentSlot->GetWidget()->GetDesiredSize();
            return FVector2D(ChildSize.X + ContentSlot->Padding.Left + ContentSlot->Padding.Right,
                           ChildSize.Y + ContentSlot->Padding.Top + ContentSlot->Padding.Bottom);
        }
        return FVector2D(0, 0);
    }

private:
    TSharedPtr<FSlot> ContentSlot;
};

// ============================================================================
// SBox - Simple layout box with size constraints
// ============================================================================

class SBox : public SPanel
{
public:
    struct FSlot : public ISlot
    {
        FSlot() = default;
        explicit FSlot(TSharedPtr<SWidget> InWidget) : ISlot(InWidget) {}

        FSlot& WidthOverride(float InWidth) { bWidthOverride = true; WidthOverride = InWidth; return *this; }
        FSlot& HeightOverride(float InHeight) { bHeightOverride = true; HeightOverride = InHeight; return *this; }
        FSlot& MinDesiredWidth(float InWidth) { MinDesiredWidth = InWidth; return *this; }
        FSlot& MinDesiredHeight(float InHeight) { MinDesiredHeight = InHeight; return *this; }
        FSlot& MaxDesiredWidth(float InWidth) { bMaxWidth = true; MaxDesiredWidth = InWidth; return *this; }
        FSlot& MaxDesiredHeight(float InHeight) { bMaxHeight = true; MaxDesiredHeight = InHeight; return *this; }
        FSlot& HAlign(EHorizontalAlignment Align) { HAlign = Align; return *this; }
        FSlot& VAlign(EVerticalAlignment Align) { VAlign = Align; return *this; }
        FSlot& Padding(const FMargin& InPadding) { Padding = InPadding; return *this; }

        bool bWidthOverride = false;
        float WidthOverride = 0;
        bool bHeightOverride = false;
        float HeightOverride = 0;
        float MinDesiredWidth = 0;
        float MinDesiredHeight = 0;
        bool bMaxWidth = false;
        float MaxDesiredWidth = 0;
        bool bMaxHeight = false;
        float MaxDesiredHeight = 0;
    };

    SBox() = default;

    TSharedRef<FSlot> SetContent(TSharedPtr<SWidget> InContent)
    {
        ContentSlot = MakeShared<FSlot>(InContent);
        return ContentSlot;
    }

    virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, TArray<FArrangedWidget>& OutArrangedWidgets) const override
    {
        if (ContentSlot && ContentSlot->GetWidget() && ContentSlot->GetWidget()->IsVisible())
        {
            FVector2D ChildDesiredSize = ContentSlot->GetWidget()->GetDesiredSize();

            float FinalWidth = ContentSlot->bWidthOverride ? ContentSlot->WidthOverride : ChildDesiredSize.X;
            float FinalHeight = ContentSlot->bHeightOverride ? ContentSlot->HeightOverride : ChildDesiredSize.Y;

            FinalWidth = FMath::Max(FinalWidth, ContentSlot->MinDesiredWidth);
            FinalHeight = FMath::Max(FinalHeight, ContentSlot->MinDesiredHeight);
            if (ContentSlot->bMaxWidth) FinalWidth = FMath::Min(FinalWidth, ContentSlot->MaxDesiredWidth);
            if (ContentSlot->bMaxHeight) FinalHeight = FMath::Min(FinalHeight, ContentSlot->MaxDesiredHeight);

            FVector2D SlotSize(FinalWidth, FinalHeight);
            FVector2D SlotPosition(ContentSlot->Padding.Left, ContentSlot->Padding.Top);

            // Alignment
            switch (ContentSlot->HAlign)
            {
            case EHorizontalAlignment::Center: SlotPosition.X += (AllottedGeometry.GetLocalSize().X - FinalWidth) * 0.5f; break;
            case EHorizontalAlignment::Right: SlotPosition.X += AllottedGeometry.GetLocalSize().X - FinalWidth; break;
            case EHorizontalAlignment::Fill: break;
            }

            FVector2D SlotSize(FinalWidth, FinalHeight);
            FGeometry ChildGeometry(FVector2D(ContentSlot->Padding.Left, ContentSlot->Padding.Top) + SlotPosition, SlotSize, SlotSize);

            FArrangedWidget Arranged;
            Arranged.Widget = ContentSlot->GetWidget();
            Arranged.Geometry = ChildGeometry;
            OutArrangedWidgets.Add(Arranged);
        }
    }

    virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override
    {
        if (ContentSlot && ContentSlot->GetWidget() && ContentSlot->GetWidget()->IsVisible())
        {
            FVector2D ChildSize = ContentSlot->GetWidget()->GetDesiredSize();

            float FinalWidth = ContentSlot->bWidthOverride ? ContentSlot->WidthOverride : ChildSize.X;
            float FinalHeight = ContentSlot->bHeightOverride ? ContentSlot->HeightOverride : ChildSize.Y;

            FinalWidth = FMath::Max(FinalWidth, ContentSlot->MinDesiredWidth);
            FinalHeight = FMath::Max(FinalHeight, ContentSlot->MinDesiredHeight);
            if (ContentSlot->bMaxWidth) FinalWidth = FMath::Min(FinalWidth, ContentSlot->MaxDesiredWidth);
            if (ContentSlot->bMaxHeight) FinalHeight = FMath::Min(FinalHeight, ContentSlot->MaxDesiredHeight);

            return FVector2D(FinalWidth + ContentSlot->Padding.Left + ContentSlot->Padding.Right,
                           FinalHeight + ContentSlot->Padding.Top + ContentSlot->Padding.Bottom);
        }
        return FVector2D(0, 0);
    }

private:
    TSharedPtr<FSlot> ContentSlot;
};

} // namespace grom::UI