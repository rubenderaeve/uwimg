#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "image.h"

float limit_between_min_max(float v, float min, float max) {
    return (v < min) ? min : (v > max ? max : v);
}

int get_index(image im, int c, int h, int w) {
    // use the clamp padding strategy
    int c_clamp = limit_between_min_max(c, 0, im.c - 1); // - 1: start from 0
    int h_clamp = limit_between_min_max(h, 0, im.h - 1);
    int w_clamp = limit_between_min_max(w, 0, im.w - 1);

    // image is stored in CHW format
    return c_clamp * (im.h * im.w) + h_clamp * im.w + w_clamp;
}

float get_pixel(image im, int c, int h, int w) {
    return im.data[get_index(im, c, h, w)];
}

void set_pixel(image im, int c, int h, int w, float v) {
    im.data[get_index(im, c, h, w)] = v;
}

image copy_image(image im) {
    image copy = make_image(im.c, im.h, im.w);

    for (int i = 0; i < im.c; ++i) {
        for (int j = 0; j < im.h; ++j) {
            for (int k = 0; k < im.w; ++k) {
                set_pixel(copy, i, j, k, get_pixel(im, i, j, k));
            }
        }
    }

    return copy;
}

float gray_approx(image im, int h, int w) {
    float r = get_pixel(im, 0, h, w);
    float g = get_pixel(im, 1, h, w);
    float b = get_pixel(im, 2, h, w);

    return 0.299 * r + 0.587 * g + 0.114 * b;
}

image rgb_to_grayscale(image im) {
    assert(im.c == 3);
    if (im.c != 3) {
        return im;
    }

    image gray = make_image(1, im.h, im.w);

    for (int j = 0; j < im.h; ++j) {
        for (int k = 0; k < im.w; ++k) {
            set_pixel(gray, 0, j, k, gray_approx(im, j, k));
        }
    }

    return gray;
}

void shift_image(image im, int c, float v) {
    for (int j = 0; j < im.h; ++j) {
        for (int k = 0; k < im.w; ++k) {
            set_pixel(im, c, j, k, get_pixel(im, c, j, k) + v);
        }
    }
}

void clamp_image(image im) {
    for (int i = 0; i < im.c; ++i) {
        for (int j = 0; j < im.h; ++j) {
            for (int k = 0; k < im.w; ++k) {
                set_pixel(im, i, j, k, limit_between_min_max(get_pixel(im, i, j, k), 0, 1));
            }
        }
    }
}

// These might be handy
float three_way_max(float a, float b, float c) {
    return (a > b) ? ((a > c) ? a : c) : ((b > c) ? b : c);
}

float three_way_min(float a, float b, float c) {
    return (a < b) ? ((a < c) ? a : c) : ((b < c) ? b : c);
}

void rgb_to_hsv(image im) {
    assert(im.c == 3);
    if (im.c != 3) {
        return;
    }

    for (int j = 0; j < im.h; ++j) {
        for (int k = 0; k < im.w; ++k) {
            float r = get_pixel(im, 0, j, k);
            float g = get_pixel(im, 1, j, k);
            float b = get_pixel(im, 2, j, k);

            // Value
            float v = three_way_max(r, g, b);

            // Saturation
            float m = three_way_min(r, g, b);
            float c = v - m;
            float s = v == 0 ? 0 : c / v;

            // Hue
            float h_accent = c == 0 ? 0 : (v == r ? (g - b) / c : (v == g ? (b - r) / c + 2 : (r - g) / c + 4));
            float h = h_accent < 0 ? h_accent / 6 + 1 : h_accent / 6;

            // replace rgb with hsv
            set_pixel(im, 0, j, k, h);
            set_pixel(im, 1, j, k, s);
            set_pixel(im, 2, j, k, v);
        }
    }
}

void hsv_to_rgb(image im) {
    assert(im.c == 3);
    if (im.c != 3) {
        return;
    }

    for (int j = 0; j < im.h; ++j) {
        for (int k = 0; k < im.w; ++k) {
            float h = get_pixel(im, 0, j, k);
            float s = get_pixel(im, 1, j, k);
            float v = get_pixel(im, 2, j, k);


            // formulas obtained from: https://stackoverflow.com/questions/51203917/math-behind-hsv-to-rgb-conversion-of-colors
            int i = floor(h * 6);
            float f = h * 6 - i;
            float p = v * (1 - s);
            float q = v * (1 - f * s);
            float t = v * (1 - (1 - f) * s);

            float r = 0;
            float g = 0;
            float b = 0;

            switch (i % 6) {
                case 0:
                    r = v, g = t, b = p;
                    break;
                case 1:
                    r = q, g = v, b = p;
                    break;
                case 2:
                    r = p, g = v, b = t;
                    break;
                case 3:
                    r = p, g = q, b = v;
                    break;
                case 4:
                    r = t, g = p, b = v;
                    break;
                case 5:
                    r = v, g = p, b = q;
                    break;
            }

            // replace rgb with hsv
            set_pixel(im, 0, j, k, r);
            set_pixel(im, 1, j, k, g);
            set_pixel(im, 2, j, k, b);
        }
    }
}
