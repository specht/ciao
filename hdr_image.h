#pragma once

struct hdr_image {
    hdr_image(const char* _path, float _exposure = 0.0);
    virtual ~hdr_image();
    void nope(const char *message);
    float sample(float u, float v);

    const char* path;
    float exposure;
    int width, height;
    float* buffer;
};
