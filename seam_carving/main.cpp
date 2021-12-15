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

//#define WIDTH_TYPE "WIDTH"
//#define HEIGHT_TYPE "HEIGHT"

#define JPG_EXTENSION "jpg"

#define IMAGES_DIRECTORY_PATH "/home/leber/CLionProjects/nankai_computer_vision/assets/"

int HEIGHT, WIDTH;
double WIDTH_PERCENTAGE_TO_REDUCE = 50;

vector<unsigned char> IMAGE;
vector<vector<vector<int>>> PIXELS;
vector<vector<int>> PIXELS_ENERGY;
uint8_t* OUTPUT_IMAGE;

vector<vector<int>> NEXT_STEP_MEMORY;
vector<vector<int>> PATH_ENERGY_MEMORY;
vector<int> CARVED_PATH;

void write_image(char const *filename, int& col, int&row, const int& req_comp)
{
    stbi_write_jpg(
            filename,
            col, row, req_comp, OUTPUT_IMAGE, 100
    );
}

void load_image(const string& filename, int& col, int&row, const int& req_comp)
{
    int n;
    unsigned char* data = stbi_load(filename.c_str(), &col, &row, &n, req_comp);

    if (data != nullptr) {
        IMAGE = vector<unsigned char>(data, data + col * row * req_comp);
    }

    stbi_image_free(data);

    if (data == nullptr)
    {
        cout << "Error loading IMAGE." << endl;
    }

    cout << WIDTH << " x " << HEIGHT << endl;
}

int calculator(vector<int> &pixel, vector<int> &neighboring_pixel) {
    return abs((int)(pixel[0] - neighboring_pixel[0])) + abs((int)(pixel[1] - neighboring_pixel[1])) + abs((int)(pixel[2] - neighboring_pixel[2]));
}

int calculate_energy(int col, int row) {

    int energy = 0;

    if (row > 0) energy += calculator(PIXELS[row][col], PIXELS[row - 1][col]);

    if (row + 1 < HEIGHT) energy += calculator(PIXELS[row][col], PIXELS[row + 1][col]);

    if (col > 0) energy += calculator(PIXELS[row][col], PIXELS[row][col - 1]);

    if (col + 1 < WIDTH) energy += calculator(PIXELS[row][col], PIXELS[row][col + 1]);

    return energy;
}

string generate_filename(const int i, const string& extension, const string& destination = "")
{

    string name = to_string(i);

    if (! destination.empty())
        name = destination + "/" + name;

    return IMAGES_DIRECTORY_PATH + name + "." + extension;
}

int get_cheapest_next_col(int col, int next_row) {

    int cheapest_col = static_cast<int>(col);
    int cheapest_energy = static_cast<int>(PIXELS_ENERGY[next_row][cheapest_col]);

    if (col > 0 && PIXELS_ENERGY[next_row][col - 1] < cheapest_energy) {
        cheapest_col = col - 1;
        cheapest_energy = static_cast<int>(PIXELS_ENERGY[next_row][cheapest_col]);
    }

    if (col + 1 < WIDTH && PIXELS_ENERGY[next_row][col + 1] < cheapest_energy)
        cheapest_col = col + 1;

    return cheapest_col;
}

int
calculate_path_energy_up(int col, int row) {

    if (row == 0)
        return PIXELS_ENERGY[row][col];

    if (PATH_ENERGY_MEMORY[row][col] != NULL_INT_VALUE) {
        return PATH_ENERGY_MEMORY[row][col];
    }

    int next_row = row - 1;

    NEXT_STEP_MEMORY[row][col] = get_cheapest_next_col(col, next_row);

    PATH_ENERGY_MEMORY[row][col] = PIXELS_ENERGY[row][col] + calculate_path_energy_up(NEXT_STEP_MEMORY[row][col], next_row);

    return PATH_ENERGY_MEMORY[row][col];
}

int
calculate_path_energy_down(int col, int row) {

    int next_row = row + 1;

    if (next_row == HEIGHT)
        return PIXELS_ENERGY[row][col];

    if (PATH_ENERGY_MEMORY[row][col] != NULL_INT_VALUE) {
        return PATH_ENERGY_MEMORY[row][col];
    }

    NEXT_STEP_MEMORY[row][col] = get_cheapest_next_col(col, next_row);

    PATH_ENERGY_MEMORY[row][col] = PIXELS_ENERGY[row][col] + calculate_path_energy_down(NEXT_STEP_MEMORY[row][col], next_row);

    return PATH_ENERGY_MEMORY[row][col];
}

int
calculate_path_energy(int col, int row) {

    return calculate_path_energy_up(col, row - 1) + calculate_path_energy_down(col, row);
}

void remove_2d_element(vector<vector<int>> &list, int col, int row) {
    list[row].erase(list[row].begin() + col);
}

void remove_3d_element(vector<vector<vector<int>>> &list, int col, int row) {
    list[row].erase(list[row].begin() + col);
}

void remove_pixel(int &cheapest_col, int row) {

    int remove_col = static_cast<int>(cheapest_col);

    cheapest_col = NEXT_STEP_MEMORY[row][remove_col];

    remove_3d_element(PIXELS, remove_col, row);

    remove_2d_element(PIXELS_ENERGY, remove_col, row);

    CARVED_PATH[row] = static_cast<int>(remove_col);

}

void calculate_pixel_energy(int col, int row) {
    PIXELS_ENERGY[row][col] = calculate_energy(col, row);
}

void clean_carved_path() {

    for (int row = 0; row < HEIGHT; row++) {

        if (CARVED_PATH[row] < WIDTH) {
            calculate_pixel_energy(CARVED_PATH[row], row);
        }

        if (CARVED_PATH[row] > 0) {
            calculate_pixel_energy(CARVED_PATH[row] - 1, row);
        }

        if (CARVED_PATH[row] + 1 < WIDTH) {
            calculate_pixel_energy(CARVED_PATH[row] + 1, row);
        }
    }
}

void eliminate_path(int cheapest_col, int cheapest_base_height) {

    int cheapest_col_up = NEXT_STEP_MEMORY[cheapest_base_height][cheapest_col];
    for (int row = cheapest_base_height; row < HEIGHT; row++) {
        remove_pixel(cheapest_col, row);
    }

    for (int row = cheapest_base_height - 1; row >= 0; row--) {
        remove_pixel(cheapest_col_up, row);
    }

    WIDTH--;

    clean_carved_path();
}

int calculate_next_width() {

    if (WIDTH_PERCENTAGE_TO_REDUCE <= 0 || WIDTH_PERCENTAGE_TO_REDUCE >= 100) {
        cout << "Error: Invalid width percentage to reduce." << endl;

        exit(0);
    }

    return static_cast<int>(WIDTH - (WIDTH * (WIDTH_PERCENTAGE_TO_REDUCE / 100)));
}

void carve_image_width(const int search_depth) {

    int next_width = calculate_next_width();

    int path_energy, cheapest_col, cheapest_path_energy, cheapest_base_height;

    CARVED_PATH = vector<int>(HEIGHT);

    int temp_search_depth, base_height, start_col;
    int search_height_chunks = HEIGHT / (search_depth + 1);
    bool first_search;

    while (WIDTH > next_width) {
        first_search = true;

        temp_search_depth = search_depth;

        while (temp_search_depth > 0) {
            base_height = search_height_chunks * temp_search_depth;

            PATH_ENERGY_MEMORY = vector<vector<int>>(
                    HEIGHT, vector<int>(WIDTH, NULL_INT_VALUE)
            );

            NEXT_STEP_MEMORY = vector<vector<int>>(
                    HEIGHT, vector<int>(WIDTH, NULL_INT_VALUE)
            );

            start_col = 0;
            if (first_search) {
                cheapest_col = 0,
                cheapest_base_height = base_height,
                cheapest_path_energy = calculate_path_energy(0, base_height);
                start_col = 1;
                first_search = false;
            }

            for (int col = start_col; col < WIDTH; col++) {
                path_energy = calculate_path_energy(col, base_height);

                if (path_energy < cheapest_path_energy) {
                    cheapest_path_energy = path_energy;
                    cheapest_col = col;
                    cheapest_base_height = base_height;
                }
            }

            temp_search_depth--;
        }

        eliminate_path(cheapest_col, cheapest_base_height);
    }
}

void load_pixels_from_3d_to_1d() {

    OUTPUT_IMAGE = new uint8_t[WIDTH * HEIGHT * COLOR_PROPERTY];

    int index = 0;

    for (int row = 0; row < HEIGHT; row++) {
        for (int col = 0; col < WIDTH; col++) {
            for (int px = 0; px < COLOR_PROPERTY; px++) {
                OUTPUT_IMAGE[index++] = PIXELS[row][col][px];
            }
        }
    }
}

void load_pixels_from_2d_to_1d() {

    OUTPUT_IMAGE = new uint8_t[WIDTH * HEIGHT];

    int index = 0;

    for (int row = 0; row < HEIGHT; row++) {
        for (int col = 0; col < WIDTH; col++) {
            OUTPUT_IMAGE[index++] = PIXELS_ENERGY[row][col];
        }
    }
}

void generate_energy_map() {

    PIXELS_ENERGY = vector<vector<int>>(
            HEIGHT, vector<int>(WIDTH)
    );

    for (int row = 0; row < HEIGHT; row++) {
        for (int col = 0; col < WIDTH; col++) {
            calculate_pixel_energy(col, row);
        }
    }
}

void load_pixels_from_1d_to_3d() {

    PIXELS = vector<vector<vector<int>>>(
            HEIGHT,
            vector<vector<int>>(
                    WIDTH, vector<int>(COLOR_PROPERTY)
            )
    );

    int index = 0;

    for (int row = 0; row < HEIGHT; row++) {
        for (int col = 0; col < WIDTH; col++) {
            for (int px = 0; px < COLOR_PROPERTY; px++) {
                PIXELS[row][col][px] = IMAGE[index++];
            }
        }
    }
}

void seam_carve(const int i, const string& image_extension, const int search_depth = 1)
{
    cout << i << " => ";
    load_image(generate_filename(i, image_extension), WIDTH, HEIGHT, COLOR_PROPERTY);

    load_pixels_from_1d_to_3d();

    generate_energy_map();

    load_pixels_from_2d_to_1d();
    write_image(generate_filename(i, image_extension, "energy").data(), WIDTH, HEIGHT, 1);

    carve_image_width(search_depth);

    load_pixels_from_3d_to_1d();
    write_image(generate_filename(i, image_extension, "carved").data(), WIDTH, HEIGHT, COLOR_PROPERTY);
}

int main()
{
    WIDTH_PERCENTAGE_TO_REDUCE = 25;

    seam_carve(0, JPG_EXTENSION);
    seam_carve(1, JPG_EXTENSION);
    seam_carve(2, JPG_EXTENSION);
    seam_carve(3, JPG_EXTENSION);
    seam_carve(4, JPG_EXTENSION);
    seam_carve(5, JPG_EXTENSION);
    seam_carve(6, JPG_EXTENSION);
    seam_carve(7, JPG_EXTENSION);
    seam_carve(8, JPG_EXTENSION);
    seam_carve(9, JPG_EXTENSION);
    seam_carve(10, JPG_EXTENSION);
    seam_carve(11, JPG_EXTENSION);
    seam_carve(12, JPG_EXTENSION);

    return 0;
}