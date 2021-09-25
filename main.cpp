#include <iostream>
#include <vector>
#include <cstdlib>

extern "C" {
    #define STB_IMAGE_IMPLEMENTATION
    #include "libs/stb_image.h"

    #define STBI_MSC_SECURE_CRT
    #define STB_IMAGE_WRITE_IMPLEMENTATION
    #include "libs/stb_image_write.h"
}

using namespace std;

const int COLOR_PROPERTY = 3;
int SEAM_COUNTER = 0;
int SEAM_ENERGY = 0;




// Loads as RGBA... even if file is only RGB
// Feel free to adjust this if you so please, by changing the 4 to a 0.
bool write_image(uint8_t* image, char const *filename, int& x, int&y)
{
    int n;
    stbi_write_jpg(
            filename,
            x, y, COLOR_PROPERTY, image, 100
            );
}



// Loads as RGBA... even if file is only RGB
// Feel free to adjust this if you so please, by changing the 4 to a 0.
bool load_image(std::vector<unsigned char>& image, const std::string& filename, int& x, int&y, const int& req_comp)
{
    int n;
    unsigned char* data = stbi_load(filename.c_str(), &x, &y, &n, req_comp);

    if (data != nullptr)
    {
        image = vector<unsigned char>(data, data + x * y * req_comp);
    }

    stbi_image_free(data);

    return (data != nullptr);
}

int calculator(vector<unsigned int> pixel, vector<unsigned int> neighboring_pixel) {

    int energy = 0;

    for (int i = 0; i < COLOR_PROPERTY; i++, SEAM_COUNTER++)
        energy += abs((int)(pixel[i] - neighboring_pixel[i]));

    return energy;
}

int calculate_energy(vector<vector<vector<unsigned int>>>& pixels, int x, int y, int& width, int& height) {

    SEAM_COUNTER = 0;
    SEAM_ENERGY = 0;

    if (y > 0)
        SEAM_ENERGY += calculator(pixels[y][x], pixels[y - 1][x]);

    if (y < height - 1)
        SEAM_ENERGY += calculator(pixels[y][x], pixels[y + 1][x]);

    if (x > 0)
        SEAM_ENERGY += calculator(pixels[y][x - 1], pixels[y][x]);

    if (x < width - 1)
        SEAM_ENERGY += calculator(pixels[y][x], pixels[y][x + 1]);

    return SEAM_ENERGY / SEAM_COUNTER;
}

int seam_carve(const std::string& filename,  char const *carved_filename)
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

    vector<int> accumulative_energy(width);

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            accumulative_energy[x] += (int) pixels_energy[y][x];
        }
    }

    int start_width = 0;
    int best_index = 0;
    int best_weight = 0;

    while (start_width < width - height) {
        int temp_best_weight = 0;

        for (int x = start_width; x < height; x++)
            temp_best_weight += accumulative_energy[x];

        if (temp_best_weight > best_weight) {
            best_weight = static_cast<int>(temp_best_weight);
            best_index = static_cast<int>(start_width);
        }
        start_width++;
    }

    uint8_t* output_image = new uint8_t[width * height * COLOR_PROPERTY];

    int output_counter = 0;

    for (int y = 0; y < height; y++) {
        for (int x = best_index; x < best_index + height; x++) {
            for (int px = 0; px < COLOR_PROPERTY; px++) {
                output_image[output_counter++] = pixels[y][x][px];
            }
        }
    }

    write_image(output_image, carved_filename, height, height);

    return 0;
}


int main()
{

    for (int i = 0; i <= 3; i++)
        seam_carve(
                "/home/ilak/Documents/GitHub/nankai-computer-vision/assets/" + to_string(i) + ".jpg",
                ("/home/ilak/Documents/GitHub/nankai-computer-vision/assets/" + to_string(i) + "_cropped.jpg").c_str()
                );

    return 0;
}