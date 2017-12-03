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

const size_t DATA_SIZE = 6553600; // about 50MB
const size_t CORES_NUM = 8;         // TODO: should make parametric n. of cores
const unsigned long RUNS = 1000;

// Helper functions (definitions are at the bottom of this file)

double sequentializer(double, unsigned long, std::function<double(double)>);

// Pipeline emitter

struct Emitter : public ff_node {
    private:
    const vector<double> in_stream;
    protected:
    unsigned long index;
    int svc_init() {
        index = 0;
        return 0;
    } 
    void *svc(void *t) {
        if (in_stream.empty()) {
            error("Emitter has no input\n");
            return EOS;
        }
        if (index++ >= in_stream.size()) return EOS;
        return new double(in_stream[index]);;
    }
    void svc_end() {
        index = 0;
        return;
    }
    public:
    Emitter(const vector<double>& is) : in_stream(is) { }    
};

// Pipeline collector

struct Collector: public ff_node {
    private:
    vector<double> out_stream;
    protected:
    void *svc(void *t) {
        double val = *((double*)t);
        out_stream.push_back(val);
        delete t;
        return GO_ON;
    }
    public:
    Collector(const vector<double>& os) : out_stream(os) { }    
    const vector<double>& get_output_stream() const {
        return out_stream;
    }
};

// Comp & Pipeline internal stages

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

    uniform_real_distribution<double> dist {0, 2*M_PI};
    default_random_engine engine { };
    auto next_value = bind(dist, engine);
    
    cout << "Generating random data set..." << endl;

    vector<double> data_set;
    data_set.reserve(DATA_SIZE);
    for (size_t i=0; i<DATA_SIZE; ++i) data_set.push_back(next_value());

    cout << "Done!" << endl;
    
    // sequential test
    
    vector<double> seq_result_set;
    seq_result_set.reserve(DATA_SIZE);
    cout << "Running sequential computation..." << endl;
    chrono_start = chrono::system_clock::now();
    for (size_t i=0; i<DATA_SIZE; ++i) {
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

    cout << "Running composed computation..." << endl;

    chrono_start = chrono::system_clock::now();
    for (size_t i=0; i<DATA_SIZE; ++i) {
        double* task = new double(data_set[i]);
        task = (double*) comp.run(task);
        comp_result_set.push_back(double(*task));
        delete task;
    }
    chrono_stop = chrono::system_clock::now();
    auto comp_time = ((std::chrono::duration<double, std::milli>) (chrono_stop - chrono_start)).count();
    cout << "Done! [Elapsed time: " << comp_time << "(ms)]" << endl;

    // pipeline test

    ff_pipeline pipeline;
    vector<double> pipe_result_set;
    Emitter emitter(data_set);
    Collector collector(pipe_result_set);
    pipeline.add_stage(&emitter);
    pipeline.add_stage(&stage2);
    pipeline.add_stage(&stage3);
    pipeline.add_stage(&stage4);
    pipeline.add_stage(&stage5);
    pipeline.add_stage(&stage6);
    pipeline.add_stage(&collector);

    cout << "Running pipelined computation..." << endl;

    chrono_start = chrono::system_clock::now();
    if(pipeline.run_and_wait_end()<0) error("Running pipeline\n");
    chrono_stop = chrono::system_clock::now();

    auto pipe_time = ((std::chrono::duration<double, std::milli>) (chrono_stop - chrono_start)).count();
    cout << "Done! [Elapsed time: " << pipe_time << "(ms)]" << endl;

    // performance evaluation

    double diff;
    if (comp_time > seq_time) diff = comp_time - seq_time; else diff = seq_time - comp_time;
    cout << "-- Performance evaluation --\n";
    cout << "Difference between sequential and comp: " << diff << "(ms)\n";
    cout << "Checking consistency between the result sets...\n";
    size_t i=0;
    bool consistence = true;
    while (i<comp_result_set.size() && i<seq_result_set.size() && consistence) {
        if (comp_result_set[i] != seq_result_set[i]) consistence = false;
        ++i;
    }
    if (consistence) cout << "The results are consistent" << endl; else cout << "The results are NOT consistent" << endl; 

    return EXIT_SUCCESS;

}


// definition of the helper functions

double sequentializer (double input, unsigned long runs, std::function<double(double)> fun) {
    for (size_t i=0; i<runs; ++i) input = fun(input);
    return input;
}