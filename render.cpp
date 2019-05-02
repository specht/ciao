#include "render.h"
#include <SDL2/SDL.h>
#include <time.h>

r_renderer::r_renderer(int _width, int _height)
    : width(_width), height(_height), scale(1), aa_level(0), 
      render_to_window(false), 
      render_to_renderer(NULL), accumulate(false),
      window(0), surface(0), lightness_target(0), use_lightmap(0)
{
}

void r_renderer::create_window(int width, int height, int scale)
{
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("ciao", 0, 0, width * scale, height * scale, SDL_WINDOW_BORDERLESS | SDL_WINDOW_SHOWN);
    surface = SDL_GetWindowSurface(window);
    SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0, 0, 0));
    SDL_UpdateWindowSurface(window);
}

void r_renderer::destroy_window()
{
    SDL_DestroyWindow(window);
    SDL_Quit();    
}

void r_renderer::subdivide(float x, float y, rgb c00, rgb c20, 
                           rgb c02, rgb c22, float d, int aa_level, 
                           rgb* result)
{
    if (aa_level > 0)
    {
        bool needs_refinement = false;
        for (int i = 0; i < 3 && !needs_refinement; i++)
        {
            float min = c00.c[i], max = c00.c[i];
            if (c20.c[i] < min) min = c20.c[i];
            if (c20.c[i] > max) max = c20.c[i];
            if (c02.c[i] < min) min = c02.c[i];
            if (c02.c[i] > max) max = c02.c[i];
            if (c22.c[i] < min) min = c22.c[i];
            if (c22.c[i] > max) max = c22.c[i];
            if (max - min > 4.0 / 255)
                needs_refinement = true;
        }
        if (needs_refinement)
        {
            // pixels are different, recurse
            rgb c10, c01, c11, c21, c12;
            get_color(x + d, y, &c10);
            get_color(x, y + d, &c01);
            get_color(x + d, y + d, &c11);
            get_color(x + d * 2, y + d, &c21);
            get_color(x + d, y + d * 2, &c12);
            
            rgb e00, e20, e02, e22;
            subdivide(x, y, c00, c10, c01, c11, d * 0.5, aa_level - 1, &e00);
            subdivide(x + d, y, c10, c20, c11, c21, d * 0.5, aa_level - 1, &e20);
            subdivide(x, y + d, c01, c11, c02, c12, d * 0.5, aa_level - 1, &e02);
            subdivide(x + d, y + d, c11, c21, c12, c22, d * 0.5, aa_level - 1, &e22);
            c00 = e00; c20 = e20; c02 = e02; c22 = e22;
        }
    }
    // mix 4 colors
    *result = rgb(0, 0, 0);
    for (int i = 0; i < 3; i++)
        result->c[i] = (c00.c[i] + c20.c[i] + c02.c[i] + c22.c[i]) / 4; 
}

void r_renderer::render()
{
    if (render_to_window)
        create_window(width, height, scale);
    if (render_to_renderer)
    {
        window = render_to_renderer->window;
        surface = render_to_renderer->surface;
    }
    rgb* line0 = (rgb*)malloc((width + 1) * sizeof(rgb));
    rgb* line1 = (rgb*)malloc((width + 1) * sizeof(rgb));
    accumulated = 0.0;

    for (int x = 0; x < width + 1; x++)
        get_color(x, 0, &line1[x]);
    
    SDL_Event event;
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    long last_update = round(spec.tv_nsec / 1.0e8);
    for (int y = 0; y < height; y++)
    {
        rgb* temp = line0; line0 = line1; line1 = temp;
        for (int x = 0; x < width + 1; x++)
        {            
            if (render_to_window || render_to_renderer)
            {
                
                SDL_Rect r = {x: x * scale, y: y * scale, 
                    w: scale, h: scale};
                SDL_FillRect(surface, &r, 0xff0000);
                clock_gettime(CLOCK_REALTIME, &spec);
                long now = round(spec.tv_nsec / 1.0e8);
                if (now != last_update)
                {
                    SDL_UpdateWindowSurface(window);
                    SDL_PollEvent(&event);
                    if (event.type == SDL_KEYDOWN)
                        break;
                    last_update = now;
                }
            }
            get_color(x, y + 1, &line1[x]);
            if (render_to_window || render_to_renderer)
            {
                SDL_Rect r = {x: x * scale, y: y * scale, 
                    w: scale, h: scale};
                SDL_FillRect(surface, &r, line1[x].to_i());
                clock_gettime(CLOCK_REALTIME, &spec);
                long now = round(spec.tv_nsec / 1.0e8);
                if (now != last_update)
                {
                    SDL_UpdateWindowSurface(window);
                    SDL_PollEvent(&event);
                    if (event.type == SDL_KEYDOWN)
                        break;
                    last_update = now;
                }
            }
        }
        for (int x = 0; x < width; x++)
        {
            if (render_to_window || render_to_renderer)
            {
                
                SDL_Rect r = {x: x * scale, y: y * scale, 
                    w: scale, h: scale};
                SDL_FillRect(surface, &r, 0xff0000);
                clock_gettime(CLOCK_REALTIME, &spec);
                long now = round(spec.tv_nsec / 1.0e8);
                if (now != last_update)
                {
                    SDL_UpdateWindowSurface(window);
                    SDL_PollEvent(&event);
                    if (event.type == SDL_KEYDOWN)
                        break;
                    last_update = now;
                }
            }
            rgb color;
            if (aa_level == 0)
                color = line0[x];
            else 
                subdivide(x, y, line0[x], line0[x + 1], line1[x], line1[x + 1], 0.5, aa_level, &color);
            if (use_lightmap)
            {
                float l = use_lightmap[y * 64 + x];
                color.r *= l; color.g *= l; color.b *= l;
                if (accumulate)
                    accumulated += color.r;
            }
            if (render_to_window || render_to_renderer)
            {
                SDL_Rect r = {x: x * scale, y: y * scale, 
                    w: scale, h: scale};
                SDL_FillRect(surface, &r, color.to_i());
                clock_gettime(CLOCK_REALTIME, &spec);
                long now = round(spec.tv_nsec / 1.0e8);
                if (now != last_update)
                {
                    SDL_UpdateWindowSurface(window);
                    SDL_PollEvent(&event);
                    if (event.type == SDL_KEYDOWN)
                        break;
                    last_update = now;
                }
            }
            if (lightness_target)
            {
                *(lightness_target++) = color.r;
            }
        }
        if (event.type == SDL_KEYDOWN)
            break;
    }
    if (render_to_window || render_to_renderer)
        SDL_UpdateWindowSurface(window);
    if (render_to_window)
    {
        while (event.type != SDL_KEYDOWN)
            SDL_WaitEvent(&event);
        destroy_window();
    }
    free(line0);
    free(line1);
}

