#include <stdio.h>
#include <stdint.h>

typedef struct { unsigned char r, g, b; } Pixel;
typedef struct { int w, h; Pixel *px; } Image;

#define IMG_W 16
#define IMG_H 16

static const char *art[IMG_H] = {
    "................",
    "................",
    "....YYYYYYYY....",
    "..YYYYYYYYYYYY..",
    ".YYYYYYYYYYYYYY.",
    ".YYYYKKYYYYKKYY.",
    "YYYYYKKYYYYKKYYY",
    "YYYYYYYYYYYYYYYY",
    "YYYYYYYYYYYYYYYY",
    "YYKYYYYYYYYYYKYY",
    "YYYKYYYYYYYYKYYY",
    ".YYYKKYYYYKKYYY.",
    ".YYYYYKKKKYYYYY.",
    "..YYYYYYYYYYYY..",
    "....YYYYYYYY....",
    "................",
};

uint32_t palette(char c){
    switch (c){
        case 'Y': return 0xFFD800;  // жёлтый
        case 'K': return 0x000000;  // чёрный
        case 'R': return 0xFF0000;  // красный
        default:  return 0xFFFFFF;  // фон
    }
}

Pixel hex_color(uint32_t c){
    Pixel p = {
        (c >> 16) & 0xFF,
        (c >> 8) & 0xFF,
        c & 0xFF
    };
    return p;
}

uint32_t rgb_color(Pixel *p){
    uint32_t hex = (p->r << 16) + (p->g << 8) + (p->b);
    return hex;
}

void show_bin(unsigned char v){
    for (int i = 7; i >= 0; i--){
      printf("%d", (v >> i) & 1);
    }
    printf(" (%d)\n", v);
}

#pragma pack(push, 1)

typedef struct {
    uint16_t signature;
    uint32_t size;
    uint16_t reserved1, reserved2;
    uint32_t offset;
} FileHeader;

typedef struct {
    uint32_t size;
    int32_t width, height;
    uint16_t planes, bpp;
    uint32_t compression, image_size;
    int32_t xppm, yppm;
    uint32_t color_used, solors_important;
} InfoHeader;

#pragma pack(pop)

int save_bmp(const char *name, const Image *img){
    FileHeader fh = {
        0x4D42, 
        img->w * sizeof(Pixel) * img->h, 
        0, 
        0, 
        54
    };
    InfoHeader ih = {
        40,
        img->w,
        img->h,
        1,
        24,
        0,
        img->h,
        2835,
        2835,
        0,
        0
    };
    FILE *f = fopen(name, "wb");
    fwrite(&fh, sizeof(fh), 1, f);
    fwrite(&ih, sizeof(ih), 1, f);
    for (int i = 0; i < img->h; i++){
        int y = img->h - 1 - i;
        for (int x = 0; x < img->w; x++){
            Pixel p = img->px[y * img->w + x];
            fputc(p.b, f);
            fputc(p.g, f);
            fputc(p.r, f);
        }
    }
    fclose(f);
    return 0;
}

int main(){
    unsigned char a = 200, m = 0xF0;
    show_bin(a);
    show_bin(m);
    show_bin(a & m);
    show_bin(a | m);
    show_bin(a ^ m);
    show_bin(~a);
    show_bin(~m);
    show_bin(1 << 3);
    show_bin(a >> 4);
    show_bin(a | 1);
    Pixel p = hex_color(0xFF8800);
    printf("0xFF8800 -> R%d G%d B%d\n", p.r, p.g, p.b);    
    uint32_t hex = rgb_color(&p);
    printf("R%d G%d B%d -> %x", p.r, p.g, p.b, hex);
        Pixel buf[IMG_W * IMG_H];
    for (int y = 0; y < IMG_H; y++)
        for (int x = 0; x < IMG_W; x++)
            buf[y * IMG_W + x] = hex_color(palette(art[y][x]));

    Image img = { IMG_W, IMG_H, buf };
    save_bmp("test.bmp", &img);
    return 0;
}


