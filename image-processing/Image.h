//
// Created by ilak on 9/22/21.
//

#ifndef NANKAI_COMPUTER_VISION_IMAGE_H
#define NANKAI_COMPUTER_VISION_IMAGE_H


#include <string>
#include <utility>

using namespace std;

class Image {

private:
    string filePath;
    int*** pixels;

public:
    Image(string filePath) {
        this->filePath = std::move(filePath);
    }

    int*** getPixel() {

    }

};


#endif //NANKAI_COMPUTER_VISION_IMAGE_H
