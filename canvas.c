/*
 * Занятие 1 «Рисуем кодом» — заготовка.
 *
 * Сборка и запуск:
 *   gcc -std=c17 -Wall -Wextra canvas.c -o canvas -lm
 *   .\canvas.exe          (Linux/macOS: ./canvas)
 *
 * Картинка в памяти — массив пикселей, строка 0 — ВЕРХНЯЯ.
 * Пиксель (x, y) лежит в img.px[y * img.w + x].
 * Рисовать можно, ничего не зная о формате файла: set_pixel, fill, fill_rect.
 * Всё про формат BMP живёт в одной функции — save_bmp. Её вы и допишете.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

typedef struct { unsigned char r, g, b; } Pixel;
typedef struct { int w, h; Pixel *px; } Image;

Image img_new(int w, int h) {
    Image img = { w, h, calloc((size_t)w * h, sizeof(Pixel)) };  /* calloc: всё чёрное */
    return img;
}

void set_pixel(Image *img, int x, int y, Pixel c) {
    if (x < 0 || y < 0 || x >= img->w || y >= img->h) return;     /* за краем — пропустить */
    img->px[y * img->w + x] = c;
}

void fill(Image *img, Pixel c) {
    for (int y = 0; y < img->h; y++)
        for (int x = 0; x < img->w; x++)
            set_pixel(img, x, y, c);
}

void fill_rect(Image *img, int x0, int y0, int w, int h, Pixel c) {
    for (int y = y0; y < y0 + h; y++)
        for (int x = x0; x < x0 + w; x++)
            set_pixel(img, x, y, c);
}

/*
 * Файл BMP = заголовок (54 байта, «паспорт» картинки: размер файла, ширина, высота,
 * сколько байт на пиксель) + пиксели. Заголовок уже заполнен — подробно разберём его позже.
 * #pragma pack(push, 1) нужен, чтобы структура в памяти лежала байт в байт как в файле.
 */
#pragma pack(push, 1)
typedef struct {
    uint16_t type;                  /* буквы 'BM' */
    uint32_t size;                  /* размер всего файла в байтах */
    uint16_t reserved1, reserved2;  /* нули */
    uint32_t offset;                /* с какого байта начинаются пиксели (54) */
} FileHeader;
typedef struct {
    uint32_t size;                  /* размер этого заголовка (40) */
    int32_t  width, height;         /* ширина и высота в пикселях */
    uint16_t planes, bpp;           /* 1 и 24 бита (3 байта) на пиксель */
    uint32_t compression;           /* 0 — без сжатия */
    uint32_t image_size;            /* сколько байт занимают пиксели */
    int32_t  xppm, yppm;            /* разрешение для печати, нам не важно */
    uint32_t colors_used, colors_important;  /* нули */
} InfoHeader;
#pragma pack(pop)

/* Возвращает 0, если всё записано, и 1 при ошибке. */
int save_bmp(const char *name, const Image *img) {
    int stride = (img->w * 3 + 3) / 4 * 4;   /* длина строки в файле. Почему не w * 3 — выясним */
    FileHeader fh = { 0x4D42, 54 + stride * img->h, 0, 0, 54 };
    InfoHeader ih = { 40, img->w, img->h, 1, 24, 0, stride * img->h, 2835, 2835, 0, 0 };

    FILE *f = fopen(name, "wb");
    if (f == NULL) return 1;
    fwrite(&fh, sizeof fh, 1, f);
    fwrite(&ih, sizeof ih, 1, f);

    /* ВАША ЧАСТЬ: записать пиксели. Каждый пиксель — 3 байта, строка за строкой.
       Один байт записывается так: fputc(значение, f); */

    fclose(f);
    return 0;
}

/* Проверка: сколько байт обещает заголовок и сколько их в файле на самом деле. */
void check_file(const char *name) {
    FILE *f = fopen(name, "rb");
    if (f == NULL) { printf("%s: cannot open\n", name); return; }
    FileHeader fh;
    if (fread(&fh, sizeof fh, 1, f) != 1) { printf("%s: too short\n", name); fclose(f); return; }
    fseek(f, 0, SEEK_END);
    long real = ftell(f);
    fclose(f);
    printf("%s: header says %u bytes, file has %ld bytes - %s\n",
           name, fh.size, real, (long)fh.size == real ? "OK" : "MISMATCH");
}

static const Pixel RED = {255, 0, 0}, WHITE = {255, 255, 255}, BLACK = {0, 0, 0};

int main(void) {
    /* Проверка 1: 2 x 2 красных пикселя. Посчитайте на бумаге, сколько байт должно быть в файле. */
    Image img = img_new(2, 2);
    fill(&img, RED);
    if (save_bmp("check_2x2.bmp", &img) != 0) printf("save_bmp failed\n");
    check_file("check_2x2.bmp");
    free(img.px);

    /* Дальше — задачи с карточки. */

    (void)WHITE; (void)BLACK;
    return 0;
}
