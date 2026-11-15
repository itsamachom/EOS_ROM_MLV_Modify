/* Extracted from credited upstream fos_film.c/fos_gfx.c; GPL-2.0-only.
 * See upstream/PROVENANCE.json. No upstream menu/font engine is loaded. */
#include "upstream/fos.h"
#include "upstream/fos_dev.h"
static const struct fos_film builtin[] =
{
    { "portra400", "KODAK", "Portra 400", "C-41", "Soft warm skin, gentle highlights",
      400, 400, PS_STANDARD, -1, -1, 1, 2, 5500, 0, 0,     2, 2, 2, 4, "Portra400", FOS_GROUP_COLOUR, 0 },
    { "cinestill800t", "CINESTILL", "800T", "C-41", "Tungsten cine film, night goes blue",
      800, 800, PS_STANDARD, 1, 1, -1, 2, 3200, 0, 0,      4, 3, 3, 1, "CineStill800T", FOS_GROUP_CINE, 0 },
    { "ektar100", "KODAK", "Ektar 100", "C-41", "Punchy colour, ultra fine grain",
      100, 100, PS_LANDSCAPE, 1, 2, 0, 3, 5500, 0, 0,      1, 4, 5, 3, "Ektar100", FOS_GROUP_COLOUR, 0 },
    { "gold200", "KODAK", "Gold 200", "C-41", "Golden hour, all day long",
      200, 200, PS_STANDARD, 1, 1, 2, 2, 5500, 0, 0,       3, 3, 4, 5, "Gold200", FOS_GROUP_COLOUR, 0 },
    { "superia400", "FUJIFILM", "Superia 400", "C-41", "Green shadows, everyday snaps",
      400, 400, PS_STANDARD, 1, 1, 0, 2, 5500, 0, 0,       3, 3, 3, 2, "Superia400", FOS_GROUP_COLOUR, 0 },
    { "velvia50", "FUJIFILM", "Velvia 50", "E-6", "Vivid slide film, hard contrast",
      50, 100, PS_LANDSCAPE, 3, 4, 0, 3, 5500, 0, 0,       1, 5, 5, 3, "Velvia50", FOS_GROUP_SLIDE, 0 },
    { "hp5", "ILFORD", "HP5 Plus", "B&W", "Classic B&W, forgiving latitude",
      400, 400, PS_MONO, 1, 0, 0, 2, 5500, 0, 0,           4, 2, 0, 0, "HP5", FOS_GROUP_BW, 0 },
    { "trix400", "KODAK", "Tri-X 400", "B&W", "Gritty contrast, street classic",
      400, 400, PS_MONO, 3, 0, 0, 2, 5500, 0, 0,           5, 4, 0, 0, "TriX400", FOS_GROUP_BW, 0 },
};

const struct fos_film *fos_films=builtin;
int fos_nfilms=8;
const struct fos_look *fos_more_looks;
int fos_nmore_looks;
int fos_isqrt(unsigned int v)
{
    if (!v) return 0;
    unsigned int r = 0, bit = 1u << ((31 - __builtin_clz(v)) & ~1);
    while (bit)
    {
        if (v >= r + bit) { v -= r + bit; r = (r >> 1) + bit; }
        else r >>= 1;
        bit >>= 2;
    }
    return r;
}
const uint8_t fos_fixed_rgb[FOS_FIXED_N][3] =
{
    { 0, 0, 0 }, { 22, 21, 20 }, { 52, 50, 48 }, { 88, 85, 82 }, { 128, 124, 120 }, { 170, 166, 160 },
    { 210, 206, 200 }, { 244, 241, 236 }, { 236, 168, 52 }, { 150, 104, 34 }, { 214, 64, 52 }, { 60, 58, 56 },
    { 112, 108, 104 }, { 0, 0, 0 },
};
