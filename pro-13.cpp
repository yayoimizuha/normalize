#pragma clang diagnostic push
#pragma ide diagnostic ignored "openmp-use-default-none"

#include <iostream>
#include <filesystem>
#include <FreeImagePlus.h>
#include <vector>
#include <random>
#include <algorithm>
#include <omp.h>
#include <array>
#include <numeric>

using namespace std;
using namespace filesystem;
using namespace chrono;

#define GNUPLOT_PATH "C:\\Progra~1\\gnuplot\\bin\\gnuplot.exe -persist"
#define NOISE_FREQ 1000

pair<unsigned int, vector<double>>
get_range(int location, short width, vector<unsigned char> input_array);

double lsm(const vector<double> &data);

long long int_pow(long long x, unsigned short y);

double lsm_ext(const vector<pair<float, double>> &data, uint8_t k, double x);


random_device seed_gen;
mt19937 engine(seed_gen());
uniform_real_distribution<double> distribution(0, 1.0);

struct resolution {
    int width;
    int height;
};

auto start_time = system_clock::now();

vector<vector<int>> de_noise(struct resolution image_resolution, const vector<vector<uint8_t>> &input_image);

int main(int argc, char *argv[]) {
    FreeImage_Initialise();
    cout << "FreeImage Library Version: " << FreeImage_GetVersion() << endl;
    cout << FreeImage_GetCopyrightMessage() << endl;

    cout << '[' << duration_cast<milliseconds>(system_clock::now() - start_time).count() << "ms+"
         << duration_cast<milliseconds>(system_clock::now() - start_time).count() << "ms\t] -- Program initialised."
         << endl;
    auto initialised_time = system_clock::now();

    string argv1;
    setlocale(LC_ALL, "");
    auto *wc = (wchar_t *) calloc(_mbstrlen(argv[1]) + 1, MB_LEN_MAX);
    if (argc != 2) {
        cerr << "Invalid argument." << argc << endl;
        return EXIT_FAILURE;
    } else {
        argv1 = argv[1];
        mbstowcs(wc, argv[1], _mbstrlen(argv[1]));
        if (exists(wc)) {
            cout << "File exist: " + argv1 << endl;
        } else {
            cerr << "File not exist: " + argv1 << endl;
            return EXIT_FAILURE;
        }
    }
    auto filepath = path(argv[1]);
    auto filename = filepath.filename();
    cout << "filename: " << filename.string() << endl;

    FREE_IMAGE_FORMAT FIF = FreeImage_GetFileType(argv[1]);
    FIBITMAP *image = FreeImage_Load(FIF, argv[1]);
    if (!image) {
        throw runtime_error("Load failed!");
    }

    FIBITMAP *image32 = FreeImage_ConvertTo32Bits(image);
    FreeImage_Unload(image);

    resolution image_resolution = {static_cast<int>(FreeImage_GetWidth(image32)),
                                   static_cast<int>(FreeImage_GetHeight(image32))};


    cout << '[' << duration_cast<milliseconds>(system_clock::now() - start_time).count() << "ms+"
         << duration_cast<milliseconds>(system_clock::now() - initialised_time).count()
         << "ms\t] -- Load image finished."
         << endl;
    auto image_loaded_time = system_clock::now();
    RGBQUAD col;
    bool isGray = true;
    vector<vector<vector<uint8_t>>> raw_image_array;
    vector<vector<uint8_t>> gray_image_array;
    raw_image_array.resize(image_resolution.height);
    gray_image_array.resize(image_resolution.height);
    //?????????E??
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

    cout << "image resolution: " << image_resolution.width << "x" << image_resolution.height << endl;

    cout << '[' << duration_cast<milliseconds>(system_clock::now() - start_time).count() << "ms+"
         << duration_cast<milliseconds>(system_clock::now() - image_loaded_time).count()
         << "ms\t] -- Convert to 8bit grayscale array." << endl;
    auto convert2array_time = system_clock::now();


    //?m?C?Y??????
    for (int i = 0; i < image_resolution.width * image_resolution.height / NOISE_FREQ; ++i) {
        gray_image_array[distribution(engine) * image_resolution.height]
        [distribution(engine) * image_resolution.width] = distribution(engine) * 100 - 50;
    }


    cout << '[' << duration_cast<milliseconds>(system_clock::now() - start_time).count() << "ms+"
         << duration_cast<milliseconds>(system_clock::now() - convert2array_time).count()
         << "ms\t] -- Add noise at 1/" << NOISE_FREQ << "." << endl;
    auto added_noise_time = system_clock::now();


    vector<vector<int>> de_noised_image = de_noise(image_resolution, gray_image_array);

    cout << '[' << duration_cast<milliseconds>(system_clock::now() - start_time).count() << "ms+"
         << duration_cast<milliseconds>(system_clock::now() - added_noise_time).count()
         << "ms\t] -- de_noised." << endl;
    auto de_noised_time = system_clock::now();

    FIBITMAP *save_image_de_noised = FreeImage_Allocate(image_resolution.width, image_resolution.height, 24),
            *save_image_noise = FreeImage_Allocate(image_resolution.width, image_resolution.height, 24);

    for (int i = 0; i < image_resolution.height; ++i) {
        for (int j = 0; j < image_resolution.width; ++j) {
            col.rgbRed = col.rgbGreen = col.rgbBlue = de_noised_image[i][j];
            FreeImage_SetPixelColor(save_image_de_noised, j, i, &col);
            col.rgbRed = col.rgbGreen = col.rgbBlue = gray_image_array[i][j];
            FreeImage_SetPixelColor(save_image_noise, j, i, &col);
        }
    }
    string name_save_image_de_noised, name_save_image_noise;
    name_save_image_de_noised = "de_noised_" + filename.replace_extension("").string() + ".png";
    name_save_image_noise = "noised_" + filename.replace_extension("").string() + ".png";
    FreeImage_Save(FIF_PNG, save_image_de_noised, name_save_image_de_noised.c_str(), 0);
    cout << "save: " << name_save_image_de_noised << endl;
    FreeImage_Save(FIF_PNG, save_image_noise, name_save_image_noise.c_str(), 0);
    cout << "save: " << name_save_image_noise << endl;

    cout << '[' << duration_cast<milliseconds>(system_clock::now() - start_time).count() << "ms+"
         << duration_cast<milliseconds>(system_clock::now() - de_noised_time).count()
         << "ms\t] -- Save images." << endl;
    auto saved_time = system_clock::now();

    FreeImage_DeInitialise();
}

vector<vector<int>> de_noise(struct resolution image_resolution, const vector<vector<uint8_t>> &input_image) {
    const uint8_t k = 8;
    const int near_end = 7;
    const uint8_t sum_threshold = 150;
    unsigned long long progress = 0, before_progress = 1;

    vector<vector<int>> return_image;
    int **return_image_c = (int **) malloc(sizeof(int *) * image_resolution.height);
    return_image.resize(image_resolution.height);
    for (int i = 0; i < image_resolution.height; ++i) {
        return_image[i].resize(image_resolution.width);
        return_image_c[i] = (int *) malloc(sizeof(int) * image_resolution.width);
    }

#pragma omp  parallel for shared(progress, before_progress)
    for (int i = 0; i < image_resolution.height; ++i) {
        for (int j = 0; j < image_resolution.width; ++j) {
            auto color = input_image[i][j];
            vector<int> near_list;
            near_list.clear();

            int height_start, height_end;
            height_start = i - near_end;
            height_end = i + near_end;
            if (height_start < 0)height_start = 0;
            if (height_end > image_resolution.height)height_end = image_resolution.height;
            for (int l = height_start; l < height_end; ++l) {

                int width_start, width_end;
                width_start = j - near_end * 2;
                width_end = j + near_end * 2;
                if (width_start < 0)width_start = 0;
                if (width_end > image_resolution.width)width_end = image_resolution.width;
                for (int m = width_start; m < width_end; ++m) {
                    if (abs(input_image[l][m] - color) < k)
                        near_list.emplace_back((i - l) * (i - l) + (j - m) * (j - m));
                }
            }

            sort(near_list.begin(), near_list.end());
            int o = 0;
            int sum = 0;
            bool has_6 = false;
            for (int x: near_list) {
                sum += x;
                o++;
                if (o == 6) {
                    has_6 = true;
                    break;
                }
            }

            int return_num;
            if (sum > sum_threshold || !has_6) {
                auto cont = get_range(j, 3, input_image[i]).second;
                return_num = lsm(cont);
            } else {
                return_num = input_image[i][j];
            }
            progress++;
            before_progress++;
            auto p_percent = static_cast<short>((progress * 100) / (image_resolution.width * image_resolution.height));
            auto bp_percent = static_cast<short>((before_progress * 100) /
                                                 (image_resolution.width * image_resolution.height));
            bool print_flag = false;
            stringstream output_text;
            if (p_percent != bp_percent) {
                output_text << "\r[" << duration_cast<milliseconds>(system_clock::now() - start_time).count() << "ms]|"
                            << string(static_cast<short>(p_percent / 2), '=') << '>'
                            << string(static_cast<short>(49 - p_percent / 2), ' ') << "| "
                            << ((progress * 100) / (image_resolution.width * image_resolution.height)) << "%";
                print_flag = true;
            } else {}
            //if (print_flag)cout << output_text.str();
            return_image_c[i][j] = return_num;
        }
    }
    cout << "\r[" << duration_cast<milliseconds>(system_clock::now() - start_time).count()
         << "ms]|=================================================>| 100%" << endl;
    for (int i = 0; i < image_resolution.height; ++i) {
        for (int j = 0; j < image_resolution.width; ++j) {
            return_image[i][j] = return_image_c[i][j];
        }
    }
    return return_image;
}


pair<unsigned int, vector<double>>
get_range(int location, short width, vector<unsigned char> input_array) {
    vector<double> return_array;
    return_array.resize(width * 2 + 1);
    if ((location - width) < 0) {
        for (int i = 0; i <= width * 2; ++i) {
            return_array[i] = input_array[i];
        }
    } else if ((location + width) > input_array.size()) {
        int j = 0;
        for (int i = static_cast<int>(input_array.size()) - (width * 2 + 1); i < input_array.size(); ++i) {
            return_array[j] = input_array[i];
            j++;
        }
    } else {
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

    return a1 * static_cast<int>((length - 1) / 2) + a0;

}

#pragma clang diagnostic pop