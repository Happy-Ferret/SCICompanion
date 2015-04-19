#pragma once

// Don't change the numbers here without updating CGameVersionDialog
// They correspond to radio button groups.
enum class ResourceMapFormat : uint8_t
{
    SCI0 = 0,
    SCI1 = 1,
    SCI11 = 2,
};

enum class ResourcePackageFormat : uint8_t
{
    SCI0 = 0,
    SCI1 = 1,
    SCI11 = 2,
};

enum class SoundFormat : uint8_t
{
    SCI0 = 0,
    SCI1 = 1,
};

enum class CompressionFormat : uint8_t
{
    SCI0 = 0,
    SCI1 = 1,
};

enum class ViewFormat : uint8_t
{
    EGA = 0,
    VGA1 = 1,
    VGA1_1 = 2,
};

enum class PicFormat : uint8_t
{
    EGA = 0,
    VGA1 = 1,
    VGA1_1 = 2,
};

struct SCIVersion
{
    ResourceMapFormat MapFormat;
    ResourcePackageFormat PackageFormat;
    SoundFormat SoundFormat;
    CompressionFormat CompressionFormat;
    bool HasPalette;
    ViewFormat ViewFormat;
    PicFormat PicFormat;
    bool GrayScaleCursors;
    bool lofsaOpcodeIsAbsolute;
    bool SeparateHeapResources;
    uint16_t MainVocabResource;
    int DefaultVolumeFile;          // 0 or 1
    bool HasOldSCI0ScriptHeader;
    bool sci11Palettes;

    bool operator==(const SCIVersion &src);
    bool operator!=(const SCIVersion &src);
};

// Detection possibilities:
// - resource map is easy. Once figured out, then test for palette? If so, then VGA
extern SCIVersion sciVersion0;
extern SCIVersion sciVersion1_Late;
extern SCIVersion sciVersion1_1;