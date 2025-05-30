#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdint.h>
#include "font8x8.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define FONT8_WIDTH 8
#define FONT8_HEIGHT 8
#define FONT16_WIDTH 16
#define FONT16_HEIGHT 16
#define MAX_COLS 100
#define MAX_LINES 60

int FONT_WIDTH = FONT8_WIDTH;
int FONT_HEIGHT = FONT8_HEIGHT;

// Utilitaire pour effacer le framebuffer
void clear_fb(unsigned char *fbp, int screensize) {
    memset(fbp, 0, screensize);
}

void draw_char(unsigned char *fbp, int fb_width, int x, int y, char c, int color) {
    if ((unsigned char)c > 127) return;
    for (int row = 0; row < FONT_HEIGHT; row++) {
        uint8_t bits = font8x8_basic[(uint8_t)c][row];
        for (int col = 0; col < FONT_WIDTH; col++) {
            if (bits & (1 << col)) {
                int loc = (x + col) * 4 + (y + row) * fb_width * 4;
                *(int*)(fbp + loc) = color;
            }
        }
    }
}

void draw_text(unsigned char *fbp, int fb_width, const char *text, int x, int y, int color) {
    for (int i = 0; text[i]; i++) {
        if (text[i] == '\n') {
            y += FONT_HEIGHT;
            x = 0;
            continue;
        }
        draw_char(fbp, fb_width, x, y, text[i], color);
        x += FONT_WIDTH;
    }
}

void draw_png(unsigned char *fbp, int fb_width, const char *filename, int x_offset, int y_offset) {
    int width, height, channels;
    unsigned char *img = stbi_load(filename, &width, &height, &channels, 3);
    if (!img) {
        fprintf(stderr, "Erreur chargement PNG: %s\n", filename);
        return;
    }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int i = (y * width + x) * 3;
            int loc = (x + x_offset) * 4 + (y + y_offset) * fb_width * 4;
            int color = (img[i] << 16) | (img[i + 1] << 8) | img[i + 2];
            *(int*)(fbp + loc) = color;
        }
    }
    stbi_image_free(img);
}

int main(int argc, char **argv) {
    char *png_file = NULL;
    char *text_to_show = NULL;

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-font") && i + 1 < argc) {
            if (!strcmp(argv[i + 1], "large")) {
                FONT_WIDTH = FONT16_WIDTH; // pas encore géré vraiment mais à étendre
                FONT_HEIGHT = FONT16_HEIGHT;
            }
            i++;
        } else if (!strcmp(argv[i], "-image") && i + 1 < argc) {
            png_file = argv[++i];
        } else if (!strcmp(argv[i], "-text") && i + 1 < argc) {
            text_to_show = argv[++i];
        }
    }

    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    int fbfd = open("/dev/fb0", O_RDWR);
    if (fbfd < 0) { perror("open"); return 1; }

    ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo);
    ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo);
    int screensize = vinfo.yres_virtual * finfo.line_length;

    unsigned char *fbp = mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    if ((intptr_t)fbp == -1) { perror("mmap"); return 1; }

    clear_fb(fbp, screensize);

    if (png_file) {
        draw_png(fbp, vinfo.xres, png_file, 0, 0);
    }

    if (text_to_show) {
        draw_text(fbp, vinfo.xres, text_to_show, 10, 10, 0xFFFFFF);
    } else {
        // stdin mode
        char line[1024];
        int row = 0;
        while (fgets(line, sizeof(line), stdin)) {
            for (int i = 0; line[i] && i < MAX_COLS; i++) {
                draw_char(fbp, vinfo.xres, i * FONT_WIDTH, row * FONT_HEIGHT, line[i], 0xFFFFFF);
            }
            row++;
            if (row >= MAX_LINES) {
                clear_fb(fbp, screensize);
                row = 0;
            }
        }
    }

    munmap(fbp, screensize);
    close(fbfd);
    return 0;
}
