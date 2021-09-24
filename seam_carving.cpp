//
// Created by ilak on 9/24/21.
//

#include <iostream>
#include <vector>
#include "image_processor.cpp"

using namespace std;

const int color_property = 3;

int seam_carve(const std::string& filename)
{
    int width, height;

    vector<unsigned char> image;

    bool success = load_image(image, filename, width, height, color_property);

    if (!success)
    {
        cout << "Error loading image." << endl;
        return 1;
    }

    cout << "Width = " << width << endl;
    cout << "Height = " << height << endl;

    vector<vector<vector<unsigned int>>> pixels(
            height,
            vector<vector<unsigned int>>(
                    width, vector<unsigned int>(color_property)
            )
    );

    int index = 0;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            for (int px = 0; px < color_property; px++) {
                pixels[y][x][px] = static_cast<int>(image[index++]);
            }
        }
    }

    cout << "Width: " << pixels.size() << endl;
    cout << "Height: " << pixels[0].size() << endl;
    cout << "Pixels: " << pixels[0][0].size() << endl;

    return 0;
}