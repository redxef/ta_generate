/**
 * @author      Manuel Federanko
 * @file        ta_generate.c
 * @version     0.0.0-r0
 * @since
 *
 * @brief       A brief documentation about the file.
 *
 * A detailed documentation.
 */

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <png.h>

#include "ta_generate.h"


uint32_t _bpp_to_color_type(uint32_t bpp) {
        switch(bpp) {
                default:
                case 3:
                        return PNG_COLOR_TYPE_RGB;
                case 4:
                        return PNG_COLOR_TYPE_RGBA;
        }
}

uint32_t _color_type_to_bpp(uint32_t ct) {
        switch (ct) {
                default:
                case PNG_COLOR_TYPE_RGB:
                        return 3;
                case PNG_COLOR_TYPE_RGBA:
                        return 4;
        }

}

uint32_t tag_canvas__new(struct tag_canvas *c, uint32_t w, uint32_t h,
                        uint32_t bpp) {

        c->width = w;
        c->height = h;
        c->bpp = bpp;
        c->imgdata = calloc(w * h * bpp, sizeof(uint8_t));
        if (c->imgdata == NULL)
                return 1;
        return 0;
}

uint32_t tag_canvas__delete(struct tag_canvas *c) {
        free(c->imgdata);
        return 0;
}

uint32_t tag_canvas__print(struct tag_canvas *c) {

        uint32_t i;

        printf("<\n");
        printf("width=%d\n", c->width);
        printf("height=%d\n", c->height);
        printf("bpp=%d\n", c->bpp);
        printf("\n");

        for (i = 0; i < c->width * c->height * c->bpp; i++) {
                if (i != 0 && i % (c->width * c->bpp) == 0)
                        printf("\n");
                else if (i != 0 && i % c->bpp == 0)
                        printf(" ");
                printf("%02x", c->imgdata[i]);
        }
        printf("\n>\n");
        return 0;
}

uint32_t tag_canvas__get_idx(struct tag_canvas *c, uint32_t *idx,
                            uint32_t x, uint32_t y) {

        uint32_t res = 0;
        res += c->width * y + x;        // skip y lines, the add offs
        res *= c->bpp;                  // scale by bytes per pixel
        *idx = res;
        return 0;
}

uint32_t tag_canvas__writepx(struct tag_canvas *c, uint32_t x, uint32_t y,
                            uint32_t val) {

        uint32_t idx = 0;
        uint32_t i;
        tag_canvas__get_idx(c, &idx, x, y);
        for (i = 0; i < c->bpp; i++)
                c->imgdata[idx+i] = val >> ((c->bpp-i-1)<<3);
        return 0;
}

uint32_t tag_canvas__readpx(struct tag_canvas *c, uint32_t *val,
                           uint32_t x, uint32_t y) {

        uint32_t idx = 0;
        uint32_t i;
        uint32_t res = 0;
        tag_canvas__get_idx(c, &idx, x, y);
        for (i = 0; i < c->bpp; i++) {
                res <<= 8;
                res |= c->imgdata[idx+i];
        }
        *val = res;
        return 0;
}

uint32_t tag_canvas__write_canvas(struct tag_canvas *c,
                                 struct tag_canvas *add, uint32_t x,
                                 uint32_t y) {

        uint32_t i, j;
        uint32_t pix;
        if (add->width + x > c->width)
                return 1;
        if (add->height + y > c->height)
                return 1;
        if (add->bpp != c->bpp)
                return 1;

        for (i = 0; i < add->width; i++) {
                for (j = 0; j < add->height; j++) {
                        tag_canvas__readpx(add, &pix, i, j);
                        tag_canvas__writepx(c, i+x, j+y, pix);
                }
        }


        return 0;
}

uint32_t tag_canvas__load_png(struct tag_canvas *c, const char *file) {
        FILE *fp;
        uint32_t i, j, k, val;
        unsigned char header[8];

        png_structp png_ptr;
        png_infop info_ptr;
        png_bytep *row_ptrs;

        /* setup libpng for reading the file */
        fp = fopen(file, "rb");
        if (fp == NULL)
                return TA_LOAD_PNG_EFOPEN;

        fread(header, 1, 8, fp);
        if (png_sig_cmp(header, 0, 8) != 0)
                return TA_LOAD_PNG_EHEADR;

        png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL,
                                         NULL, NULL);
        if (png_ptr == NULL) {
                fclose(fp);
                return TA_LOAD_PNG_EREADS;
        }

        info_ptr = png_create_info_struct(png_ptr);
        if (info_ptr == NULL) {
                fclose(fp);
                return TA_LOAD_PNG_EINFOS;
        }

        png_init_io(png_ptr, fp);
        png_set_sig_bytes(png_ptr, 8);
        png_read_info(png_ptr, info_ptr);

        /* initialize the canvas */
        c->width = png_get_image_width(png_ptr, info_ptr);
        c->height = png_get_image_height(png_ptr, info_ptr);
        c->bpp = png_get_channels(png_ptr, info_ptr);
        tag_canvas__new(c, c->width, c->height, c->bpp);

        /* load the image data into libpng buffer */
        row_ptrs = malloc(sizeof(png_bytep) * c->height);
        for (i = 0; i < c->height; i++)
                row_ptrs[i] = malloc(png_get_rowbytes(png_ptr, info_ptr));
        png_read_image(png_ptr, row_ptrs);

        /* write the image data into canvas buffer */
        for (i = 0; i < c->height; i++) {
                for (j = 0; j < c->width * c->bpp; j += c->bpp) {
                        val = 0;
                        for (k = 0; k < c->bpp; k++) {
                                val <<= 8;
                                val |= row_ptrs[i][j+k];
                        }
                        tag_canvas__writepx(c, j/c->bpp, i, val);
                }
        }

        /* free up memory */
        fclose(fp);
        for (i = 0; i < c->height; i++)
                free(row_ptrs[i]);
        free(row_ptrs);
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

        return 0;
}

uint32_t tag_canvas__write_png(struct tag_canvas *c, const char *file) {
        uint32_t i;
        FILE *fp;
        png_structp png_ptr;
        png_infop info_ptr;
        png_bytep *row_ptrs;

        fp = fopen(file, "wb");
        if (fp == NULL) {
                return TA_WRITE_PNG_EFOPEN;
        }

        png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
                        NULL, NULL, NULL);
        if (png_ptr == NULL) {
                fclose(fp);
                return TA_WRITE_PNG_EWRITS;
        }

        info_ptr = png_create_info_struct(png_ptr);
        if (info_ptr == NULL) {
                fclose(fp);
                return TA_WRITE_PNG_EINFOS;
        }

        png_init_io(png_ptr, fp);
        png_set_IHDR(
                png_ptr, info_ptr,
                c->width, c->height,
                8,
                _bpp_to_color_type(c->bpp),
                PNG_INTERLACE_NONE,
                PNG_COMPRESSION_TYPE_DEFAULT,
                PNG_FILTER_TYPE_DEFAULT
        );


        row_ptrs = malloc(sizeof(png_bytep) * c->height);
        for (i = 0; i < c->height; i++) {
                row_ptrs[i] = &c->imgdata[i * c->width * c->bpp];
        }

        png_write_info(png_ptr, info_ptr);
        png_write_image(png_ptr, row_ptrs);
        png_write_end(png_ptr, NULL);

        fclose(fp);
        free(row_ptrs);
        png_destroy_write_struct(&png_ptr, &info_ptr);

        return 0;
}

uint32_t tag_generator_node__new(struct tag_generator_node *g,
                                uint32_t x, uint32_t y,
                                uint32_t w, uint32_t h) {

        g->child[0] = NULL;
        g->child[1] = NULL;
        g->x = x;
        g->y = y;
        g->w = w;
        g->h = h;
        g->img = NULL;
        return 0;
}

uint32_t tag_generator_node__delete(
                struct tag_generator_node *g, uint32_t free_self) {
        if (g->img != NULL) {
                tag_canvas__delete(g->img);
                free(g->img);
                g->img = NULL;
        }
        if (g->child[0] != NULL)
                tag_generator_node__delete(g->child[0], 1);
                g->child[0] = NULL;
        if (g->child[0] != NULL)
                tag_generator_node__delete(g->child[1], 1);
                g->child[1] = NULL;
        if (free_self)
                free(g);
        return 0;
}

uint32_t tag_generator_node__print(struct tag_generator_node *g,
                                  uint32_t indent) {

        if (g == NULL)
                return 0;
        uint32_t i = indent;
        while (i--)
                printf("  ");
        printf("%p\n", (void *) g->img);
        tag_generator_node__print(g->child[0], indent+2);
        tag_generator_node__print(g->child[1], indent+2);
        return 0;
}

uint32_t tag_generator_node__insert_canvas(struct tag_generator_node *g,
                                          struct tag_canvas *c,
                                          uint32_t *x, uint32_t *y) {
        int64_t dw, dh, len;
        struct tag_canvas *c0;


        if (g->child[0] != NULL || g->child[1] != NULL) {
                if (tag_generator_node__insert_canvas(
                                g->child[0], c, x, y) == 0)
                        return 0;
                if (tag_generator_node__insert_canvas(
                                g->child[1], c, x, y) == 0)
                        return 0;
                return 1;
        }

        /* already image here */
        if (g->img != NULL)
                return 1;

        /* img doesnt fit */
        if (c->width > g->w || c->height > g->h)
                return 1;

        len = c->width * c->height * c->bpp;
        c0 = malloc(sizeof(struct tag_canvas));
        memcpy(c0, c, sizeof(struct tag_canvas));
        c0->imgdata = malloc(sizeof(uint8_t) * len);
        memcpy(c0->imgdata, c->imgdata, len);


        /* if we fit, we are happy */
        if (c->width == g->w && c->height == g->h) {
                *x = g->x;
                *y = g->y;
                g->img = c0;
                return 0;
        }

        /* split node */
        dw = g->w - c->width;
        dh = g->h - c->height;

        if (dw < 0 || dh < 0)
                return 1;

        g->img = c0;
        *x = g->x;
        *y = g->y;
        g->child[0] = malloc(sizeof(struct tag_generator_node));
        g->child[1] = malloc(sizeof(struct tag_generator_node));
        tag_generator_node__new(g->child[0], 0, 0, 0, 0);
        tag_generator_node__new(g->child[1], 0, 0, 0, 0);

        if (dw < dh) {
                g->child[0]->x = g->x + c->width;
                g->child[0]->y = g->y;
                g->child[0]->w = g->w - c->width;
                g->child[0]->h = c->height;

                g->child[1]->x = g->x;
                g->child[1]->y = g->y + c->height;
                g->child[1]->w = g->w;
                g->child[1]->h = g->h - c->height;
        } else {
                g->child[0]->x = g->x;
                g->child[0]->y = g->y + c->height;
                g->child[0]->w = c->width;
                g->child[0]->h = g->h - c->height;

                g->child[1]->x = g->x + c->width;
                g->child[1]->y = g->y;
                g->child[1]->w = g->w - c->width;
                g->child[1]->h = g->h;
        }
        return 0;
}

uint32_t tag_generator_node__to_canvas_recursive(
                struct tag_generator_node *g, struct tag_canvas *c) {
        if (g == NULL)
                return 0;
        if (g->img == NULL)
                return 0;
        tag_canvas__write_canvas(c, g->img, g->x, g->y);
        tag_generator_node__to_canvas_recursive(g->child[0], c);
        tag_generator_node__to_canvas_recursive(g->child[1], c);
        return 0;
}

uint32_t tag_generator_node__to_canvas(struct tag_generator_node *g,
                                      struct tag_canvas *c) {

        tag_canvas__new(c, g->w, g->h, g->img->bpp);
        return tag_generator_node__to_canvas_recursive(g, c);
}






