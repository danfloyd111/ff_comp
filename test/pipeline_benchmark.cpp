/*
 *  Author: Daniele Paolini, daniele.paolini@hotmail.it
 * 
 *  This program is a benchmark for the ff_comp construct, it will be compared with a standard ff_pipeline version and a completely sequential version of the same
 *  code. We are going to read a big number of double values and perform several heavy operations on them in order to see if there are any differences between
 *  the completely sequential code and the code that uses ff_comp, obiouvsly we expect to see an huge speedup using the ff_pipeline construct.
 * 
 *  Tested with valgrind http://valgrind.org/info/about.html
 *
*/

#include <iostream>
#include <random>
#include <functional>
#include <vector>
#include <cmath>
#include <chrono>

using namespace std;

// helper functions (definitions are at the bottom of this file)
double seq_sin(double);
double seq_cos(double);
double seq_fun(double);

int main() {

    // creating an universal random big data set for the benchmark (~50MB)
    
    const size_t DATA_SIZE = 6553600; 

    uniform_real_distribution<double> dist {1, 10000};
    default_random_engine engine { };
    auto next_value = bind(dist, engine);
    
    cout << "Generating random data set..." << endl;

    vector<double> data_set;
    data_set.reserve(DATA_SIZE);
    for (auto i=0; i<DATA_SIZE; ++i) data_set.push_back(next_value());

    cout << "Done!" << endl;
    
    // sequential test
    
    vector<double> seq_result_set;
    std::chrono::time_point<std::chrono::system_clock> seq_start;
    std::chrono::time_point<std::chrono::system_clock> seq_end;
    seq_result_set.reserve(DATA_SIZE);
    cout << "Beginning sequential computation..." << endl;
    seq_start = chrono::system_clock::now();
    for (auto i=0; i<DATA_SIZE; ++i) {
        auto tmp = seq_sin(data_set[i]); // stage 1
        tmp = seq_cos(tmp);              // stage 2
        tmp = sqrt(tmp);                 // stage 3
        tmp = seq_fun(tmp);              // stage 4
        tmp = seq_sin(tmp);              // stage 5
        tmp = seq_cos(tmp);              // stage 6
        tmp = atan(tmp*tmp);             // stage 7
        seq_result_set.push_back(tmp);
    }
    seq_end = chrono::system_clock::now();
    cout << "Done! [Elapsed time: " << ((std::chrono::duration<double, std::milli>) (seq_end-seq_start)).count() << "(ms)]" << endl;

    return EXIT_SUCCESS;

}


// definition of the helper functions

double seq_sin(double x) {
    for (auto i=0; i<10; ++i) x = sin(x);
    return x;
}

double seq_cos(double x) {
    for (auto i=0; i<10; ++i) x = cos(x);
    return x;
}

double seq_fun(double x) {
    return log(2*M_PI*x);
}