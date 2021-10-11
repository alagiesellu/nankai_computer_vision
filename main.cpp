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

#define UNDEFINED_VALUE -1
#define COLOR_PROPERTY 3

int WIDTH, HEIGHT;

void
load_pixels(vector<int>& pixels, vector<uint8_t>& image) {

    pixels = vector<int>(image.size());

    for (int i = 0; i < image.size(); i++) {
        pixels[i] = static_cast<int>(image[i]);
    }
}

// Loads as RGBA... even if file is only RGB
// Feel free to adjust this if you so please, by changing the 4 to a 0.
void
write_image(uint8_t* image, char const *filename, int& width, int& height, const int& req_comp)
{
    stbi_write_jpg(
            filename,
            width, height, req_comp, image, 100
    );
}

// Loads as RGBA... even if file is only RGB
// Feel free to adjust this if you so please, by changing the 4 to a 0.
void
load_image(vector<int>& image_pixels, const string& filename, int& width, int& height, const int& req_comp)
{
    int n;
    uint8_t* data = stbi_load(filename.c_str(), &width, &height, &n, req_comp);
    vector<uint8_t> image_input;

    if (data != nullptr) {
        image_input = vector<uint8_t>(data, data + width * height * req_comp);
    }

    stbi_image_free(data);

    if (data == nullptr)
    {
        cout << "Error loading image." << endl;
    }

    load_pixels(image_pixels, image_input);
}

void
output_image_from_energy(vector<int>& energy_map, uint8_t*& output_pixels) {

    output_pixels = new uint8_t[energy_map.size()];

    for (int i = 0; i < energy_map.size(); i++) {
        output_pixels[i] = energy_map[i];
    }
}

string
generate_filename(int i, const string& type) {

    string name = to_string(i);

    if (! type.empty())
        name = type + "/" + name;

    return "/home/ilak/Documents/GitHub/nankai-computer-vision/assets/" + name + ".jpg";
}

int calculate_index(int row, int col, int color, int depth) {
    return (row * WIDTH * depth) + (col * depth) + color;
}

int calculate_multi_dimension_index(int row, int col, int color) {
    return calculate_index(row, col, color, COLOR_PROPERTY);
}

int calculate_uni_dimension_index(int row, int col) {
    return calculate_index(row, col,0, 1);
}

int calculate_energy(vector<int>& image_pixels, int& row1, int& col1, int row2, int col2) {
    int energy = 0;

    for (int i = 0; i < COLOR_PROPERTY; i++) {
        energy += abs(
                (int)(
                        image_pixels[calculate_multi_dimension_index(row1, col1, i)]
                        -
                        image_pixels[calculate_multi_dimension_index(row2, col2, i)]
                        )
                );
    }

    return (energy / COLOR_PROPERTY);
}

int calculate_energy_of_pixel(vector<int>& input_pixels, int row, int col) {

    int energy = 0;

    if (row > 0) {
        energy += calculate_energy(input_pixels, row, col, row-1, col);
    }

    if (row < HEIGHT - 1) {
        energy += calculate_energy(input_pixels, row, col, row+1, col);
    }

    if (col > 0) {
        energy += calculate_energy(input_pixels, row, col, row, col-1);
    }

    if (col < WIDTH - 1) {
        energy += calculate_energy(input_pixels, row, col, row, col+1);
    }

    return energy;
}

void calculate_energy_map(vector<int>& input_pixels, vector<int>& energy_map) {

    energy_map = vector<int>(WIDTH * HEIGHT);

    for (int row = 0; row < HEIGHT; row++) {
        for (int col = 0; col < WIDTH; col++) {
            energy_map[calculate_uni_dimension_index(row, col)] = calculate_energy_of_pixel(input_pixels, row, col);
        }
    }
}

void
get_three_column_positions(
        vector<int>& three_neighbors,
        vector<int>& input_pixels,
        int row,
        int col
        ) {

    int col_pos = col;

    for (int i = 0; i < 2; i++) {

        while (
                col_pos >= 0
                &&
                input_pixels[calculate_multi_dimension_index(row, col_pos, 0)] == UNDEFINED_VALUE
                )
            col_pos--;

        three_neighbors[i] = static_cast<int>(col_pos);
        col_pos--;
    }

    col_pos = three_neighbors[0] + 1;

    while (
            col_pos < WIDTH
            &&
            input_pixels[calculate_multi_dimension_index(row, col_pos, 0)] == UNDEFINED_VALUE
            )
        col_pos++;

    three_neighbors[2] = static_cast<int>(col_pos);
}

int
find_path(
        vector<int>& input_pixels,
        vector<int>& energy_map,
        vector<vector<int>>& next_step_column,
        int row,
        int col
) {

    if (input_pixels[calculate_multi_dimension_index(row, col, 0)] == UNDEFINED_VALUE)
        return -1;

    int next_row = row + 1;
    int lowest_col, lowest_energy;

    if (next_row >= HEIGHT)
        return 0;

    if (! next_step_column[row][col])
    {
        vector<int> three_neighbors = vector<int>(3);
        get_three_column_positions(three_neighbors, input_pixels, row, col);

        lowest_col = three_neighbors[0];
        lowest_energy = energy_map[calculate_uni_dimension_index(row, lowest_col)];

        if (three_neighbors[1] != UNDEFINED_VALUE && energy_map[calculate_uni_dimension_index(row, three_neighbors[1])] < lowest_energy) {
            lowest_col = three_neighbors[1];
            lowest_energy = static_cast<int>(energy_map[calculate_uni_dimension_index(row, lowest_col)]);
        }

        if (three_neighbors[2] != UNDEFINED_VALUE && energy_map[calculate_uni_dimension_index(row, three_neighbors[2])] < lowest_energy)
            lowest_col = three_neighbors[2];

        next_step_column[row][col] = lowest_col;
    }

    return
            calculate_energy(input_pixels, row, col, next_row, next_step_column[row][col])
            +
            find_path(input_pixels, energy_map, next_step_column, next_row, next_step_column[row][col]);
}


void
eliminate(vector<int>& input_pixels, vector<vector<int>>& next_step_column, int col) {

    for (int row = 0; row < HEIGHT; row++) {
        for (int i = 0; i < COLOR_PROPERTY; ++i) {
            input_pixels[calculate_multi_dimension_index(row, col, i)] = UNDEFINED_VALUE;
        }
        col = next_step_column[row][col];
    }
}

void
eliminate_column(
        vector<int>& input_pixels,
        vector<int>& energy_map,
        vector<vector<int>>& next_step_column,
        vector<int>& columns_weight
        ) {
    int lowest_column = 0, path_weight;
    int lowest_column_weight = find_path(input_pixels, energy_map, next_step_column, 0, lowest_column);

    for (int col = 1; col < WIDTH; col++) {
        path_weight = find_path(input_pixels, energy_map, next_step_column, 0, col);
        if (
                lowest_column_weight == UNDEFINED_VALUE
                ||
                (path_weight != UNDEFINED_VALUE && path_weight < lowest_column_weight)
        ) {
            lowest_column = col;
            lowest_column_weight = path_weight;
        }
    }

    eliminate(input_pixels, next_step_column, lowest_column);
}

void
seam_carve(vector<int>& input_pixels, vector<int>& energy_map, int to_eliminate)
{
    vector<vector<int>> next_step_column = vector<vector<int>>(HEIGHT,vector<int>(WIDTH));
    vector<int> columns_weight = vector<int>(WIDTH);

    cout << "To Eliminate:" << to_eliminate << endl;

    int eliminated_columns = 0;
    while (eliminated_columns < to_eliminate) {
        eliminate_column(input_pixels, energy_map, next_step_column, columns_weight);
//        cout << eliminated_columns << " - ";
        eliminated_columns++;
    }
    next_step_column.clear();
//    cout << "Eliminated:" << eliminated_columns << endl;
}

void
output_image_from_pixels(vector<int>& input_pixels, uint8_t*& output_pixels, int width, int height) {

    output_pixels = new uint8_t[width * height * COLOR_PROPERTY];

    cout << "Length: " << (width * height * COLOR_PROPERTY) << endl;

    int pixel_counter = 0;

    for (int i = 0; i < input_pixels.size(); i++) {
//        cout << input_pixels[i] << " ";
        if (input_pixels[i] != UNDEFINED_VALUE) {
            pixel_counter++;
//            output_pixels[pixel_counter++] = static_cast<int>(input_pixels[i]);
        }
    }
    cout << "Length: " << (pixel_counter) << endl;
}

int
seam_carve_width(const int i)
{
    vector<int> input_pixels;
    vector<int> energy_map;
    uint8_t* output_pixels;

    load_image(input_pixels, generate_filename(i, ""), WIDTH, HEIGHT, COLOR_PROPERTY);
    cout << "WIDTH:" << WIDTH << " HEIGHT:" << HEIGHT << endl;

    calculate_energy_map(input_pixels, energy_map);

    output_image_from_energy(energy_map, output_pixels);

    write_image(output_pixels, generate_filename(i, "energy").data(), WIDTH, HEIGHT, 1);

    int to_eliminate = (WIDTH - HEIGHT);
    int new_width = WIDTH - to_eliminate;

    seam_carve(input_pixels, energy_map, to_eliminate);

    energy_map.clear();

    output_image_from_pixels(input_pixels, output_pixels, new_width, HEIGHT);

    input_pixels.clear();

    write_image(
            output_pixels,
            generate_filename(i, "carved").data(),
            HEIGHT,
            HEIGHT,
            COLOR_PROPERTY
            );

    return 0;
}

int
test_code() {
    vector<int> energy_map = vector<int>(10);

    for (int i : energy_map) {
        if (i)
            cout << i;
    }

    return 0;
}

int
main()
{
//    return test_code();

    const int pictures = 1;
    for (int i = 0; i < pictures; i++) {
        seam_carve_width(i);
    }

    return 0;
}