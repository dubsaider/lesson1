/*
 * Занятие 1 — генератор «сломанных» картинок для разбора на экране (только для преподавателя).
 * Каждая картинка записана save_bmp с ОДНОЙ типичной ошибкой.
 *
 *   gcc -std=c17 -Wall -Wextra make_bugs.c -o make_bugs
 *   ./make_bugs
 *
 * bug1.bmp — флаг России, перепутан порядок байтов (RGB вместо BGR)
 * bug2.bmp — флаг России, строки записаны сверху вниз (а надо снизу вверх)
 * bug3.bmp — флаг Японии 150 x 100, в строках нет байтов выравнивания
 *            (в заголовке размеры честные, а данных на 2 байта в строке меньше)
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct { unsigned char r, g, b; } Pixel;
typedef struct { int w, h; Pixel *px; } Image;

#pragma pack(push, 1)
typedef struct {
    uint16_t type; uint32_t size; uint16_t reserved1, reserved2; uint32_t offset;
} FileHeader;
typedef struct {
    uint32_t size; int32_t width, height; uint16_t planes, bpp;
    uint32_t compression, image_size; int32_t xppm, yppm;
    uint32_t colors_used, colors_important;
} InfoHeader;
#pragma pack(pop)

enum { OK, BUG_RGB, BUG_TOP_DOWN, BUG_NO_PADDING };

static void save_bmp_buggy(const char *name, const Image *img, int bug) {
    int stride = (img->w * 3 + 3) / 4 * 4;
    int row_bytes = (bug == BUG_NO_PADDING) ? img->w * 3 : stride;
    FileHeader fh = { 0x4D42, 54 + stride * img->h, 0, 0, 54 };
    InfoHeader ih = { 40, img->w, img->h, 1, 24, 0, stride * img->h, 2835, 2835, 0, 0 };
    FILE *f = fopen(name, "wb");
    if (f == NULL) return;
    fwrite(&fh, sizeof fh, 1, f);
    fwrite(&ih, sizeof ih, 1, f);
    unsigned char *row = calloc(stride, 1);
    for (int i = 0; i < img->h; i++) {
        int y = (bug == BUG_TOP_DOWN) ? i : img->h - 1 - i;
        for (int x = 0; x < img->w; x++) {
            Pixel p = img->px[y * img->w + x];
            if (bug == BUG_RGB) { row[x*3] = p.r; row[x*3+1] = p.g; row[x*3+2] = p.b; }
            else                { row[x*3] = p.b; row[x*3+1] = p.g; row[x*3+2] = p.r; }
        }
        fwrite(row, 1, row_bytes, f);
    }
    /* при BUG_NO_PADDING файл короче, чем обещает заголовок — допишем нули,
       чтобы просмотрщик открыл картинку, а не отказался */
    if (bug == BUG_NO_PADDING)
        for (int i = 0; i < (stride - row_bytes) * img->h; i++) fputc(0, f);
    free(row);
    fclose(f);
}

static Image flag_ru(void) {
    Image img = { 101, 60, malloc(101 * 60 * sizeof(Pixel)) };
    for (int y = 0; y < 60; y++)
        for (int x = 0; x < 101; x++)
            img.px[y * 101 + x] = y < 20 ? (Pixel){255, 255, 255}
                                : y < 40 ? (Pixel){0, 57, 166} : (Pixel){213, 43, 30};
    return img;
}

static Image flag_jp(void) {
    Image img = { 150, 100, malloc(150 * 100 * sizeof(Pixel)) };
    for (int y = 0; y < 100; y++)
        for (int x = 0; x < 150; x++)
            img.px[y * 150 + x] = ((x-75)*(x-75) + (y-50)*(y-50) <= 900)
                                ? (Pixel){188, 0, 45} : (Pixel){255, 255, 255};
    return img;
}

int main(void) {
    Image ru = flag_ru(), jp = flag_jp();
    save_bmp_buggy("bug1.bmp", &ru, BUG_RGB);
    save_bmp_buggy("bug2.bmp", &ru, BUG_TOP_DOWN);
    save_bmp_buggy("bug3.bmp", &jp, BUG_NO_PADDING);
    free(ru.px);
    free(jp.px);
    printf("done\n");
    return 0;
}
