#include <iostream>
#include <vector>
#include <cstdlib>

extern "C" {
    #define STB_IMAGE_IMPLEMENTATION
    #include "libs/stb_image.h"
}

using namespace std;

const int COLOR_PROPERTY = 3;
int SEAM_COUNTER = 0;


// Loads as RGBA... even if file is only RGB
// Feel free to adjust this if you so please, by changing the 4 to a 0.
bool load_image(std::vector<unsigned char>& image, const std::string& filename, int& x, int&y, const int& req_comp)
{
    int n;
    unsigned char* data = stbi_load(filename.c_str(), &x, &y, &n, req_comp);

    if (data != nullptr)
    {
        image = std::vector<unsigned char>(data, data + x * y * req_comp);
    }

    stbi_image_free(data);

    return (data != nullptr);
}

int calculator(vector<unsigned int> pixel, vector<unsigned int> neighbor) {

    int energy = 0;

    for (int i = 0; i < 3; i++) {
        int diff = (int)(pixel[i] - neighbor[i]);
        energy += abs(diff);
        SEAM_COUNTER++;
    }

    return energy;
}

int calculate_energy(vector<vector<vector<unsigned int>>>& pixels, int x, int y, int& width, int& height) {

    SEAM_COUNTER = 0;

    int energy = 0;

    if (y > 0) {
        energy += calculator(pixels[y][x], pixels[y - 1][x]);
    }

    if (y < height - 1) {
        energy += calculator(pixels[y][x], pixels[y + 1][x]);
    }

    if (x > 0) {
        energy += calculator(pixels[y][x - 1], pixels[y][x]);
    }

    if (x < width - 1) {
        energy += calculator(pixels[y][x], pixels[y][x + 1]);
    }

    energy /= SEAM_COUNTER;

    return energy;
}

int seam_carve(const std::string& filename)
{
    int width, height;

    vector<unsigned char> image;

    bool success = load_image(image, filename, width, height, COLOR_PROPERTY);

    if (!success)
    {
        cout << "Error loading image." << endl;
        return 1;
    }

    vector<vector<vector<unsigned int>>> pixels(
            height,
            vector<vector<unsigned int>>(
                    width, vector<unsigned int>(COLOR_PROPERTY)
            )
    );

    int index = 0;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            for (int px = 0; px < COLOR_PROPERTY; px++) {
                pixels[y][x][px] = static_cast<int>(image[index++]);
            }
        }
    }

    cout << "Width: " << pixels.size() << endl;
    cout << "Height: " << pixels[0].size() << endl;
    cout << "Pixels: " << pixels[0][0].size() << endl;

    vector<vector<unsigned int>> pixels_energy(
            height, vector<unsigned int>(width)
    );

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            pixels_energy[y][x] = calculate_energy(pixels, x, y, width, height);
        }
    }

    return 0;
}


int main()
{
    std::string filename = "/home/ilak/Documents/GitHub/nankai-computer-vision/assets/sellu.jpg";

    seam_carve(filename);

    return 0;
}