#include "sh_color.h"
#include "sh_utils.h"
#include <math.h>
#include "khash.h"

// Background Colours, Foreground Colours
uint32_t color_map[] = {
        0x002b36, //  0 | 40
        0xcb4b16, //  1 | 41
        0x586e75, //  2 | 42
        0x657b83, //  3 | 43
        0x839496, //  4 | 44
        0x6c71c4, //  5 | 45
        0x93a1a1, //  6 | 46
        0xfdf6e3, //  7 | 47
        0x4f545c, //  8 | 31
        0xdc322f, //  9 | 32
        0x859900, // 10 | 33
        0xb58900, // 11 | 34
        0x268bd2, // 12 | 35
        0xd33682, // 13 | 36
        0x2aa198, // 14 | 37
        0xffffff  // 15 | 38
};

color_yuv_t yuv_color_map[N_COLORS]; ///< palette of the terminal colors in YUV format


void rgb2yuv(const color_t *rgb, color_yuv_t *yuv)
{
        float r = rgb->r/255.f, g = rgb->g/255.f, b = rgb->b/255.f;
        yuv->y = .299f*r + .587f*g + .114f*b;
        yuv->u = -.14173f*r + -.28886f*g + .436f*b;
        yuv->v = .615f*r + -.51499f*g + -.10001f*b;
}

float col_yuv_distance(const color_yuv_t *a, const color_yuv_t *b)
{
        return SQUARED(b->y - a->y) + SQUARED(b->u - a->u) + SQUARED(b->v - a->v);
}

// convert color to the terminal 256 colors
// if it have been already converted, fond it in the map
// First convert to XYZ, then to *L*a*b
// finnally find the closest color in the map and
// store it
KHASH_MAP_INIT_INT(uint32_t, uint32_t)
khash_t(uint32_t) *hash_colors;
void init_hash_colors()
{
        hash_colors = kh_init(uint32_t);
        int ret;
        khiter_t k;
        color_t rgb;
        for (int i = 0; i < N_COLORS; i++) {
                k = kh_put(uint32_t, hash_colors, color_map[i], &ret);
                /*if (!ret) kh_del(uint32_t, hash_colors, k);*/
                kh_value(hash_colors, k) = color_map[i];
                rgb.r = X2R(color_map[i]);
                rgb.g = X2G(color_map[i]);
                rgb.b = X2B(color_map[i]);
                rgb2yuv(&rgb, &yuv_color_map[i]);
        }
}

void free_hash_colors()
{
        kh_destroy(uint32_t, hash_colors);
}

uint32_t find_nearest_color(color_t *col)
{
        color_yuv_t yuv;
        rgb2yuv(col, &yuv);
        uint32_t b = 0;
        float dist = INFINITY, tmp;
        for (uint32_t i = 1; i < N_COLORS; ++i) {
                tmp = col_yuv_distance(&yuv, &yuv_color_map[i]);
                if (tmp < dist) {
                        dist = tmp;
                        b = i;
                }
        }
        return b;
}

void convert_color(color_t  *col, color_t *out)
{
        if (col->a == 0)
                return; // we don't care about this color
        /*int32_t r = col->r, g = col->g, b = col->b;*/
        /*double x,y,z;*/
        /*x = 0.412453*r + 0.357580*g + 0.180423*b;*/
        /*y = 0.212671*r + 0.715160*g + 0.072169*b;*/
        /*z = 0.019334*r + 0.119193*g + 0.950227*b;*/

        khiter_t k;
        k = kh_get(uint32_t, hash_colors, RGB2X(col->r, col->g, col->b));

        uint32_t in = find_nearest_color(col);

        out->r = (in < 8) + 3;
        out->g = in & 0b111;
        out->b = in;

        // printf("%u%u %u\n", out->r, out->g, out->b);
        // if (k == kh_end(hash_colors)) { // it doesn't exist
        //         uint32_t in = find_nearest_color(col);

        //         int ret;
        //         khiter_t k;
        //         k = kh_put(uint32_t, hash_colors, RGB2X(col->r, col->g, col->b), &ret);
        //         /*if (!ret) kh_del(uint32_t, hash_colors, k);*/
        //         kh_value(hash_colors, k) = color_map[in];
        //         out->r = X2R(color_map[in]);
        //         out->g = X2G(color_map[in]);
        //         out->b = X2B(color_map[in]);
        // } else {
        //         uint32_t xcol = kh_value(hash_colors, k);
        //         out->r = X2R(xcol);
        //         out->g = X2G(xcol);
        //         out->b = X2B(xcol);
        // }
}

void col_cpy(const color_t *col, color_t *out)
{
        out->r = col->r;
        out->g = col->g;
        out->b = col->b;
        out->a = col->a;
}
