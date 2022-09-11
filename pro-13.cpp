#include <iostream>
#include <filesystem>
#include <FreeImagePlus.h>
#include <vector>
#include <random>
#include <algorithm>

using namespace std;
using namespace std::filesystem;

#define GNUPLOT_PATH "C:\\Progra~1\\gnuplot\\bin\\gnuplot.exe -persist"
#define NOISE_FREQ 1000

pair<unsigned int, vector<double>>
get_range(int location, short width, vector<unsigned char> input_array);

double lsm(const vector<double> &data);;

random_device seed_gen;
mt19937 engine(seed_gen());
uniform_real_distribution<double> distribution(0, 1.0);

struct resolution {
    int width;
    int height;
};

vector<vector<int>> de_noise(struct resolution image_resolution, const vector<vector<uint8_t>> &input_image);

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
    //system("pause");

    cout << image_resolution.width << "x" << image_resolution.height << endl;

    //ノイズを生成。
    for (int i = 0; i < image_resolution.width * image_resolution.height / NOISE_FREQ; ++i) {
        gray_image_array[distribution(engine) * image_resolution.height]
        [distribution(engine) * image_resolution.width] = distribution(engine) * 100 - 50;
    }

    vector<vector<int>> de_noised_image = de_noise(image_resolution, gray_image_array);

    FILE *gnuplot_1, *gnuplot_2;
    if ((gnuplot_1 = popen(GNUPLOT_PATH, "w")) == nullptr) {
        cerr << "Can't execute gnuplot_1. " << GNUPLOT_PATH << endl;
        return EXIT_FAILURE;
    }
    if ((gnuplot_2 = popen(GNUPLOT_PATH, "w")) == nullptr) {
        cerr << "Can't execute gnuplot_1. " << GNUPLOT_PATH << endl;
        return EXIT_FAILURE;
    }
    fprintf(gnuplot_1, "set terminal windows color\n");
    fprintf(gnuplot_1, "set size ratio -1\n");
    fprintf(gnuplot_1, "set grid\n");
    fprintf(gnuplot_1, "set palette defined (0 'black',255 'white')\n");
    fprintf(gnuplot_1, "plot '-' matrix with image title 'de_noised'\n");
    fprintf(gnuplot_1, "e\n");

    fprintf(gnuplot_2, "set terminal windows color\n");
    fprintf(gnuplot_2, "set size ratio -1\n");
    fprintf(gnuplot_2, "set grid\n");
    fprintf(gnuplot_2, "set palette defined (0 'black',255 'white')\n");
    fprintf(gnuplot_2, "plot '-' matrix with image title 'noise'\n");
    fprintf(gnuplot_2, "e\n");


    for (int i = 0; i < image_resolution.height; ++i) {
        for (int j = 0; j < image_resolution.width; ++j) {
            fprintf(gnuplot_1, "%d\t", de_noised_image[i][j]);
            fflush(gnuplot_1);
            //fprintf(stdout, "%d\t", de_noised_image[i][j]);

        }
        fprintf(gnuplot_1, "\n");
        fflush(gnuplot_1);
        //fprintf(stdout, "\n");
    }
    fprintf(gnuplot_1, "e\n");
    pclose(gnuplot_1);


    for (int i = 0; i < image_resolution.height; ++i) {
        for (int j = 0; j < image_resolution.width; ++j) {
            fprintf(gnuplot_2, "%d\t", gray_image_array[i][j]);
            fflush(gnuplot_2);
            //fprintf(stdout, "%d\t", de_noised_image[i][j]);

        }
        fprintf(gnuplot_2, "\n");
        fflush(gnuplot_2);
        //fprintf(stdout, "\n");
    }
    fprintf(gnuplot_2, "e\n");

    pclose(gnuplot_2);


}

vector<vector<int>> de_noise(struct resolution image_resolution, const vector<vector<uint8_t>> &input_image) {
    uint8_t color, k,near_end;
    k = 10;
    near_end=20;
    vector<int> near_list;
    vector<vector<int>> return_image;
    return_image.resize(image_resolution.height);
    for (int i = 0; i < image_resolution.height; ++i) {
        return_image[i].resize(image_resolution.width);
        for (int j = 0; j < image_resolution.width; ++j) {
            color = input_image[i][j];
            near_list.clear();

            int height_start, height_end;
            height_start = i - near_end;
            height_end = i + near_end;
            if (height_start < 0)height_start = 0;
            if (height_end > image_resolution.height)height_end = image_resolution.height;

            for (int l = height_start; l < height_end; ++l) {

                int width_start, width_end;
                width_start = i - near_end;
                width_end = i + near_end;
                if (width_start < 0)width_start = 0;
                if (width_end > image_resolution.width)width_end = image_resolution.width;

                for (int m = 0; m < image_resolution.width; ++m) {
                    if (abs(input_image[l][m] - color) < k)
                        near_list.emplace_back((i - l) * (i - l) + (j - m) * (j - m));
                }
            }
            sort(near_list.begin(), near_list.end());
            int o = 0;
            int sum = 0;
            for (int x: near_list) {
                sum += x;
                o++;
                if (o == 6)break;
            }
            //return_image[i][j] = sum;
            if (sum > 400) {
                cout << i << "\t" << j << "\t" << sum << endl;
                return_image[i][j] = get_range(j, 4, input_image[i]).second[5];
            } else {
                return_image[i][j] = input_image[i][j];
            }

        }
    }
    return return_image;


}


pair<unsigned int, vector<double>>
get_range(int location, short width, vector<unsigned char> input_array) {
    vector<double> return_array;
    int mode;
    return_array.resize(width * 2 + 1);
    if ((location - width) < 0) {
        mode = 0;
        for (int i = 0; i <= width * 2; ++i) {
            return_array[i] = input_array[i];
        }
    } else if ((location + width) > input_array.size()) {
        mode = 1;
        int j = 0;
        for (int i = static_cast<int>(input_array.size()) - (width * 2 + 1); i < input_array.size(); ++i) {
            return_array[j] = input_array[i];
            j++;
        }
    } else {
        mode = 2;
        int j = 0;
        for (int i = static_cast<int>(location - width); i <= location + width; ++i) {
            return_array[j] = input_array[i];
            j++;
        }
    }
    return make_pair(width * 2 + 1, return_array);

}


double lsm(const vector<double> &data) {

    auto length = data.size();
    vector<int> x_array;
    x_array.resize(length);
    iota(x_array.begin(), x_array.end(), 1);
    double a0, a1, A00, A01, A02, A11, A12, d;
    A00 = A01 = A02 = A11 = A12 = 0;
    for (int i = 0; i < length; ++i) {
        if (i == (length - 1) / 2)continue;
        A00 += 1.0;
        A01 += x_array[i];
        A02 += data[i];
        A11 += x_array[i] * x_array[i];
        A12 += x_array[i] * data[i];
    }
    d = A11 * A00 - A01 * A01;
    a0 = (A11 * A02 - A12 * A01) / d;
    a1 = (A12 * A00 - A01 * A02) / d;

    return a1 * static_cast<int>(length / 2) + a0;

}