#include <vector>
#include <iostream>
#include <array>
#include <cmath>
#include <random>
#include <algorithm>

#define GNUPLOT_PATH "C:\\Progra~1\\gnuplot\\bin\\gnuplot.exe -persist"
#define PITCH 0.001
#define LENGTH 10
using namespace std;

random_device seed_gen;
mt19937 engine(seed_gen());
uniform_real_distribution<double> distribution(-0.15, 0.15);

pair<unsigned int, vector<double>>
get_range(int location, short width, vector<unsigned char> input_array);

double lsm(const vector<double> &data);

int main() {
    vector<double> source, noise, output, de_noise;
    source.resize(LENGTH / PITCH);
    noise.resize(LENGTH / PITCH);
    output.resize(LENGTH / PITCH);
    de_noise.resize(LENGTH / PITCH);

    for (int i = 0; i < source.size(); ++i) {
        source[i] = sin(i * PITCH) * 5;
        noise[i] = distribution(engine);
        output[i] = source[i] + noise[i];
    }
    FILE *gnuplot;
    if ((gnuplot = popen(GNUPLOT_PATH, "w")) == nullptr) {
        cerr << "Can't execute gnuplot. " << GNUPLOT_PATH << endl;
        return EXIT_FAILURE;
    }
    fprintf(gnuplot, "set terminal windows color\n");
    fprintf(gnuplot, "set grid\n");
    fprintf(gnuplot, "set size square\n");
    fprintf(gnuplot, "plot '-' using 1:2 w l title 'source sin(x)' lc 'yellow' lw 20,"
                     "'-' using 1:2 w l title 'random noise',"
                     "'-' using 1:2 w l title 'source + noise' lc rgb 0xaa000044,"
                     "'-' using 1:2 w l title 'de-noised lsm' lc 'red',"
                     "'-' using 1:2 w l title 'de-noised average' lc 'blue',"
                     "'-' using 1:2 w l title 'de-noised median' lc 'green'\n");


    for (int i = 0; i < static_cast<size_t>(LENGTH / PITCH); ++i) {
        fprintf(gnuplot, "%f\t%f\n", i * PITCH, source[i]);
    }
    fprintf(gnuplot, "e\n");


    for (int i = 0; i < static_cast<size_t>(LENGTH / PITCH); ++i) {
        fprintf(gnuplot, "%f\t%f\n", i * PITCH, noise[i]);
    }
    fprintf(gnuplot, "e\n");


    for (int i = 0; i < static_cast<size_t>(LENGTH / PITCH); ++i) {
        fprintf(gnuplot, "%f\t%f\n", i * PITCH, output[i]);
    }
    fprintf(gnuplot, "e\n");


    for (int i = 0; i < static_cast<int>(LENGTH / PITCH); ++i) {
        auto range = get_range(i, 40, output);
        de_noise[i] = lsm(range.second);
    }
    for (int i = 0; i < static_cast<size_t>(LENGTH / PITCH); ++i) {
        fprintf(gnuplot, "%f\t%f\n", i * PITCH, de_noise[i]);
    }
    fprintf(gnuplot, "e\n");


    for (int i = 0; i < static_cast<int>(LENGTH / PITCH); ++i) {
        auto range = get_range(i, 40, output);
        auto average = accumulate(range.second.begin(), range.second.end(), 0.0) / range.first;
        de_noise[i] = average;
    }
    for (int i = 0; i < static_cast<size_t>(LENGTH / PITCH); ++i) {
        fprintf(gnuplot, "%f\t%f\n", i * PITCH, de_noise[i]);
    }
    fprintf(gnuplot, "e\n");


    for (int i = 0; i < static_cast<int>(LENGTH / PITCH); ++i) {
        auto range = get_range(i, 40, output);
        sort(range.second.begin(), range.second.end());
        auto median = range.second[(range.first - 1) / 2];
        de_noise[i] = median;
    }
    for (int i = 0; i < static_cast<size_t>(LENGTH / PITCH); ++i) {
        fprintf(gnuplot, "%f\t%f\n", i * PITCH, de_noise[i]);
    }
    fprintf(gnuplot, "e\n");


    pclose(gnuplot);
}

pair<unsigned int, vector<double>>
get_range(int location, short width, const vector<double> &input_array) {
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