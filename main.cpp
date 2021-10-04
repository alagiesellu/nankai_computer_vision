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

int width, height, lowest, lowest_index, next_h;

// Loads as RGBA... even if file is only RGB
// Feel free to adjust this if you so please, by changing the 4 to a 0.
bool
write_image(uint8_t* image, char const *filename, int& x, int&y, const int& req_comp)
{
    stbi_write_jpg(
            filename,
            x, y, req_comp, image, 100
            );
}

// Loads as RGBA... even if file is only RGB
// Feel free to adjust this if you so please, by changing the 4 to a 0.
bool
load_image(std::vector<unsigned char>& image, const std::string& filename, int& x, int&y, const int& req_comp)
{
    int n;
    unsigned char* data = stbi_load(filename.c_str(), &x, &y, &n, req_comp);

    if (data != nullptr) {
        image = vector<unsigned char>(data, data + x * y * req_comp);
    }

    stbi_image_free(data);

    return (data != nullptr);
}

int
calculator(vector<unsigned int> pixel, vector<unsigned int> neighboring_pixel) {

    int energy = 0;

    for (int i = 0; i < COLOR_PROPERTY; i++, SEAM_COUNTER++)
        energy += abs((int)(pixel[i] - neighboring_pixel[i]));

    return energy;
}

vector<int>
calculate_energy(vector<vector<vector<unsigned int>>>& pixels, int x, int y) {

    vector<int> exit = vector<int>(2);
    SEAM_COUNTER = 0;
    SEAM_ENERGY = 0;

    if (y > 0) SEAM_ENERGY += calculator(pixels[y][x], pixels[y - 1][x]);

    if (y < height - 1) SEAM_ENERGY += calculator(pixels[y][x], pixels[y + 1][x]);

    if (x > 0) SEAM_ENERGY += calculator(pixels[y][x - 1], pixels[y][x]);

    if (x < width - 1) SEAM_ENERGY += calculator(pixels[y][x], pixels[y][x + 1]);

    exit[0] = SEAM_ENERGY / SEAM_COUNTER;
    exit[1] = -1;

    return exit;
}

string
generate_filename(const int i, const std::string& type)
{
    string name = to_string(i);

    if (type.length() > 0)
        name = type + "/" + name;

    return "/home/ilak/Documents/GitHub/nankai-computer-vision/assets/" + name + ".jpg";
}

int
find_path(
        vector<vector<vector<unsigned int>>>& pixels,
        vector<vector<vector<int>>>& energy_and_memory_yx,
        int w,
        int h
        ) {

    next_h = h + 1;

    if (next_h >= height)
        return 0;

    if (energy_and_memory_yx[h][w][1] == -1)
    {
        lowest_index = w;
        lowest = static_cast<int>(energy_and_memory_yx[next_h][lowest_index][0]);

        if (w > 0 && energy_and_memory_yx[next_h][w - 1][0] < lowest) {
            lowest_index = w - 1;
            lowest = static_cast<int>(energy_and_memory_yx[next_h][lowest_index][0]);
        }

        if (w + 1 < width && energy_and_memory_yx[next_h][w + 1][0] < lowest)
            lowest_index = w + 1;

        energy_and_memory_yx[h][w][1] = lowest_index;
    }

    return
            calculator(pixels[h][w], pixels[next_h][energy_and_memory_yx[h][w][1]])
            +
            find_path(pixels, energy_and_memory_yx, energy_and_memory_yx[h][w][1], next_h);
}

void
eliminate_column(
        vector<vector<vector<unsigned int>>>& pixels,
        vector<vector<vector<int>>>& energy_and_memory_yx,
        int w
        ) {
    for (int h = 0; h < height; h++) {
        pixels[h][w][0] = pixels[h][w][1] = pixels[h][w][2] = -1;
        w = static_cast<int>(energy_and_memory_yx[h][w][1]);
    }
}

void
output_image_from_energy(vector<vector<vector<int>>>& energy_and_memory, uint8_t*& output_image) {

    output_image = new uint8_t[width * height];

    int output_counter = 0;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            output_image[output_counter++] = energy_and_memory[y][x][0];
        }
    }
}

void
output_image_from_carved(vector<vector<vector<unsigned int>>>& pixels, uint8_t*& output_image) {

    output_image = new uint8_t[height * width * COLOR_PROPERTY];

    int output_counter = 0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            for (int px = 0; px < COLOR_PROPERTY; px++) {
                if (pixels[y][x][px] != -1)
                    output_image[output_counter++] = static_cast<int>(pixels[y][x][px]);
            }
        }
    }
}

void
calculate_energy_and_memory(vector<vector<vector<unsigned int>>>& pixels, vector<vector<vector<int>>>& energy_and_memory) {

    energy_and_memory = vector<vector<vector<int>>>(
            height,
            vector<vector<int>>(
                    width, vector<int>(2)
            )
    );

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            energy_and_memory[y][x] = calculate_energy(pixels, x, y);
        }
    }
}

void
load_pixels(vector<vector<vector<unsigned int>>>& pixels, vector<unsigned char>& image) {

    pixels = vector<vector<vector<unsigned int>>>(
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
}

void
seam_carve(
        vector<vector<vector<unsigned int>>>& pixels,
        vector<vector<vector<int>>>& energy_and_memory
        )
{
    vector<unsigned int> columns_sum(width);

//    cout << width << " - " << height << endl;
    for (int w = 0; w < width; ++w) {
        columns_sum[w] = find_path(pixels, energy_and_memory, w, 0);
//        cout << w << ":" << columns_sum[w] << endl;
    }

    int cheapest_index, cheapest_column_weight;
    int columns_to_eliminate = width - height;
    int eliminated_columns = 0;

//    cout << eliminated_columns << " < " << columns_to_eliminate << endl;
    while (eliminated_columns < columns_to_eliminate)
    {
        cheapest_column_weight = -1;
        cheapest_index = 0;

        for (int w = 0; w < width; ++w) {
            if (pixels[0][w][0] != -1) {
                if (cheapest_column_weight == - 1) {
                    cheapest_index = w;
                    cheapest_column_weight = static_cast<int>(columns_sum[cheapest_index]);
                } else if (
                        pixels[0][cheapest_index][0] != -1
                        &&
                        columns_sum[w] < cheapest_column_weight
                        ) {
                    cheapest_index = w;
                    cheapest_column_weight = static_cast<int>(columns_sum[cheapest_index]);
                }
            }
        }
        eliminate_column(pixels,energy_and_memory, cheapest_index);
        eliminated_columns++;
    }
}

int
seam_crop_width(const int i)
{

    vector<unsigned char> image;

    bool success = load_image(image, generate_filename(i, ""), width, height, COLOR_PROPERTY);

    if (!success)
    {
        cout << "Error loading image." << endl;
        return 1;
    }

    vector<vector<vector<unsigned int>>> pixels;

    load_pixels(pixels, image);


    vector<vector<vector<int>>> energy_and_memory;
    uint8_t* output_image;

    calculate_energy_and_memory(pixels, energy_and_memory);

    output_image_from_energy(energy_and_memory, output_image);

    write_image(output_image, generate_filename(i, "energy").data(), width, height, 1);

    seam_carve(pixels, energy_and_memory);

    output_image_from_carved(pixels, output_image);

    write_image(output_image, generate_filename(i, "cropped").data(), height, height, COLOR_PROPERTY);

    return 0;
}

int
main()
{
    const int pictures = 6;
    for (int i = 0; i <= pictures; i++)
        seam_crop_width(i);
    return 0;
}