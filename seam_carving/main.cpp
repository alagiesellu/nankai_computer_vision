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

#define COLOR_PROPERTY 3
#define NULL_INT_VALUE (-1)

int HEIGHT, WIDTH;

// Loads as RGBA... even if file is only RGB
// Feel free to adjust this if you so please, by changing the 4 to a 0.
void write_image(uint8_t* image, char const *filename, int& col, int&row, const int& req_comp)
{
    stbi_write_jpg(
            filename,
            col, row, req_comp, image, 100
    );
}

// Loads as RGBA... even if file is only RGB
// Feel free to adjust this if you so please, by changing the 4 to a 0.
bool load_image(std::vector<unsigned char>& image, const std::string& filename, int& col, int&row, const int& req_comp)
{
    int n;
    unsigned char* data = stbi_load(filename.c_str(), &col, &row, &n, req_comp);

    if (data != nullptr) {
        image = vector<unsigned char>(data, data + col * row * req_comp);
    }

    stbi_image_free(data);

    return (data != nullptr);
}

int calculator(vector<int> pixel, vector<int> neighboring_pixel) {

    int energy = 0;

    for (int i = 0; i < COLOR_PROPERTY; i++)
        energy += abs((int)(pixel[i] - neighboring_pixel[i]));

    return energy;
}

int calculate_energy(vector<vector<vector<int>>>& pixels, int col, int row) {

    int energy = 0;

    if (row > 0) energy += calculator(pixels[row][col], pixels[row - 1][col]);

    if (row < HEIGHT - 1) energy += calculator(pixels[row][col], pixels[row + 1][col]);

    if (col > 0) energy += calculator(pixels[row][col - 1], pixels[row][col]);

    if (col < WIDTH - 1) energy += calculator(pixels[row][col], pixels[row][col + 1]);

    return energy;
}

string generate_filename(const int i, const string& type)
{

    string name = to_string(i);

    if (! type.empty())
        name = type + "/" + name;

    return "/home/leber/CLionProjects/nankai_computer_vision/assets/" + name + ".jpg";
}

int get_cheapest_col(vector<vector<int>> &pixels_energy, int col, int next_row) {

    int cheapest_col = col;
    int cheapest_energy = static_cast<int>(pixels_energy[next_row][col]);

    if (col > 0 && pixels_energy[next_row][col - 1] < cheapest_energy) {
        cheapest_col = col - 1;
        cheapest_energy = static_cast<int>(pixels_energy[next_row][cheapest_col]);
    }

    if (col + 1 < WIDTH && pixels_energy[next_row][col + 1] < cheapest_energy)
        cheapest_col = col + 1;

    return cheapest_col;
}

int
calculate_path_energy(
        vector<vector<vector<int>>>& pixels, vector<vector<int>>& pixels_energy, vector<vector<int>>& next_step_memory,
        int col, int row
        ) {

    if (row + 1 >= HEIGHT)
        return pixels_energy[row][col];

    int next_row = row + 1;

    if (next_step_memory[row][col] == NULL_INT_VALUE) {
        next_step_memory[row][col] = get_cheapest_col(pixels_energy, col, next_row);
    }

    return
            pixels_energy[row][col]
            +
            calculate_path_energy(pixels, pixels_energy, next_step_memory, next_step_memory[row][col], next_row);
}

void remove_pixel(vector<vector<vector<int>>> &pixels, vector<vector<int>> &pixels_energy, vector<vector<int>> &next_step_memory,
                  int remove_col, int &col, int row) {

    remove_col = static_cast<int>(col);

    if (next_step_memory[row][col] == NULL_INT_VALUE) {
        next_step_memory[row][col] = get_cheapest_col(pixels_energy, col, row + 1);
    }

    col = next_step_memory[row][col];

    pixels[row][remove_col].clear();
//    pixels[row].erase (pixels[row].begin() + remove_col);
    pixels_energy[row].clear();//.erase (pixels_energy[row].begin() + remove_col);
    next_step_memory[row].clear();//.erase (next_step_memory[row].begin() + remove_col);

    pixels_energy[row][col] = calculate_energy(pixels, col, row);
}

void eliminate_path(vector<vector<vector<int>>> &pixels, vector<vector<int>> &pixels_energy, vector<vector<int>> &next_step_memory,
                    int col) {
    int remove_col;
    for (int row = 0; row < HEIGHT - 1; row++) {
        remove_pixel(pixels, pixels_energy, next_step_memory, remove_col, col, row);
        next_step_memory[row][col] = get_cheapest_col(pixels_energy, col, row + 1);
    }
    remove_pixel(pixels, pixels_energy, next_step_memory, remove_col, col, HEIGHT - 1);

}

void carve_image(vector<vector<vector<int>>>& pixels, vector<vector<int>>& pixels_energy) {

    vector<vector<int>> next_step_memory(
            HEIGHT, vector<int>(WIDTH, NULL_INT_VALUE)
    );

    while (WIDTH > HEIGHT) {

        int path_energy, cheapest_col = 0, cheapest_path_energy = calculate_path_energy(pixels, pixels_energy, next_step_memory, 0, 0);

        for (int col = 1; col < WIDTH; col++) {
            path_energy = calculate_path_energy(pixels, pixels_energy, next_step_memory, col, 0);

            if (path_energy < cheapest_path_energy) {
                cheapest_path_energy = path_energy;
                cheapest_col = col;
            }
        }

        eliminate_path(pixels, pixels_energy, next_step_memory, cheapest_col);

        WIDTH--;
    }
}

int seam_crop_width(const int i)
{
    vector<unsigned char> image;

    bool success = load_image(image, generate_filename(i, ""), WIDTH, HEIGHT, COLOR_PROPERTY);

    cout << "Width: " << WIDTH << " Height: " << HEIGHT << endl;

    if (!success)
    {
        cout << "Error loading image." << endl;
        return 1;
    }

    vector<vector<vector<int>>> pixels(
            HEIGHT,
            vector<vector<int>>(
                    WIDTH, vector<int>(COLOR_PROPERTY)
            )
    );

    int index = 0;

    for (int row = 0; row < HEIGHT; row++) {
        for (int col = 0; col < WIDTH; col++) {
            for (int px = 0; px < COLOR_PROPERTY; px++) {
                pixels[row][col][px] = static_cast<int>(image[index++]);
            }
        }
    }

    vector<vector<int>> pixels_energy(
            HEIGHT, vector<int>(WIDTH)
    );

    for (int row = 0; row < HEIGHT; row++) {
        for (int col = 0; col < WIDTH; col++) {
            pixels_energy[row][col] = calculate_energy(pixels, col, row);
        }
    }

    uint8_t* output_image = new uint8_t[WIDTH * HEIGHT];

    int output_counter = 0;

    for (int row = 0; row < HEIGHT; row++) {
        for (int col = 0; col < WIDTH; col++) {
            output_image[output_counter++] = pixels_energy[row][col];
        }
    }

    write_image(output_image, generate_filename(i, "energy").data(), WIDTH, HEIGHT, 1);

    carve_image(pixels, pixels_energy);

    output_image = new uint8_t[WIDTH * HEIGHT * COLOR_PROPERTY];

    write_image(output_image, generate_filename(i, "carved").data(), WIDTH, HEIGHT, COLOR_PROPERTY);

    return 0;
}


int main()
{
    const int pictures = 1;
    for (int i = 0; i < pictures; i++)
        seam_crop_width(i);
    return 0;
}