/**
 *
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "ta_generate.h"


int main(int argc, char **argv) {
        struct tag_generator_node g;
        struct tag_canvas c[4];
        uint32_t val, val0;
        uint32_t x, y;
        tag_generator_node__new(&g, 0, 0, 4, 4);

        tag_canvas__new(&c[0], 1, 1, 3);

        for (int i = 0; i < 16; i++) {
                val0 = 0;
                val0 |= (255 * ((val>>2) & 1)) << 16;
                val0 |= (255 * ((val>>1) & 1)) << 8;
                val0 |= (255 * ((val>>0) & 1)) << 0;

                tag_canvas__writepx(&c[0], 0, 0, val0);
                tag_generator_node__insert_canvas(&g, &c[0], &x, &y);
                printf("x=%d, y=%d\n", x, y);

                val++;
                if (val == 7)
                        val = 0;
                tag_generator_node__to_canvas(&g, &c[3]);
                tag_canvas__write_png(&c[3], "/Users/redxef/Desktop/atest.png");
                sleep(4);
        }


        return 0;
}
