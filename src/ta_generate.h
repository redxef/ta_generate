
/**
 * @author      Manuel Federanko
 * @file        ta_generate.h
 * @version     0.0.0-r0
 * @since
 *
 * @brief       A brief documentation about the file.
 *
 * A detailed documentation.
 */

#include <stdint.h>

#define TA_CANV_NEW_EMEM        0x100u

#define TA_LOAD_PNG_EFOPEN      0x200u
#define TA_LOAD_PNG_EHEADR      0x201u
#define TA_LOAD_PNG_EREADS      0x202u
#define TA_LOAD_PNG_EINFOS      0x203u

#define TA_WRITE_PNG_EFOPEN     0x210u
#define TA_WRITE_PNG_EWRITS     0x212u
#define TA_WRITE_PNG_EINFOS     0x213u

#ifndef __TA_GENERATE_H_
#define __TA_GENERATE_H_

struct tag_canvas {
        uint32_t width;
        uint32_t height;
        uint32_t bpp;
        uint8_t *imgdata;
};

struct tag_generator_node {
        struct tag_generator_node *child[2];
        uint32_t x, y;
        uint32_t w, h;
        struct tag_canvas *img;
};

uint32_t tag_canvas__new(struct tag_canvas *c, uint32_t w, uint32_t h,
                        uint32_t bpp);


uint32_t tag_canvas__print(struct tag_canvas *c);

uint32_t tag_canvas__read_idx(struct tag_canvas *c, uint32_t *idx,
                            uint32_t x, uint32_t y);

uint32_t tag_canvas__writepx(struct tag_canvas *c, uint32_t x, uint32_t y,
                            uint32_t val);

uint32_t tag_canvas__readpx(struct tag_canvas *c, uint32_t *val,
                          uint32_t x, uint32_t y);

uint32_t tag_canvas__write_canvas(struct tag_canvas *c,
                                struct tag_canvas *add, uint32_t x,
                                uint32_t y);

uint32_t tag_canvas__load_png(struct tag_canvas *c, const char *file);

uint32_t tag_generator_node__new(struct tag_generator_node *g,
                                uint32_t x, uint32_t y,
                                uint32_t w, uint32_t h);

uint32_t tag_generator_node__print(struct tag_generator_node *g,
                                  uint32_t indent);

uint32_t tag_generator_node__insert_canvas(struct tag_generator_node *g,
                                     struct tag_canvas *c,
                                     uint32_t *x, uint32_t *y);

uint32_t tag_generator_node__to_canvas(struct tag_generator_node *g,
                                      struct tag_canvas *c);

#endif
