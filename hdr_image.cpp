#include "hdr_image.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

struct buffered_reader {
    FILE* f;
    uint8_t* rbuf;
    size_t rbuf_size, rbuf_offset;
    const size_t max_size = 1024 * 1024 * 16;
    
    buffered_reader(FILE* _f)
    {
        f = _f;
        rbuf_size = 0;
        rbuf_offset = 0;
        rbuf = (uint8_t*)malloc(max_size);
    }
    
    virtual ~buffered_reader()
    {
        delete [] rbuf;
    }

    uint8_t getb()
    {
        if (rbuf_offset >= rbuf_size)
        {
            rbuf_size = fread(rbuf, 1, max_size, f);
            rbuf_offset = 0;
        }
        return rbuf[rbuf_offset++];
    }
};

hdr_image::hdr_image(const char* _path, float _exposure)
    : path(_path), exposure(pow(2, _exposure)), width(0), height(0), buffer(0)
{
    fprintf(stderr, "Loading HDR image from %s\n", path);
    FILE* f = fopen(path, "r");
    char line[1024];
    line[0] = 0;
    fgets(line, 1023, f);
    if ((strncmp(line, "#?RADIANCE", 10) != 0) && (strncmp(line, "#?RGBE", 6) != 0))
        nope("unknown header");
    while (true)
    {
        fgets(line, 1023, f);
        if (strncmp(line, "FORMAT=", 7) == 0)
        {
            if (strncmp(line + 7, "32-bit_rle_rgbe", 15) != 0)
                nope("unknown format");
        }
        if (strlen(line) == 1)
            break;
    }
    fgets(line, 1023, f);
    int offset = -1;
    for (int i = 0; i < 2; i++)
    {
        offset = strchr(line + offset + 1, ' ') - line;
        char which = line[offset - 1];
        int value = atoi(line + offset);
        offset = strchr(line + offset + 1, ' ') - line;
        if (which == 'X')
            width = value;
        else if (which == 'Y')
            height = value;
        else
            nope("dimensions");
    }
    if (width <= 0 || height <= 0)
        nope("invalid dimensions");
    buffer = new float[width * height * 3];
    int buffer_offset = 0;
    unsigned char* scanline = new unsigned char[width * 4];
    buffered_reader reader(f);
    for (int y = 0; y < height; y++)
    {
        unsigned char snip[4] = { reader.getb(), reader.getb(), reader.getb(), reader.getb()}; 
        if (snip[0] != 2 || snip[1] != 2 || ((((int)snip[2]) << 8) | snip[3]) != width)
            nope("invalid row format");
        int offset = 0;
        for (int c = 0; c < 4; c++)
        {
            while (offset < width * 4)
            {
                uint8_t b = reader.getb();
                if (b > 0x80)
                {
                    uint8_t c = reader.getb();
                    for (int i = 0; i < b - 0x80; i++)
                        scanline[offset++] = c;
                }
                else
                {
                    for (int i = 0; i < b; i++)
                    {
                        uint8_t c = reader.getb();
                        scanline[offset++] = c;
                    }
                }
            }
        }
        for (int x = 0; x < width; x++)
        {
            float b = ((float)scanline[x + 0]) / 255.0;
            float g = ((float)scanline[x + width]) / 255.0;
            float r = ((float)scanline[x + width * 2]) / 255.0;
            int e = scanline[x + width * 3];
            float f = pow(2, e - 128);
            r_color rgb(r * f, g * f, b * f);
            r_color lab;
            rgb.rgb_to_lab(&lab);
            // convert RGB to CIELab and only scale L
//             lab.l *= f;
            buffer[buffer_offset++] = lab.l;
            buffer[buffer_offset++] = lab.a;
            buffer[buffer_offset++] = lab.b;
        }
    }
    delete [] scanline;
    fclose(f);
}

hdr_image::~hdr_image()
{
    if (buffer)
        delete [] buffer;
}

void hdr_image::nope(const char* message)
{
    fprintf(stderr, "Unable to load HDR image file %s (%s).\n", path, message);
    exit(1);
}

void hdr_image::sample(float u, float v, r_color* result)
{
//     r_color c(0.5, 0.5, 0.5);
//     c.rgb_to_lab(result);
//     return;
    u *= width;
    v *= height;
    int x0 = ((int)floor(u)) % width;
    int y0 = ((int)floor(v)) % height;
    int x1 = (x0 + 1) % width;
    int y1 = (y0 + 1) % height;
    float fx = u - (int)floor(u);
    float fy = v - (int)floor(v);
    for (int i = 0; i < 3; i++)
    {
        float l = buffer[(y0 * width + x0) * 3 + i] * (1 - fy) + buffer[(y1 * width + x0) * 3 + i] * fy;
        float r = buffer[(y0 * width + x1) * 3 + i] * (1 - fy) + buffer[(y1 * width + x1) * 3 + i] * fy;
        result->c[i] = (l * (1 - fx) + r * fx) * exposure;
    }
}
