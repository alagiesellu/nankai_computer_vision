#include <iostream>
#include <vector>

extern "C" {
    #define STB_IMAGE_IMPLEMENTATION
    #include "libs/stb_image.h"
}

using namespace std;

const size_t RGB = 3;

// Loads as RGBA... even if file is only RGB
// Feel free to adjust this if you so please, by changing the 4 to a 0.
bool load_image(std::vector<unsigned char>& image, const std::string& filename, int& x, int&y)
{
    int n;
    unsigned char* data = stbi_load(filename.c_str(), &x, &y, &n, RGB);

    if (data != nullptr)
    {
        image = std::vector<unsigned char>(data, data + x * y * RGB);
    }

    stbi_image_free(data);

    return (data != nullptr);
}

int main()
{
    std::string filename = "/home/ilak/Documents/GitHub/nankai-computer-vision/assets/sellu.jpg";

    int width, height;

    vector<unsigned char> image;

    bool success = load_image(image, filename, width, height);

    if (!success)
    {
        cout << "Error loading image." << endl;
        return 1;
    }

    cout << "Width = " << width << endl;
    cout << "Height = " << height << endl;

    vector<vector<vector<unsigned int>>> pixels(
            width,
            vector<vector<unsigned int>>(
                    height, vector<unsigned int>(RGB)
                            )
            );

    for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++) {
            size_t px = RGB * (y * width + x);
            pixels[y][x][0] = static_cast<int>(image[px + 0]);
            pixels[y][x][1] = static_cast<int>(image[px + 1]);
            pixels[y][x][2] = static_cast<int>(image[px + 2]);
        }

    cout << "Width: " << pixels.size() << endl;
    cout << "Height: " << pixels[0].size() << endl;
    cout << "Pixels: " << pixels[0][0].size() << endl;

    return 0;
}