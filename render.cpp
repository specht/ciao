#include "render.h"
#include <SDL2/SDL.h>

SDL_Window* window = 0;
SDL_Surface* screenSurface = 0;

void create_window(int width, int height, int scale)
{
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("ciao", 0, 0, 
                              width * scale, height * scale, 
                              SDL_WINDOW_SHOWN);
    screenSurface = SDL_GetWindowSurface(window);
    SDL_FillRect(screenSurface, NULL, 
                 SDL_MapRGB(screenSurface->format, 0, 0, 0));
    SDL_UpdateWindowSurface(window);
}

void destroy_window()
{
    SDL_DestroyWindow(window);
    SDL_Quit();    
}

unsigned char subdivide(float x, float y, 
                        unsigned char c00, 
                        unsigned char c20,
                        unsigned char c02,
                        unsigned char c22,
                        float d, int aa_level,
                        unsigned char (*get_color)(float x, float y))
{
    unsigned char t00 = c00 >> 2;
    unsigned char t20 = c20 >> 2;
    unsigned char t02 = c02 >> 2;
    unsigned char t22 = c22 >> 2;
    if ((aa_level > 0) && (t00 != t20 || t00 != t02 || t00 != t22 || t20 != t22 || t02 != t22 || t20 != t02))
    {
        // pixels are different, recurse
        unsigned char c10 = get_color(x + d, y);
        unsigned char c01 = get_color(x, y + d);
        unsigned char c11 = get_color(x + d, y + d);
        unsigned char c21 = get_color(x + d * 2, y + d);
        unsigned char c12 = get_color(x + d, y + d * 2);
        return ((int)
            subdivide(x, y, c00, c10, c01, c11, d * 0.5, aa_level - 1, get_color) +
            subdivide(x + d, y, c10, c20, c11, c21, d * 0.5, aa_level - 1, get_color) +
            subdivide(x, y + d, c01, c11, c02, c12, d * 0.5, aa_level - 1, get_color) +
            subdivide(x + d, y + d, c11, c21, c12, c22, d * 0.5, aa_level - 1, get_color)) >> 2;
    }
    else
        return ((int)c00 + c20 + c02 + c22) >> 2;
}

void render(int width, int height, int scale, int aa_level, unsigned char (*get_color)(float x, float y))
{
    create_window(width, height, scale);
    unsigned char* line0 = (unsigned char*)malloc(width + 1);
    unsigned char* line1 = (unsigned char*)malloc(width + 1);

    for (int x = 0; x < width + 1; x++)
        line1[x] = get_color(x, 0);
    
    for (int y = 0; y < height; y++)
    {
        unsigned char* temp = line0; line0 = line1; line1 = temp;
        for (int x = 0; x < width + 1; x++)
            line1[x] = get_color(x, y + 1);
        for (int x = 0; x < width; x++)
        {
            uint32_t color = 
                subdivide(x, y, 
                          line0[x], line0[x + 1],
                          line1[x], line1[x + 1],
                          0.5, aa_level, get_color);
            color |= color << 8;
            color |= color << 8;
            SDL_Rect r = {x: x * scale, y: y * scale, 
                w: scale, h: scale};
            SDL_FillRect(screenSurface, &r, color);
        }
        SDL_UpdateWindowSurface(window);
    }
    free(line0);
    free(line1);
    getc(stdin);
}
