#include <iostream>
#include <filesystem>
#include <FreeImagePlus.h>

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
    const FREE_IMAGE_TYPE src_type = FreeImage_GetImageType(image);

    FIBITMAP *image32 = FreeImage_ConvertTo32Bits(image);

    //pair<unsigned int, unsigned int> resolution(FreeImage_GetWidth(image32), FreeImage_GetHeight(image32));
    resolution image_resolution = {static_cast<int>(FreeImage_GetWidth(image32)),
                                   static_cast<int>(FreeImage_GetHeight(image32))};
    FIBITMAP *grey_image = FreeImage_ConvertToGreyscale(image32);
    RGBQUAD col, g_col;
    int grey;

    //左下から右上
    for (int i = 0; i < image_resolution.height; ++i) {
        for (int j = 0; j < image_resolution.width; ++j) {
            FreeImage_GetPixelColor(image32, j, i, &col);
            cout << i << "\t" << j << endl;
            printf("%#x\t", col.rgbRed);
            printf("%#x\t", col.rgbGreen);
            printf("%#x\n", col.rgbBlue);
            FreeImage_GetPixelColor(grey_image, j, i, &g_col);
            printf("%#x\t", g_col.rgbRed);
            printf("%#x\t", g_col.rgbGreen);
            printf("%#x\n", g_col.rgbBlue);
            printf("\n");
        }
        printf("\n\n");
    }

    cout << FreeImage_GetWidth(image) << "x" << FreeImage_GetHeight(image) << endl;


}