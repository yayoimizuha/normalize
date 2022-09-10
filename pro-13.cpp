#include <iostream>
#include <filesystem>
#include <FreeImagePlus.h>
#include <vector>

using namespace std;
using namespace std::filesystem;

struct resolution {
    int width;
    int height;
};

int main(int argc, char *argv[]) {
    cout << "FreeImage Library Version: " << FreeImage_GetVersion() << endl;
    cout << FreeImage_GetCopyrightMessage() << endl;
    if (argc != 2) {
        cerr << "Invalid argument." << argc << endl;
        return EXIT_FAILURE;
    } else {
        if (exists(argv[1])) {
            cout << "File exist: " << argv[1] << endl;
        } else {
            cerr << "File not exist: " << argv[1] << endl;
            return EXIT_FAILURE;
        }

    }
    FREE_IMAGE_FORMAT FIF = FreeImage_GetFileType(argv[1]);

    FIBITMAP *image = FreeImage_Load(FIF, argv[1]);

    if (!image) {
        throw runtime_error("Load failed!");
    }


    FIBITMAP *image32 = FreeImage_ConvertTo32Bits(image);
    FreeImage_Unload(image);

    //pair<unsigned int, unsigned int> resolution(FreeImage_GetWidth(image32), FreeImage_GetHeight(image32));
    resolution image_resolution = {static_cast<int>(FreeImage_GetWidth(image32)),
                                   static_cast<int>(FreeImage_GetHeight(image32))};
    RGBQUAD col;
    bool isGray = true;
    vector<vector<vector<uint8_t>>> raw_image_array;
    vector<vector<uint8_t>> gray_image_array;
    raw_image_array.resize(image_resolution.height);
    gray_image_array.resize(image_resolution.height);
    //左下から右上
    for (int i = 0; i < image_resolution.height; ++i) {
        raw_image_array[i].resize(image_resolution.width);
        gray_image_array[i].resize(image_resolution.width);
        for (int j = 0; j < image_resolution.width; ++j) {
            FreeImage_GetPixelColor(image32, j, i, &col);
            if (!(col.rgbRed == col.rgbGreen && col.rgbGreen == col.rgbBlue))isGray = false;
            raw_image_array[i][j].resize(3);
            raw_image_array[i][j][0] = col.rgbRed;
            raw_image_array[i][j][1] = col.rgbGreen;
            raw_image_array[i][j][2] = col.rgbBlue;
            gray_image_array[i][j] = col.rgbRed;
        }
    }
    FreeImage_Unload(image32);

    cout << "Image is GrayScale?: " << boolalpha << isGray << endl;
    if (!isGray) {
        for (int i = 0; i < image_resolution.height; ++i) {
            for (int j = 0; j < image_resolution.width; ++j) {
                gray_image_array[i][j] = static_cast<uint8_t>(0.2126 * raw_image_array[i][j][0] +
                                                              0.7152 * raw_image_array[i][j][1] +
                                                              0.0722 * raw_image_array[i][j][2]);
            }
        }
    }
    raw_image_array.clear();
    system("pause");

    cout << image_resolution.width << "x" << image_resolution.height << endl;


}