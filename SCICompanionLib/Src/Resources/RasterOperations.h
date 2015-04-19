#pragma once
#include "Components.h"
#include "View.h"

enum class RasterChangeHint
{
    None = 0x0000,
    CelSelection = 0x0001,
    LoopSelection = 0x0002,
    Cel = 0x0004,
    Loop = 0x0008,
    NewView = 0x0010,
    Color = 0x0020,
    PenWidth = 0x0040,
    SampleText = 0x0080,
    ApplyToAll = 0x0100,
    Palette = 0x0200,
};

DEFINE_ENUM_FLAGS(RasterChangeHint, uint32_t)

struct RasterChange
{
    RasterChange() {}
    RasterChange(RasterChangeHint hint, CelIndex index) : hint(hint), index(index) {}
    RasterChange(RasterChangeHint hint) : hint(hint) {}
    RasterChange(const RasterChange &src) = default;
    RasterChange& operator=(const RasterChange &src) = default;

    RasterChangeHint hint;
    CelIndex index;
};

enum class RasterResizeFlags
{
    Normal,
    AnchorBottomRight,
    Stretch,
};

class ResourceEntity;
struct SCIBitmapInfoEx;
struct RasterComponent;
struct PaletteComponent;
struct Cel;
struct Loop;

enum class BitmapScaleOptions
{
    None = 0,
    AllowMag = 0x1,
    AllowMin = 0x2,
};
DEFINE_ENUM_FLAGS(BitmapScaleOptions, uint8_t)

HBITMAP GetBitmap(RasterComponent &raster, const PaletteComponent *palette, CelIndex celIndex, int cx, int cy, BitmapScaleOptions scaleOptions);
HBITMAP GetBitmap(Cel &cel, const PaletteComponent *palette, int cx, int cy, BitmapScaleOptions scaleOptions, uint8_t bgFillColor);
void CopyBitmapData(const RasterComponent &raster, CelIndex celIndex, uint8_t *pData, size16 size);
RasterChange SetPlacement(RasterComponent &raster, CelIndex celIndex, int16_t x, int16_t y);
RasterChange SetGroupPlacement(RasterComponent &raster, int cItems, CelIndex *rgdwIndex, int16_t x, int16_t y);
RasterChange SetTransparentColor(RasterComponent &raster, CelIndex celIndex, uint8_t color);
RasterChange SetGroupTransparentColor(RasterComponent &raster, int cCels, CelIndex *rgdwIndex, uint8_t color);
RasterChange SetSize(RasterComponent &raster, CelIndex celIndex, size16 size, RasterResizeFlags resizeFlags);
RasterChange SetGroupSize(RasterComponent &raster, int cCels, CelIndex *rgdwIndex, const size16 *rgSizes, RasterResizeFlags resizeFlags);
RasterChange FillEmpty(RasterComponent &raster, CelIndex celIndex, size16 size);
bool ClampSize(const RasterComponent &raster, size16 &size);    // True if we clamped
void CreateDegenerate(Cel &cel, uint8_t bColor);
RasterChange MirrorLoopFrom(Loop &loop, uint8_t nOriginal, const Loop &orig);
void UpdateMirrors(RasterComponent &raster, int nLoop);
void UpdateMirrors(RasterComponent &raster, CelIndex celIndex);
RasterChange SetBitmapData(RasterComponent &raster, CelIndex celIndex, const uint8_t *data);
RasterChange SetGroupBitmapData(RasterComponent &raster, int cCels, CelIndex *rgdwIndex, const uint8_t* const* ppData);
RasterChange MoveLoopFromTo(RasterComponent &raster, int nLoop, int celFrom, int celTo);
HBITMAP CreateBitmapFromResource(const ResourceEntity &resource, const PaletteComponent *palette, SCIBitmapInfo *pbmi, uint8_t **ppBitsDest);
RasterChange InsertLoop(RasterComponent &raster, int nLoop, bool before);
RasterChange RemoveLoop(RasterComponent &raster, int nLoop);
RasterChange InsertCel(RasterComponent &raster, CelIndex celIndex, bool before, bool dupeNewCels);
RasterChange InsertCel(RasterComponent &raster, CelIndex celIndex, const Cel &cel);   // Let's us insert to a cel-less loop
RasterChange RemoveCel(RasterComponent &raster, CelIndex celIndex);
RasterChange MakeMirrorOf(RasterComponent &raster, int nLoop, int nOriginal);
void SyncCelMirrorState(Cel &celMirror, const Cel &celOrig);

// These don't adhere to any persisted format. Use only at runtime.
void SerializeCelRuntime(sci::ostream &out, const Cel &cel);
void DeserializeCelRuntime(sci::istream &in, Cel &cel);