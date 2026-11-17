/*
 * bmpsplit — раскладывает BMP на 3 изображения в оттенках серого (R, G, B).
 *
 * Сборка:  gcc -O2 -Wall -o bmpsplit bmpsplit.c
 * Запуск:  ./bmpsplit input.bmp [out_prefix]
 * Результат: <prefix>_R.bmp, <prefix>_G.bmp, <prefix>_B.bmp
 *
 * Вход:  несжатый BMP 24 или 32 бит (BI_RGB / BI_BITFIELDS для 32 бит
 *        со стандартными масками), bottom-up и top-down.
 * Выход: 8-битный BMP с серой палитрой (256 оттенков).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

static uint16_t rd16(const uint8_t *p) { return (uint16_t)(p[0] | p[1] << 8); }
static uint32_t rd32(const uint8_t *p) { return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24; }
static void wr16(uint8_t *p, uint16_t v) { p[0] = v; p[1] = v >> 8; }
static void wr32(uint8_t *p, uint32_t v) { p[0] = v; p[1] = v >> 8; p[2] = v >> 16; p[3] = v >> 24; }

static int write_gray_bmp(const char *path, const uint8_t *data, int w, int h)
{
    uint32_t row = (uint32_t)(w + 3) & ~3u;
    uint32_t pal = 256 * 4;
    uint32_t off = 14 + 40 + pal;
    uint32_t img = row * (uint32_t)h;

    uint8_t hdr[54] = {0};
    hdr[0] = 'B'; hdr[1] = 'M';
    wr32(hdr + 2, off + img);
    wr32(hdr + 10, off);
    wr32(hdr + 14, 40);
    wr32(hdr + 18, (uint32_t)w);
    wr32(hdr + 22, (uint32_t)h);          /* bottom-up */
    wr16(hdr + 26, 1);
    wr16(hdr + 28, 8);
    wr32(hdr + 30, 0);                    /* BI_RGB */
    wr32(hdr + 34, img);
    wr32(hdr + 38, 2835);                 /* 72 dpi */
    wr32(hdr + 42, 2835);
    wr32(hdr + 46, 256);
    wr32(hdr + 50, 256);

    FILE *f = fopen(path, "wb");
    if (!f) { perror(path); return -1; }
    fwrite(hdr, 1, sizeof hdr, f);

    for (int i = 0; i < 256; i++) {
        uint8_t e[4] = { (uint8_t)i, (uint8_t)i, (uint8_t)i, 0 };
        fwrite(e, 1, 4, f);
    }

    uint8_t *buf = calloc(1, row);
    if (!buf) { fclose(f); return -1; }
    for (int y = h - 1; y >= 0; y--) {    /* data хранится сверху вниз */
        memcpy(buf, data + (size_t)y * w, (size_t)w);
        fwrite(buf, 1, row, f);
    }
    free(buf);

    if (fclose(f) != 0) { perror(path); return -1; }
    return 0;
}

int main(int argc, char **argv)
{
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s input.bmp [out_prefix]\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(argv[1], "rb");
    if (!f) { perror(argv[1]); return 1; }
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (fsize < 54) { fprintf(stderr, "Файл слишком мал для BMP\n"); fclose(f); return 1; }

    uint8_t *file = malloc((size_t)fsize);
    if (!file || fread(file, 1, (size_t)fsize, f) != (size_t)fsize) {
        fprintf(stderr, "Ошибка чтения\n"); fclose(f); free(file); return 1;
    }
    fclose(f);

    if (file[0] != 'B' || file[1] != 'M') { fprintf(stderr, "Не BMP (нет сигнатуры BM)\n"); free(file); return 1; }

    uint32_t off   = rd32(file + 10);
    uint32_t hsize = rd32(file + 14);
    if (hsize < 40) { fprintf(stderr, "Неподдерживаемый заголовок (OS/2 BMP)\n"); free(file); return 1; }

    int32_t  w     = (int32_t)rd32(file + 18);
    int32_t  hraw  = (int32_t)rd32(file + 22);
    uint16_t bpp   = rd16(file + 28);
    uint32_t comp  = rd32(file + 30);

    int topdown = hraw < 0;
    int h = topdown ? -hraw : hraw;

    if (w <= 0 || h <= 0) { fprintf(stderr, "Некорректный размер %dx%d\n", w, hraw); free(file); return 1; }
    if (bpp != 24 && bpp != 32) { fprintf(stderr, "Поддерживается только 24/32 бит, а тут %u\n", bpp); free(file); return 1; }
    if (!(comp == 0 || (comp == 3 && bpp == 32))) { fprintf(stderr, "Сжатые BMP не поддерживаются (compression=%u)\n", comp); free(file); return 1; }

    /* Для BI_BITFIELDS проверяем, что маски стандартные (BGRA) */
    if (comp == 3) {
        /* и для BITMAPINFOHEADER, и для V4/V5 маски лежат по смещению 54 */
        const uint8_t *m = file + 54;
        uint32_t rm = rd32(m), gm = rd32(m + 4), bm = rd32(m + 8);
        if (rm != 0x00FF0000 || gm != 0x0000FF00 || bm != 0x000000FF) {
            fprintf(stderr, "Нестандартные битовые маски не поддерживаются\n"); free(file); return 1;
        }
    }

    int bytespp = bpp / 8;
    uint32_t stride = ((uint32_t)w * bpp + 31) / 32 * 4;
    if ((uint64_t)off + (uint64_t)stride * h > (uint64_t)fsize) {
        fprintf(stderr, "Файл обрезан: данных пикселей меньше, чем заявлено\n"); free(file); return 1;
    }

    size_t n = (size_t)w * h;
    uint8_t *ch[3];
    for (int c = 0; c < 3; c++) {
        ch[c] = malloc(n);
        if (!ch[c]) { fprintf(stderr, "Нет памяти\n"); return 1; }
    }

    /* ch[0]=R, ch[1]=G, ch[2]=B; в BMP порядок байт B,G,R(,A) */
    for (int y = 0; y < h; y++) {
        int srcy = topdown ? y : h - 1 - y;
        const uint8_t *src = file + off + (size_t)srcy * stride;
        for (int x = 0; x < w; x++) {
            const uint8_t *px = src + (size_t)x * bytespp;
            size_t i = (size_t)y * w + x;
            ch[2][i] = px[0];
            ch[1][i] = px[1];
            ch[0][i] = px[2];
        }
    }
    free(file);

    /* Префикс: аргумент или имя входа без .bmp */
    char prefix[4096];
    if (argc == 3) {
        snprintf(prefix, sizeof prefix, "%s", argv[2]);
    } else {
        snprintf(prefix, sizeof prefix, "%s", argv[1]);
        size_t L = strlen(prefix);
        if (L > 4 && (strcmp(prefix + L - 4, ".bmp") == 0 || strcmp(prefix + L - 4, ".BMP") == 0))
            prefix[L - 4] = '\0';
    }

    const char *names[3] = { "R", "G", "B" };
    int rc = 0;
    for (int c = 0; c < 3; c++) {
        char out[4200];
        snprintf(out, sizeof out, "%s_%s.bmp", prefix, names[c]);
        if (write_gray_bmp(out, ch[c], w, h) == 0)
            printf("%s\n", out);
        else
            rc = 1;
        free(ch[c]);
    }
    return rc;
}