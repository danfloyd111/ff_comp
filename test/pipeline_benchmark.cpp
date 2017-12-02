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

// helper function (definitions are at the bottom of this file)
double sequentializer(double, unsigned long, std::function<double(double)>);

int main() {

    const size_t DATA_SIZE = 6553600;
    const size_t CORES_NUM = 8;
    const unsigned long RUNS = 1000;

    // creating an universal random big data set for the benchmark (~50MB) 

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
        auto tmp = sequentializer(data_set[i], RUNS, static_cast<double(*)(double)>(sin));  // stage 1
        tmp = sequentializer(tmp, RUNS, static_cast<double(*)(double)>(cos));               // stage 2
        tmp = sequentializer(tmp, RUNS, static_cast<double(*)(double)>(sin));               // stage 3
        tmp = sequentializer(tmp, RUNS, static_cast<double(*)(double)>(atan));              // stage 4
        tmp = sequentializer(tmp, RUNS, static_cast<double(*)(double)>(sin));               // stage 5
        tmp = sequentializer(tmp, RUNS, static_cast<double(*)(double)>(cos));               // stage 6
        tmp = sequentializer(tmp, RUNS, static_cast<double(*)(double)>(sin));               // stage 7
        seq_result_set.push_back(tmp);
    }
    seq_end = chrono::system_clock::now();
    cout << "Done! [Elapsed time: " << ((std::chrono::duration<double, std::milli>) (seq_end-seq_start)).count() << "(ms)]" << endl;

    return EXIT_SUCCESS;

}


// definition of the helper functions

double sequentializer (double input, unsigned long runs, std::function<double(double)> fun) {
    for (auto i=0; i<runs; ++i) input = fun(input);
    return input;
}