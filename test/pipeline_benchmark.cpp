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
#include "../comp.hpp"

using namespace std;
using namespace ff;

const size_t DATA_SIZE = 6553600;
const size_t CORES_NUM = 8;         // TODO: should make parametric n. of cores
const unsigned long RUNS = 1000;

// Helper functions (definitions are at the bottom of this file)

double sequentializer(double, unsigned long, std::function<double(double)>);

// Comp stages

struct SinStage : public ff_node {
    void *svc(void *t) {
        double val = *((double*)t);
        *((double*)t) = sequentializer(val, RUNS, static_cast<double(*)(double)>(sin));
        return t;
    }
};

struct CosStage : public ff_node {
    void *svc(void *t) {
        double val = *((double*)t);
        *((double*)t) = sequentializer(val, RUNS, static_cast<double(*)(double)>(cos));
        return t;
    }
};

struct AtaStage : public ff_node {
    void *svc(void *t) {
        double val = *((double*)t);
        *((double*)t) = sequentializer(val, RUNS, static_cast<double(*)(double)>(atan));
        return t;
    }
};

int main() {

    std::chrono::time_point<std::chrono::system_clock> chrono_start;
    std::chrono::time_point<std::chrono::system_clock> chrono_stop;

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
    seq_result_set.reserve(DATA_SIZE);
    cout << "Beginning sequential computation..." << endl;
    chrono_start = chrono::system_clock::now();
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
    chrono_stop = chrono::system_clock::now();
    auto seq_time = ((std::chrono::duration<double, std::milli>) (chrono_stop - chrono_start)).count();
    cout << "Done! [Elapsed time: " << seq_time << "(ms)]" << endl;

    // comp test

    ff_comp comp;
    SinStage stage1, stage3, stage5, stage7;
    CosStage stage2, stage6;
    AtaStage stage4;
    vector<double> comp_result_set;
    comp_result_set.reserve(DATA_SIZE);

    comp.add_stage(&stage1);
    comp.add_stage(&stage2);
    comp.add_stage(&stage3);
    comp.add_stage(&stage4);
    comp.add_stage(&stage5);
    comp.add_stage(&stage6);
    comp.add_stage(&stage7);

    cout << "Beginning composed computation..." << endl;

    chrono_start = chrono::system_clock::now();
    for (auto i=0; i<DATA_SIZE; ++i) {
        double* task = new double(data_set[i]);
        task = (double*) comp.run(task);
        comp_result_set.push_back(double(*task));
        delete task;
    }
    chrono_stop = chrono::system_clock::now();
    auto comp_time = ((std::chrono::duration<double, std::milli>) (chrono_stop - chrono_start)).count();
    cout << "Done! [Elapsed time: " << comp_time << "(ms)]" << endl;

    double diff;
    if (comp_time > seq_time) diff = comp_time - seq_time; else diff = seq_time - comp_time;
    cout << "-- Performance --\n";
    cout << "Difference between sequential and comp: " << diff << "(ms)\n";
    cout << "Checking consistency between the result sets...\n";
    auto i=0;
    bool go_on = true;
    while (i<comp_result_set.size() && i<seq_result_set.size() && go_on) {
        if (comp_result_set[i] != seq_result_set[i]) go_on = false;
        ++i;
    }
    if (!go_on) cout << "The results are consistent" << endl; else cout << "The results are NOT consistent" << endl; 

    return EXIT_SUCCESS;

}


// definition of the helper functions

double sequentializer (double input, unsigned long runs, std::function<double(double)> fun) {
    for (auto i=0; i<runs; ++i) input = fun(input);
    return input;
}