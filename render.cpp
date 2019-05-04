#include "render.h"
#include <SDL2/SDL.h>
#include <time.h>

r_renderer::r_renderer(int _width, int _height)
    : width(_width), height(_height), scale(1), aa_level(0), 
      render_to_window(false), 
      render_to_renderer(NULL), accumulate(false),
      window(0), surface(0), lightness_target(0), use_lightmap(0),
      last_update(0), cancel(false)
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

void r_renderer::subdivide(float x, float y, r_color c00, r_color c20, 
                           r_color c02, r_color c22, float d, int aa_level, 
                           r_color* result)
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
            if (i == 0)
            {
                if (max - min > 4.0 / 255)
                    needs_refinement = true;
            }
            else
            {
                if (max - min > 8.0 / 255)
                    needs_refinement = true;
            }
        }
        if (needs_refinement)
        {
            // pixels are different, recurse
            r_color c10, c01, c11, c21, c12;
            get_color(x + d, y, &c10);
            get_color(x, y + d, &c01);
            get_color(x + d, y + d, &c11);
            get_color(x + d * 2, y + d, &c21);
            get_color(x + d, y + d * 2, &c12);
            
            r_color e00, e20, e02, e22;
            subdivide(x, y, c00, c10, c01, c11, d * 0.5, aa_level - 1, &e00);
            subdivide(x + d, y, c10, c20, c11, c21, d * 0.5, aa_level - 1, &e20);
            subdivide(x, y + d, c01, c11, c02, c12, d * 0.5, aa_level - 1, &e02);
            subdivide(x + d, y + d, c11, c21, c12, c22, d * 0.5, aa_level - 1, &e22);
            c00 = e00; c20 = e20; c02 = e02; c22 = e22;
        }
    }
    // mix 4 colors
    *result = r_color(0, 0, 0);
    for (int i = 0; i < 3; i++)
        result->c[i] = (c00.c[i] + c20.c[i] + c02.c[i] + c22.c[i]) / 4; 
}

void r_renderer::update()
{
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    long now = round(spec.tv_nsec / 1.0e8);
    if (now != last_update)
    {
        SDL_UpdateWindowSurface(window);
        SDL_Event event;
        SDL_PollEvent(&event);
        if (event.type == SDL_KEYDOWN)
            cancel = true;
        last_update = now;
    }
}

void r_renderer::set_pixel(int x, int y, uint32_t color)
{
    if (render_to_window || render_to_renderer)
    {
        
        SDL_Rect r = {x: x * scale, y: y * scale, 
            w: scale, h: scale};
        SDL_FillRect(surface, &r, color);
        update();
    }
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
    r_color* line0 = (r_color*)malloc((width + 1) * sizeof(r_color));
    r_color* line1 = (r_color*)malloc((width + 1) * sizeof(r_color));
    accumulated = 0.0;

    for (int x = 0; x < width + 1; x++)
        get_color(x, 0, &line1[x]);
    
    SDL_Event event;
    for (int y = 0; y < height; y++)
    {
        r_color* temp = line0; line0 = line1; line1 = temp;
        for (int x = 0; x < width + 1; x++)
        {           
            if (x < width && !use_lightmap)
                set_pixel(x, y, 0xff0000);
            get_color(x, y + 1, &line1[x]);
            if (x < width && !use_lightmap)
                set_pixel(x, y, line1[x].to_i());
            if (cancel)
                break;
        }
        for (int x = 0; x < width; x++)
        {
            if (!use_lightmap)
                set_pixel(x, y, 0xff0000);
            r_color color;
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
            if (lightness_target)
                *(lightness_target++) = pow(color.r, 0.5);
            set_pixel(x, y, color.to_i());
            if (cancel)
                break;
        }
        if (event.type == SDL_KEYDOWN)
            break;
        if (cancel)
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

