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
get_range(int location, int width, const vector<double> &input_array);

double lsm(const vector<double> &data);

double lsm_ext(const vector<pair<float, double>> &data, uint8_t k, double x);

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

    double lsm_diff, ave_diff, median_diff, random_diff;

    for (int i = 0; i < static_cast<size_t>(LENGTH / PITCH); ++i) {
        fprintf(gnuplot, "%f\t%f\n", i * PITCH, source[i]);
    }
    fprintf(gnuplot, "e\n");


    for (int i = 0; i < static_cast<size_t>(LENGTH / PITCH); ++i) {
        fprintf(gnuplot, "%f\t%f\n", i * PITCH, noise[i]);
    }
    fprintf(gnuplot, "e\n");

    random_diff = 0.0;
    for (int i = 0; i < static_cast<size_t>(LENGTH / PITCH); ++i) {
        fprintf(gnuplot, "%f\t%f\n", i * PITCH, output[i]);
        random_diff += fabs(output[i] - source[i]);
    }
    fprintf(gnuplot, "e\n");

    lsm_diff = 0.0;
    for (int i = 0; i < static_cast<int>(LENGTH / PITCH); ++i) {
        int x = 2000;
        auto range = get_range(i, x, output);
        vector<pair<float, double>> input_array;
        input_array.resize(range.second.size());
        for (int j = 0; j < range.second.size(); ++j) {
            input_array[j].first = static_cast<float>(j);
            input_array[j].second = range.second[j];
        }
        if (i < x)x = i;
        if ((LENGTH / PITCH) < (i + x)) {
            x = (x * 2 + i - static_cast<int>(LENGTH / PITCH));
        }
        de_noise[i] = lsm_ext(input_array, 11, x);
        lsm_diff += fabs(de_noise[i] - source[i]);
    }
    for (int i = 0; i < static_cast<size_t>(LENGTH / PITCH); ++i) {
        fprintf(gnuplot, "%f\t%f\n", i * PITCH, de_noise[i]);
    }
    fprintf(gnuplot, "e\n");


    ave_diff = 0.0;
    for (int i = 0; i < static_cast<int>(LENGTH / PITCH); ++i) {
        auto range = get_range(i, 40, output);
        auto average = accumulate(range.second.begin(), range.second.end(), 0.0) / range.first;
        de_noise[i] = average;
        ave_diff += fabs(de_noise[i] - source[i]);
    }
    for (int i = 0; i < static_cast<size_t>(LENGTH / PITCH); ++i) {
        fprintf(gnuplot, "%f\t%f\n", i * PITCH, de_noise[i]);
    }
    fprintf(gnuplot, "e\n");


    median_diff = 0.0;
    for (int i = 0; i < static_cast<int>(LENGTH / PITCH); ++i) {
        auto range = get_range(i, 40, output);
        sort(range.second.begin(), range.second.end());
        auto median = range.second[(range.first - 1) / 2];
        de_noise[i] = median;
        median_diff += fabs(de_noise[i] - source[i]);
    }
    for (int i = 0; i < static_cast<size_t>(LENGTH / PITCH); ++i) {
        fprintf(gnuplot, "%f\t%f\n", i * PITCH, de_noise[i]);
    }
    fprintf(gnuplot, "e\n");


    pclose(gnuplot);
    cout << "noise diff:" << random_diff << endl;
    cout << "LSM diff:" << lsm_diff << endl;
    cout << "average diff:" << ave_diff << endl;
    cout << "median diff:" << median_diff << endl;
}

pair<unsigned int, vector<double>>
get_range(int location, int width, const vector<double> &input_array) {
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

vector<vector<double>> Transpose_array(const vector<vector<double>> &input_array) {
    auto array_length = input_array[0].size();
    for (const vector<double> &row: input_array) {
        if (array_length != row.size()) {
            cout << "Array is not square" << endl;
            return {{-1}};
        }
    }
    vector<vector<double>> return_array;
    return_array.resize(input_array[0].size());
    for (int i = 0; i < input_array.size(); ++i) {
        for (int j = 0; j < input_array[0].size(); ++j) {
            return_array[j].resize(input_array.size());
            return_array[j][i] = input_array[i][j];
        }
    }
    return return_array;
}

vector<vector<double>> Produce_array(const vector<vector<double>> &arrayA, const vector<vector<double>> &arrayB) {
    if (arrayA[0].size() != arrayB.size()) {
        cout << "Can't define array product." << endl;
    }
    vector<double> vecA;
    vecA.resize(arrayB.size());
    vector<double> vecB;
    vecB.resize(arrayB.size());

    double inner_product;
    vector<vector<double>> arrayC;
    arrayC.resize(arrayA.size());
    for (int i = 0; i < arrayA.size(); ++i) {
        arrayC[i].resize(arrayB[0].size());
        for (int j = 0; j < arrayB[0].size(); ++j) {
            inner_product = 0.0;
            for (int k = 0; k < arrayB.size(); ++k) {
                inner_product += arrayA[i][k] * arrayB[k][j];
            }
            arrayC[i][j] = inner_product;
        }
    }
    return arrayC;
}

vector<vector<double>> Inverse_array(const vector<vector<double>> &input_array) {
    vector<vector<double>> return_array, sweeper;
    double a;
    sweeper.resize(input_array.size());
    for (int i = 0; i < input_array.size(); ++i) {
        sweeper[i].resize(input_array.size() * 2);
        for (int j = 0; j < input_array.size(); ++j) {
            sweeper[i][j] = input_array[i][j];
            sweeper[i][input_array.size() + j] = (i == j) ? 1 : 0;
        }
    }

    for (int k = 0; k < input_array.size(); ++k) {
        double max = fabs(sweeper[k][k]);
        int max_i = k;
        for (int i = k + 1; i < input_array.size(); ++i) {
            if (fabs(sweeper[i][k]) > max) {
                max = sweeper[i][k];
                max_i = i;
            }
        }
        if (fabs(sweeper[max_i][k]) <= 1e-7) {
            return {{-1}};
        }
        if (k != max_i) {
            for (int j = 0; j < input_array.size() * 2; ++j) {
                double tmp = sweeper[max_i][j];
                sweeper[max_i][j] = sweeper[k][j];
                sweeper[k][j] = tmp;
            }
        }

        a = 1 / sweeper[k][k];

        for (int j = 0; j < input_array.size() * 2; ++j) {
            sweeper[k][j] *= a;
        }
        for (int i = 0; i < input_array.size(); ++i) {
            if (i == k)continue;
            a = -sweeper[i][k];
            for (int j = 0; j < input_array.size() * 2; ++j) {
                sweeper[i][j] += sweeper[k][j] * a;
            }
        }
    }
    return_array.resize(input_array.size());
    for (int i = 0; i < input_array.size(); ++i) {
        return_array[i].resize(input_array.size());
        for (int j = 0; j < input_array.size(); ++j) {
            return_array[i][j] = sweeper[i][input_array.size() + j];
        }
    }
    return return_array;
}

double lsm_ext(const vector<pair<float, double>> &data, uint8_t k, double x) {
    vector<vector<double>> A;
    A.resize(data.size());
    for (int i = 0; i < A.size(); ++i) {
        A[i].resize(k + 1);
        for (int j = 0; j <= k; ++j) {
            A[i][j] = pow(data[i].first, k - j);
        }
    }
    vector<vector<double>> B;
    B.resize(data.size());
    for (int i = 0; i < B.size(); ++i) {
        B[i].resize(1);
        B[i][0] = data[i].second;
    }
    auto AT = Transpose_array(A);
    auto ATA = Produce_array(AT, A);
    auto ATA_1 = Inverse_array(ATA);
    auto ATA_1AT = Produce_array(ATA_1, AT);
    auto X = Produce_array(ATA_1AT, B);

    double ans = 0.0;
    for (int i = 0; i <= k; ++i) {
        ans += pow(x, k - i) * X[i][0];
    }
    return ans;
}

