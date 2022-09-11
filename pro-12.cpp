#include <vector>
#include <iostream>
#include <array>
#include <cmath>
#include <random>

#define GNUPLOT_PATH "C:\\Progra~1\\gnuplot\\bin\\gnuplot.exe -persist"
#define PITCH 0.001
#define LENGTH 10
using namespace std;

random_device seed_gen;
mt19937 engine(seed_gen());
uniform_real_distribution<double> distribution(-0.5, 0.5);

int main() {
    array<double, static_cast<size_t>( LENGTH * (1 / PITCH))> source, noise, output, de_noise;
    for (int i = 0; i < source.size(); ++i) {
        source[i] = sin(i * PITCH) * 5;
        noise[i] = distribution(engine);
        output[i] = source[i] + noise[i];


    }
}

