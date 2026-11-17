/*
 * Занятие 1 «Рисуем кодом» — ЭТАЛОННОЕ РЕШЕНИЕ (только для преподавателя).
 *
 * Сборка и запуск:
 *   gcc -std=c17 -Wall -Wextra canvas_solution.c -o canvas_solution -lm
 *   ./canvas_solution            (на Windows: .\canvas_solution.exe)
 *
 * Создаёт в текущей папке все картинки занятия (см. README.md рядом).
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

/* ---------- картинка в памяти: строка 0 — верхняя, без выравнивания ---------- */

typedef struct { unsigned char r, g, b; } Pixel;
typedef struct { int w, h; Pixel *px; } Image;

Image img_new(int w, int h) {
    Image img = { w, h, calloc((size_t)w * h, sizeof(Pixel)) };  /* calloc: всё чёрное */
    return img;
}

void set_pixel(Image *img, int x, int y, Pixel c) {
    if (x < 0 || y < 0 || x >= img->w || y >= img->h) return;     /* за краем — молча пропустить */
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

/* ---------- запись BMP: то, что студенты пишут сами ---------- */

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

int save_bmp(const char *name, const Image *img) {
    int stride = (img->w * 3 + 3) / 4 * 4;               /* строка в файле кратна 4 байтам */
    FileHeader fh = { 0x4D42, 54 + stride * img->h, 0, 0, 54 };
    InfoHeader ih = { 40, img->w, img->h, 1, 24, 0, stride * img->h, 2835, 2835, 0, 0 };

    FILE *f = fopen(name, "wb");                          /* "wb", не "w" */
    if (f == NULL) return 1;
    fwrite(&fh, sizeof fh, 1, f);
    fwrite(&ih, sizeof ih, 1, f);

    unsigned char *row = calloc(stride, 1);               /* хвост строки остаётся нулями */
    if (row == NULL) { fclose(f); return 1; }
    for (int i = 0; i < img->h; i++) {                    /* первой в файле идёт НИЖНЯЯ строка */
        int y = img->h - 1 - i;
        for (int x = 0; x < img->w; x++) {
            Pixel p = img->px[y * img->w + x];
            row[x * 3]     = p.b;                         /* порядок B, G, R */
            row[x * 3 + 1] = p.g;
            row[x * 3 + 2] = p.r;
        }
        fwrite(row, 1, stride, f);
    }
    free(row);
    fclose(f);
    return 0;
}

/* ---------- цвета ---------- */

static const Pixel RED = {255, 0, 0}, GREEN = {0, 255, 0}, BLUE = {0, 0, 255};
static const Pixel WHITE = {255, 255, 255}, BLACK = {0, 0, 0};
static const Pixel RU_BLUE = {0, 57, 166}, RU_RED = {213, 43, 30};

/* ---------- задачи ---------- */

/* Проверка 1: 2 x 2 красных — файл ровно 70 байт (54 + 8 * 2). */
void task_check_2x2(void) {
    Image img = img_new(2, 2);
    fill(&img, RED);
    save_bmp("check_2x2.bmp", &img);
    free(img.px);
}

/* Проверка 2: нарисовать test_5x3.bmp по таблице и сравнить с эталоном (fc.exe /b). */
void task_check_5x3(void) {
    Pixel grid[3][5] = {
        { RED, GREEN, BLUE, WHITE, BLACK },
        { {255,255,0}, {0,255,255}, {255,0,255}, {128,128,128}, {200,100,50} },
        { {10,10,10}, {60,60,60}, {110,110,110}, {160,160,160}, {210,210,210} },
    };
    Image img = img_new(5, 3);
    for (int y = 0; y < 3; y++)
        for (int x = 0; x < 5; x++)
            set_pixel(&img, x, y, grid[y][x]);
    save_bmp("my_5x3.bmp", &img);
    free(img.px);
}

/* Задача 1: флаг России 101 x 60 (ширина нечётная — нарочно). */
void task_flag_ru(void) {
    Image img = img_new(101, 60);
    fill_rect(&img, 0, 0,  101, 20, WHITE);
    fill_rect(&img, 0, 20, 101, 20, RU_BLUE);
    fill_rect(&img, 0, 40, 101, 20, RU_RED);
    save_bmp("flag_ru.bmp", &img);
    free(img.px);
}

/* Задача 2: флаг Японии 150 x 100 — круг: (x - cx)^2 + (y - cy)^2 <= r^2. */
void task_flag_jp(void) {
    Image img = img_new(150, 100);
    fill(&img, WHITE);
    int cx = 75, cy = 50, r = 30;
    for (int y = 0; y < img.h; y++)
        for (int x = 0; x < img.w; x++)
            if ((x - cx) * (x - cx) + (y - cy) * (y - cy) <= r * r)
                set_pixel(&img, x, y, (Pixel){188, 0, 45});
    save_bmp("flag_jp.bmp", &img);
    free(img.px);
}

/* Задача 3: градиент 256 x 256 — красный растёт слева направо, синий сверху вниз. */
void task_gradient(void) {
    Image img = img_new(256, 256);
    for (int y = 0; y < 256; y++)
        for (int x = 0; x < 256; x++)
            set_pixel(&img, x, y, (Pixel){ x, 0, y });
    save_bmp("gradient.bmp", &img);
    free(img.px);
}

/* Задача 4: шахматная доска 160 x 160, клетка 20, левая верхняя — белая. */
void task_chess(void) {
    Image img = img_new(160, 160);
    for (int y = 0; y < img.h; y++)
        for (int x = 0; x < img.w; x++)
            set_pixel(&img, x, y, ((x / 20 + y / 20) % 2 == 0) ? WHITE : BLACK);
    save_bmp("chess.bmp", &img);
    free(img.px);
}

/* Задача 5: мишень 201 x 201 — кольца шириной 20, красное и белое по очереди. */
void task_target(void) {
    Image img = img_new(201, 201);
    for (int y = 0; y < img.h; y++)
        for (int x = 0; x < img.w; x++) {
            double d = sqrt((x - 100) * (x - 100) + (y - 100) * (y - 100));
            set_pixel(&img, x, y, ((int)(d / 20) % 2 == 0) ? RED : WHITE);
        }
    save_bmp("target.bmp", &img);
    free(img.px);
}

/* Задача 6 ★: треугольник Серпинского — одна битовая операция. */
void task_sierpinski(void) {
    Image img = img_new(256, 256);
    for (int y = 0; y < 256; y++)
        for (int x = 0; x < 256; x++)
            set_pixel(&img, x, y, ((x & y) == 0) ? BLACK : WHITE);
    save_bmp("sierpinski.bmp", &img);
    free(img.px);
}

/* Демо в начале пары («вот что можно нарисовать кодом»): множество Мандельброта. */
void demo_mandelbrot(void) {
    Image img = img_new(600, 400);
    for (int py = 0; py < img.h; py++)
        for (int px = 0; px < img.w; px++) {
            double cr = -2.2 + 3.2 * px / img.w;      /* точка комплексной плоскости */
            double ci = -1.2 + 2.4 * py / img.h;
            double zr = 0, zi = 0;
            int n = 0;
            while (n < 100 && zr * zr + zi * zi < 4) { /* z = z^2 + c */
                double t = zr * zr - zi * zi + cr;
                zi = 2 * zr * zi + ci;
                zr = t;
                n++;
            }
            Pixel c = BLACK;
            if (n < 100) c = (Pixel){ (n * 9) % 256, (n * 4) % 256, 128 + (n * 13) % 128 };
            set_pixel(&img, px, py, c);
        }
    save_bmp("mandelbrot.bmp", &img);
    free(img.px);
}

int main(void) {
    task_check_2x2();
    task_check_5x3();
    task_flag_ru();
    task_flag_jp();
    task_gradient();
    task_chess();
    task_target();
    task_sierpinski();
    demo_mandelbrot();
    printf("done\n");
    return 0;
}
