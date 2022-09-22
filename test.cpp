#include <iostream>
#include <vector>
#include <cmath>

using namespace std;


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
    for (const auto &i: X) {
        for (auto j: i) {
            std::cout << j << std::endl;
        }
    }
    double ans = 0.0;
    for (int i = 0; i <= k; ++i) {
        ans += pow(x, k - i) * X[i][0];
    }
    cout << ans << endl;
    return ans;
}


int main() {
    // Your code here!
    vector<pair<float, double>> test_data;
    test_data.resize(20);
    for (int i = 0; i < test_data.size(); i++) {
        test_data[i].first = i;
        test_data[i].second = i * i * 1.5 - 5 * i + 4;
    }
    lsm_ext(test_data, 2, 40);
}
