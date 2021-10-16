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

int calculator(vector<int> &pixel, vector<int> &neighboring_pixel) {
    return abs((int)(pixel[0] - neighboring_pixel[0])) + abs((int)(pixel[1] - neighboring_pixel[1])) + abs((int)(pixel[2] - neighboring_pixel[2]));
}

int calculate_energy(vector<vector<vector<int>>>& pixels, int col, int row) {

    int energy = 0;

    if (row > 0) energy += calculator(pixels[row][col], pixels[row - 1][col]);

    if (row + 1 < HEIGHT) energy += calculator(pixels[row][col], pixels[row + 1][col]);

    if (col > 0) energy += calculator(pixels[row][col], pixels[row][col - 1]);

    if (col + 1 < WIDTH) energy += calculator(pixels[row][col], pixels[row][col + 1]);

    return energy;
}

string generate_filename(const int i, const string& type)
{

    string name = to_string(i);

    if (! type.empty())
        name = type + "/" + name;

    return "/home/leber/CLionProjects/nankai_computer_vision/assets/" + name + ".jpg";
}

int get_cheapest_next_col(vector<vector<int>> &pixels_energy, int col, int row) {

    int next_row = row + 1;

    int cheapest_col = static_cast<int>(col);
    int cheapest_energy = static_cast<int>(pixels_energy[next_row][cheapest_col]);

    if (col > 0 && pixels_energy[next_row][col - 1] < cheapest_energy) {
        cheapest_col = col - 1;
        cheapest_energy = static_cast<int>(pixels_energy[next_row][cheapest_col]);
    }

    if (col + 1 < WIDTH && pixels_energy[next_row][col + 1] < cheapest_energy)
        cheapest_col = col + 1;

//    cout << "C:" << col << " - R:" << row << " = " << cheapest_col << endl;

    return cheapest_col;
}

int
calculate_path_energy(
        vector<vector<vector<int>>>& pixels, vector<vector<int>>& pixels_energy,
        vector<vector<int>>& next_step_memory, vector<vector<int>>& path_energy_memory,
        int col, int row
        ) {

    int next_row = row + 1;

    if (next_row == HEIGHT)
        return pixels_energy[row][col];

    if (next_step_memory[row][col] == NULL_INT_VALUE) {
        next_step_memory[row][col] = get_cheapest_next_col(pixels_energy, col, row);
    }

    if (path_energy_memory[row][col] == NULL_INT_VALUE) {
        path_energy_memory[row][col] =
                pixels_energy[row][col]
                +
                calculate_path_energy(pixels, pixels_energy, next_step_memory, path_energy_memory, next_step_memory[row][col], next_row);
    }

    return path_energy_memory[row][col];
}

void remove_2d_element(vector<vector<int>> &list, int col, int row) {
    list[row].erase(list[row].begin() + col);
}

void remove_3d_element(vector<vector<vector<int>>> &list, int col, int row) {
    list[row].erase(list[row].begin() + col);
}

void remove_pixel(
        vector<vector<vector<int>>> &pixels, vector<vector<int>> &pixels_energy, vector<vector<int>> &next_step_memory,
         int &cheapest_col, int row
        ) {

    int remove_col = static_cast<int>(cheapest_col);

    cheapest_col = next_step_memory[row][remove_col];

    remove_3d_element(pixels, remove_col, row);

    remove_2d_element(pixels_energy, remove_col, row);
    remove_2d_element(next_step_memory, remove_col, row);

//    pixels_energy[row][remove_col] = calculate_energy(pixels, remove_col, row);
//
//    int next_remove_col = remove_col - 1;
//    if (next_remove_col > 0)
//        pixels_energy[row][next_remove_col] = calculate_energy(pixels, next_remove_col, row);
//
//    int next_row = row - 1;
//    if (next_row > 0)
//        pixels_energy[next_row][remove_col] = calculate_energy(pixels, remove_col, next_row);

}

void eliminate_path(
        vector<vector<vector<int>>> &pixels,
        vector<vector<int>> &pixels_energy,
        vector<vector<int>> &next_step_memory,
        int cheapest_col
        ) {

    for (int row = 0; row < HEIGHT; row++) {
        remove_pixel(
                pixels,
                pixels_energy,
                next_step_memory,
                cheapest_col,
                row
                );
//        cout << "E: ";
//        next_step_memory[row][cheapest_col] = get_cheapest_next_col(pixels_energy, cheapest_col, row);
    }
}

void carve_image(vector<vector<vector<int>>>& pixels, vector<vector<int>>& pixels_energy) {

    vector<vector<int>> next_step_memory;
    vector<vector<int>> path_energy_memory;

    int path_energy, cheapest_col, cheapest_path_energy;

    while (WIDTH > HEIGHT) {

        next_step_memory = vector<vector<int>>(
                HEIGHT, vector<int>(WIDTH, NULL_INT_VALUE)
        );

        path_energy_memory = vector<vector<int>>(
                HEIGHT, vector<int>(WIDTH, NULL_INT_VALUE)
        );

        cheapest_col = 0, cheapest_path_energy = calculate_path_energy(pixels, pixels_energy, next_step_memory, path_energy_memory, 0, 0);

        for (int col = 1; col < WIDTH; col++) {
            path_energy = calculate_path_energy(pixels, pixels_energy, next_step_memory, path_energy_memory, col, 0);

            if (path_energy < cheapest_path_energy) {
                cheapest_path_energy = path_energy;
                cheapest_col = col;
            }
        }

        eliminate_path(
                pixels,
                pixels_energy,
                next_step_memory,
                cheapest_col
                );
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

    output_counter = 0;

    for (int row = 0; row < HEIGHT; row++) {
        for (int col = 0; col < WIDTH; col++) {
            for (int px = 0; px < COLOR_PROPERTY; px++) {
                output_image[output_counter++] = pixels[row][col][px];
            }
        }
    }

    write_image(output_image, generate_filename(i, "carved").data(), WIDTH, HEIGHT, COLOR_PROPERTY);

    return 0;
}

void test2d(int num) {

    vector<vector<string>> list(
            num,
            vector<string>(num)
    );

    for (int row = 0; row < num; row++) {
        for (int col = 0; col < num; col++) {
            list[row][col] = to_string(row) + to_string(col);
        }
    }

//    remove_2d_element(list, 2, 2);

    for (int row = 0; row < list.size(); row++) {
        for (int col = 0; col < list[row].size(); col++) {
            cout << list[row][col] << "  ";
        }
        cout << endl;
    }
}

void test3d(int num) {

    vector<vector<vector<string>>> list(
            num,
            vector<vector<string>>(
                    num, vector<string>(COLOR_PROPERTY)
            )
    );

    for (int row = 0; row < num; row++) {
        for (int col = 0; col < num; col++) {
            for (int px = 0; px < COLOR_PROPERTY; px++) {
                list[row][col][px] = to_string(row) + to_string(col) + to_string(px);
            }
        }
    }

//    remove_3d_element(list, 2, 2);

    for (int row = 0; row < list.size(); row++) {
        for (int col = 0; col < list[row].size(); col++) {
            for (int px = 0; px < COLOR_PROPERTY; px++) {
                cout << list[row][col][px] << ",";
            }
            cout << "  ";
        }
        cout << endl;
    }
}


int main()
{
//    int num = 6;
//    test3d(num);
//    cout << "------" << endl;
//    test2d(num);
//    return 0;

    const int pictures = 7;
    for (int i = 0; i < pictures; i++)
        seam_crop_width(i);

//    seam_crop_width(1);

    return 0;
}