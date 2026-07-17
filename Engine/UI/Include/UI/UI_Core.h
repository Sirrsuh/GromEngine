#pragma once
#include <Core/Types.h>
#include <Core/Container.h>
#include <Core/Math.h>
#include <Core/Memory.h>
#include <RHI/RHI_Texture.h>

namespace grom::UI
{

// Forward declarations
struct FGeometry;
struct FPaintArgs;
class SWidget;
class SPanel;
class FSlateRenderer;
class FSlateApplication;
struct FInputEvent;
struct FKeyEvent;
struct FPointerEvent;
struct FCharacterEvent;

// ============================================================================
// Enums
// ============================================================================

enum class EHorizontalAlignment : u8
{
    Left,
    Center,
    Right,
    Fill,
};

enum class EVerticalAlignment : u8
{
    Top,
    Center,
    Bottom,
    Fill,
};

enum class EVisibility : u8
{
    Visible,
    Hidden,
    Collapsed,
    HitTestInvisible,
    SelfHitTestInvisible,
};

enum class ETextJustify : u8
{
    Left,
    Center,
    Right,
};

enum class ETextFlowDirection : u8
{
    LeftToRight,
    RightToLeft,
};

enum class EFontHinting : u8
{
    Default,
    None,
    Light,
    Normal,
    Full,
};

enum class EMouseCursor : u8
{
    None,
    Default,
    TextEdit,
    ResizeLeftRight,
    ResizeUpDown,
    ResizeNESW,
    ResizeNWSE,
    Hand,
    Grab,
    GrabClosed,
    Crosshairs,
    NotAllowed,
};

enum class EMouseButton : u8
{
    Left,
    Right,
    Middle,
    Thumb1,
    Thumb2,
};

enum class EKey : u16
{
    None,
    // Letters
    A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    // Numbers
    Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
    // Function keys
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    // Modifiers
    LeftShift, RightShift, LeftCtrl, RightCtrl, LeftAlt, RightAlt, LeftCmd, RightCmd,
    // Special
    Enter, Escape, Space, Tab, Backspace, Delete, Insert, Home, End, PageUp, PageDown,
    Up, Down, Left, Right,
    // Numpad
    NumPad0, NumPad1, NumPad2, NumPad3, NumPad4, NumPad5, NumPad6, NumPad7, NumPad8, NumPad9,
    NumPadAdd, NumPadSubtract, NumPadMultiply, NumPadDivide, NumPadDecimal, NumPadEnter,
    // Other
    PrintScreen, ScrollLock, Pause, CapsLock, NumLock,
};

enum class EInputEvent : u8
{
    Pressed,
    Released,
    Repeat,
    DoubleClick,
    Wheel,
    Move,
};

enum class ENavigation : u8
{
    Up,
    Down,
    Left,
    Right,
    Next,
    Previous,
    Accept,
    Cancel,
};

// ============================================================================
// Math/Geometry
// ============================================================================

struct FVector2D
{
    f32 X = 0.0f, Y = 0.0f;

    FVector2D() = default;
    FVector2D(f32 InX, f32 InY) : X(InX), Y(InY) {}

    FVector2D& operator+=(const FVector2D& Other) { X += Other.X; Y += Other.Y; return *this; }
    FVector2D& operator-=(const FVector2D& Other) { X -= Other.X; Y -= Other.Y; return *this; }
    FVector2D& operator*=(f32 Scale) { X *= Scale; Y *= Scale; return *this; }
    FVector2D& operator/=(f32 Scale) { X /= Scale; Y /= Scale; return *this; }

    friend FVector2D operator+(FVector2D A, const FVector2D& B) { return A += B; }
    friend FVector2D operator-(FVector2D A, const FVector2D& B) { return A -= B; }
    friend FVector2D operator*(FVector2D A, f32 Scale) { return A *= Scale; }
    friend FVector2D operator*(f32 Scale, FVector2D A) { return A *= Scale; }
    friend FVector2D operator/(FVector2D A, f32 Scale) { return A /= Scale; }

    bool operator==(const FVector2D& Other) const { return X == Other.X && Y == Other.Y; }
    bool operator!=(const FVector2D& Other) const { return !(*this == Other); }

    f32 Size() const { return sqrtf(X*X + Y*Y); }
    f32 SizeSquared() const { return X*X + Y*Y; }
    FVector2D GetSafeNormal() const { f32 S = Size(); return S > 0 ? *this / S : FVector2D(0, 0); }
    f32 Dot(const FVector2D& Other) const { return X*Other.X + Y*Other.Y; }
};

struct FVector2f { f32 X, Y; }; // Alias

struct FIntPoint
{
    i32 X = 0, Y = 0;
    FIntPoint() = default;
    FIntPoint(i32 InX, i32 InY) : X(InX), Y(InY) {}
    bool operator==(const FIntPoint& Other) const { return X == Other.X && Y == Other.Y; }
};

struct FIntRect
{
    i32 MinX = 0, MinY = 0, MaxX = 0, MaxY = 0;
    FIntRect() = default;
    FIntRect(i32 X, i32 Y, i32 W, i32 H) : MinX(X), MinY(Y), MaxX(X+W), MaxY(Y+H) {}
    i32 Width() const { return MaxX - MinX; }
    i32 Height() const { return MaxY - MinY; }
    bool Contains(const FIntPoint& Pt) const { return Pt.X >= MinX && Pt.X < MaxX && Pt.Y >= MinY && Pt.Y < MaxY; }
};

struct FSlateRect
{
    f32 Left = 0, Top = 0, Right = 0, Bottom = 0;
    FSlateRect() = default;
    FSlateRect(f32 L, f32 T, f32 R, f32 B) : Left(L), Top(T), Right(R), Bottom(B) {}
    f32 Width() const { return Right - Left; }
    f32 Height() const { return Bottom - Top; }
    bool IsEmpty() const { return Right <= Left || Bottom <= Top; }
    bool Intersects(const FSlateRect& Other) const { return Left < Other.Right && Right > Other.Left && Top < Other.Bottom && Bottom > Other.Top; }
    FSlateRect Intersection(const FSlateRect& Other) const { return FSlateRect(max(Left, Other.Left), max(Top, Other.Top), min(Right, Other.Right), min(Bottom, Other.Bottom)); }
};

struct FGeometry
{
    FVector2D LocalSize;
    FVector2D AbsolutePosition;
    FVector2D AbsoluteSize;
    FSlateRect LayoutRect;
    FSlateRect CullingRect;

    FGeometry() = default;
    FGeometry(const FVector2D& InLocalSize, const FVector2D& InAbsPos, const FVector2D& InAbsSize)
        : LocalSize(InLocalSize), AbsolutePosition(InAbsPos), AbsoluteSize(InAbsSize)
        , LayoutRect(InAbsPos.X, InAbsPos.Y, InAbsPos.X + InAbsSize.X, InAbsPos.Y + InAbsSize.Y)
        , CullingRect(LayoutRect) {}

    FVector2D GetLocalTopLeft() const { return AbsolutePosition - AbsolutePosition; }
    FVector2D GetAbsoluteTopLeft() const { return AbsolutePosition; }
    FVector2D GetLocalSize() const { return LocalSize; }
    FVector2D GetAbsoluteSize() const { return AbsoluteSize; }
    FVector2D GetAbsolutePosition() const { return AbsolutePosition; }
    FSlateRect GetLayoutRect() const { return LayoutRect; }
    FSlateRect GetCullingRect() const { return CullingRect; }
    bool IsUnderLocation(const FVector2D& ScreenPos) const { return CullingRect.Contains(FIntPoint((i32)ScreenPos.X, (i32)ScreenPos.Y)); }
};

struct FPaintArgs
{
    f32 DeltaTime = 0.0f;
    bool bParentEnabled = true;
    u32 LayerId = 0;
    bool bIsEnabled = true;
    FSlateRect CullingRect;

    FPaintArgs() = default;
    FPaintArgs(f32 InDeltaTime, u32 InLayerId, const FSlateRect& InCullingRect)
        : DeltaTime(InDeltaTime), LayerId(InLayerId), CullingRect(InCullingRect) {}

    FPaintArgs WithNewParent(const FGeometry& ChildGeometry) const
    {
        FPaintArgs Args = *this;
        Args.CullingRect = Args.CullingRect.Intersection(ChildGeometry.GetCullingRect());
        Args.LayerId++;
        return Args;
    }
};

// ============================================================================
// Color/Brush
// ============================================================================

struct FLinearColor
{
    f32 R = 1.0f, G = 1.0f, B = 1.0f, A = 1.0f;

    FLinearColor() = default;
    FLinearColor(f32 R, f32 G, f32 B, f32 A = 1.0f) : R(R), G(G), B(B), A(A) {}
    FLinearColor(u32 Hex) { R = ((Hex >> 16) & 0xFF) / 255.0f; G = ((Hex >> 8) & 0xFF) / 255.0f; B = (Hex & 0xFF) / 255.0f; A = 1.0f; }

    FLinearColor WithAlpha(f32 NewA) const { return FLinearColor(R, G, B, NewA); }
    FLinearColor operator*(f32 Scalar) const { return FLinearColor(R*Scalar, G*Scalar, B*Scalar, A*Scalar); }
    FLinearColor operator+(const FLinearColor& Other) const { return FLinearColor(R+Other.R, G+Other.G, B+Other.B, A+Other.A); }

    static FLinearColor White() { return FLinearColor(1,1,1,1); }
    static FLinearColor Black() { return FLinearColor(0,0,0,1); }
    static FLinearColor Transparent() { return FLinearColor(0,0,0,0); }
    static FLinearColor Red() { return FLinearColor(1,0,0,1); }
    static FLinearColor Green() { return FLinearColor(0,1,0,1); }
    static FLinearColor Blue() { return FLinearColor(0,0,1,1); }
};

struct FSlateBrush
{
    ESlateBrushDrawType DrawType = ESlateBrushDrawType::Image;
    FLinearColor Tint = FLinearColor::White();
    FLinearColor BorderColor = FLinearColor::Transparent();
    FSlateBrushTileMode TileMode = ESlateBrushTileMode::NoTile;
    FVector2D ImageSize = FVector2D(0, 0);
    FVector2D UV0 = FVector2D(0, 0);
    FVector2D UV1 = FVector2D(1, 1);
    FMargin Margin = FMargin(0);
    RHI::Texture* Resource = nullptr;

    FSlateBrush() = default;
    FSlateBrush(RHI::Texture* InResource) : Resource(InResource) {}
    FSlateBrush(const FLinearColor& InColor) : DrawType(ESlateBrushDrawType::Box), Tint(InColor) {}

    bool IsEmpty() const { return !Resource && DrawType != ESlateBrushDrawType::Box; }
};

enum class ESlateBrushDrawType : u8
{
    NoDraw,
    Box,
    Border,
    Image,
    RoundedBox,
};

enum class ESlateBrushTileMode : u8
{
    NoTile,
    Horizontal,
    Vertical,
    Both,
};

struct FMargin
{
    f32 Left = 0, Top = 0, Right = 0, Bottom = 0;
    FMargin() = default;
    FMargin(f32 Uniform) : Left(Uniform), Top(Uniform), Right(Uniform), Bottom(Uniform) {}
    FMargin(f32 L, f32 T, f32 R, f32 B) : Left(L), Top(T), Right(R), Bottom(B) {}
    FMargin(f32 H, f32 V) : Left(H), Top(V), Right(H), Bottom(V) {}

    f32 Horizontal() const { return Left + Right; }
    f32 Vertical() const { return Top + Bottom; }
    FVector2D GetSize() const { return FVector2D(Horizontal(), Vertical()); }
};

} // namespace grom::UI