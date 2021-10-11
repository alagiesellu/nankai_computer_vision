//
// Created by ilak on 10/8/21.
//

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
load_image(std::vector<uint8_t>& image, const std::string& filename, int& x, int&y, const int& req_comp)
{
    int n;
    uint8_t* data = stbi_load(filename.c_str(), &x, &y, &n, req_comp);

    if (data != nullptr) {
        image = vector<uint8_t>(data, data + x * y * req_comp);
    }

    stbi_image_free(data);

    return (data != nullptr);
}

int
calculator(vector<unsigned int> pixel, vector<unsigned int> neighboring_pixel) {

    int energy = 0;

    for (int i = 0; i < COLOR_PROPERTY; i++, SEAM_COUNTER++) {
        energy += abs((int)(pixel[i] - neighboring_pixel[i]));
    }

    return energy/COLOR_PROPERTY;
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

    if (next_h >= pixels.size())
        return 0;

    if (energy_and_memory_yx[h][w][1] == -1)
    {
        lowest_index = w;
        lowest = static_cast<int>(energy_and_memory_yx[next_h][lowest_index][0]);

        if (w > 0 && energy_and_memory_yx[next_h][w - 1][0] < lowest) {
            lowest_index = w - 1;
            lowest = static_cast<int>(energy_and_memory_yx[next_h][lowest_index][0]);
        }

        if (w + 1 < pixels[0].size() && energy_and_memory_yx[next_h][w + 1][0] < lowest)
            lowest_index = w + 1;

        energy_and_memory_yx[h][w][1] = lowest_index;
    }

    return
            calculator(pixels[h][w], pixels[next_h][energy_and_memory_yx[h][w][1]])
            +
            find_path(pixels, energy_and_memory_yx, energy_and_memory_yx[h][w][1], next_h);
}

int remove_w;

void
eliminate_column(
        vector<vector<vector<unsigned int>>>& pixels,
        vector<vector<vector<int>>>& energy_and_memory_yx,
        int w
) {

    vector<unsigned int> remove_columns(pixels.size());

    for (int h = 0; h < pixels.size(); h++) {
        remove_columns[h] = w;
        w = static_cast<int>(energy_and_memory_yx[h][w][1]);
    }

    for (int h = 0; h < remove_columns.size(); h++) {
        pixels[h].erase (pixels[h].begin() + remove_columns[h]);
        energy_and_memory_yx[h].erase (energy_and_memory_yx[h].begin() + remove_columns[h]);
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
load_pixels(vector<vector<vector<unsigned int>>>& pixels, vector<uint8_t>& image) {

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
    vector<unsigned int> columns_sum(pixels[0].size());

    int cheapest_index;

    for (int i = 0; i < width - height; ++i)
    {
        cheapest_index = -1;

        for (int w = 0; w < pixels[0].size(); ++w) {
            columns_sum[w] = find_path(pixels, energy_and_memory, w, 0);

            if (cheapest_index == -1 || columns_sum[w] < columns_sum[cheapest_index])
                cheapest_index = w;
        }

        eliminate_column(pixels,energy_and_memory, cheapest_index);
    }
}

int
seam_crop_width(const int i)
{

    vector<uint8_t> image;

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

    write_image(output_image, generate_filename(i, "carved").data(), height, height, COLOR_PROPERTY);

    return 0;
}

int
main()
{
    const int pictures = 2;
    for (int i = 0; i <= pictures; i++)
        seam_crop_width(i);

    return 0;
}