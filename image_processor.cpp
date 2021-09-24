//
// Created by ilak on 9/24/21.
//

#include <iostream>
#include <vector>

extern "C" {
#define STB_IMAGE_IMPLEMENTATION
#include "libs/stb_image.h"
}

using namespace std;

// Loads as RGBA... even if file is only RGB
// Feel free to adjust this if you so please, by changing the 4 to a 0.
bool load_image(std::vector<unsigned char>& image, const std::string& filename, int& x, int&y, const int& color_property)
{
    int n;
    unsigned char* data = stbi_load(filename.c_str(), &x, &y, &n, color_property);

    if (data != nullptr)
    {
        image = std::vector<unsigned char>(data, data + x * y * color_property);
    }

    stbi_image_free(data);

    return (data != nullptr);
}