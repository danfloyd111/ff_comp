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
#include <unistd.h>
#include <vector>
#include <cmath>
#include <chrono>
#include <iomanip>
#include "../comp.hpp"
#include <ff/farm.hpp>

using namespace std;
using namespace ff;

size_t DATA_SIZE = 6250000;    // default size is 50MB
size_t CORES_NUM = 7;          // default n. of cores
unsigned long RUNS = 1000;     // default computation grain

// Helper functions (definitions are at the bottom of this file)

double sequentializer(double, unsigned long, std::function<double(double)>);
inline double diff(double a, double b) { double res; (a>b) ? res = (a-b) : res = (b-a); return res; }
inline float diff_perc(double a, double b) { double res; (a>b) ? res = (a-b) / a : res = (b-a) / b; return res*100.0; }

// Farm Worker

struct FarmWorker : public ff_node {
private:
    const size_t runs, n_stages;
protected:
    void *svc(void *t) {
        double temp;
        for (size_t j=0; j<n_stages; ++j) {
            if (j==0) temp = sequentializer(*((double*)t), runs, static_cast<double(*)(double)>(sin)); // first call
            if (j%2==0) temp = sequentializer(temp, runs, static_cast<double(*)(double)>(sin)); // sin call
            else temp = sequentializer(temp, runs, static_cast<double(*)(double)>(cos)); // cos call
        }
        *((double*)t) = temp;
        return t;
    }
public:
    FarmWorker(size_t runs, size_t n_stages) : runs(runs), n_stages(n_stages) { }
};

// Pipeline/Farm emitter

struct Emitter : public ff_node {
private:
    const vector<double> in_stream;
    unsigned long index;
protected:
    int svc_init() {
        index = 0;
        return 0;
    } 
    void *svc(void *) {
        if (in_stream.empty()) {
            error("Emitter has no input\n");
            return EOS;
        }
        auto val = in_stream[index];
        if (index++ >= in_stream.size()) return EOS;
        return new double(sequentializer(val,RUNS,static_cast<double(*)(double)>(sin)));
    }
    void svc_end() {
        index = 0;
        return;
    }
public:
    Emitter(const vector<double>& is) : in_stream(is) { }    
};

// Pipeline/Farm collector

struct Collector: public ff_node {
private:
    vector<double> out_stream;
   const bool odd;
protected:
    void *svc(void *t) {
        double val = *((double*)t);
        if (odd) out_stream.push_back(sequentializer(val,RUNS,static_cast<double(*)(double)>(sin)));
        else out_stream.push_back(sequentializer(val,RUNS,static_cast<double(*)(double)>(cos)));
        delete (double*) t;
        return GO_ON;
    }
public:
    Collector(bool is_odd) : odd(is_odd) { }    
    const vector<double>& get_output_stream() const { return out_stream; }
};

// Comp & Pipeline internal comp_stages

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

int main(int argc, char **argv) {

    // parsing command line options
    
    int param;
    const char *pattern = "hc:r:s:";
    while ((param = getopt(argc, argv, pattern)) != -1) {
        try {
            switch (param) {
            case 'h':
                cout << "Usage: comp_benchmark [-c number of cores] [-r parallelism grain] [-s data set size]" << endl;
                return EXIT_SUCCESS;
            case 'c':
                CORES_NUM = stoi(optarg);
                if (CORES_NUM < 2) {
                    cerr << "Error: number of cores must be greater than one and shouldn't exceed the available cores on your machine" << endl;
                    return EXIT_FAILURE;
                }
                break;
            case 'r':
                RUNS = stoi(optarg);
                if (RUNS < 1) {
                    cerr << "Error: parallelism grain must be greater than zero" << endl;
                    return EXIT_FAILURE;
                }
                break;
            case 's':
                DATA_SIZE = stoi(optarg);
                if (DATA_SIZE < 1) {
                    cerr << "Error: data size must be greater than zero" << endl;
                    return EXIT_FAILURE;
                }
                break;
            case '?':
                if (optopt == 'c' || optopt == 'r' || optopt == 's')
                    cerr << "Error: option -" << optopt << " requires an argument" << endl;
                else if (isprint(optopt))
                    cerr << "Error: unknown option " << optopt << endl;
                else
                    cerr << "Error: unknown option character" << endl;
            default:
                cerr << "Error: parsing command line options" << endl;
                return EXIT_FAILURE;
            }
        } catch (exception &e) {
            cerr << "Error: invalid command line argument\n";
            return EXIT_FAILURE;
        }
    }

    std::chrono::time_point<std::chrono::system_clock> chrono_start;
    std::chrono::time_point<std::chrono::system_clock> chrono_stop;

    // creating an universal random big data set for the benchmark (50MB) 

    uniform_real_distribution<double> dist {0, 2*M_PI};
    default_random_engine engine(random_device{ }());
    auto next_value = bind(dist, engine);
    
    cout << "Generating random data set..." << endl;

    vector<double> data_set;
    data_set.reserve(DATA_SIZE);
    for (size_t i=0; i<DATA_SIZE; ++i) data_set.push_back(next_value());

    cout << "Done!" << endl;

    cout << "-- Benchmark specifications --\n";
    cout << "Number of cores (for the pipeline test): " << CORES_NUM << "\n";
    cout << "Data set size:                           " << DATA_SIZE*8 / (float) 1000000 << "(MB)\n";
    cout << "Parallelism grain:                       " << RUNS << " runs per stage\n";
    cout << "Warning: it's recommended to not exceed the number of cores of this machine\n";
    
    // sequential test
    
    vector<double> seq_result_set;
    seq_result_set.reserve(DATA_SIZE);
    cout << "Running sequential computation..." << endl;
    chrono_start = chrono::system_clock::now();
    for (size_t i=0; i<DATA_SIZE; ++i) {
        double temp;
        for (size_t j=0; j<CORES_NUM; ++j) {
            if (j == 0) temp = sequentializer(data_set[i], RUNS, static_cast<double(*)(double)>(sin)); // first call
            else if (j%2 == 0) temp = sequentializer(temp, RUNS, static_cast<double(*)(double)>(sin)); // sin call
            else temp = sequentializer(temp, RUNS, static_cast<double(*)(double)>(cos)); // cos call
        }
        seq_result_set.push_back(temp);
    }
    chrono_stop = chrono::system_clock::now();
    auto seq_time = ((std::chrono::duration<double, std::milli>) (chrono_stop - chrono_start)).count();
    cout << "Done! [Elapsed time: " << seq_time << "(ms)]" << endl;

    // comp test

    ff_comp comp;
    vector<ff_node*> comp_stages;
    comp_stages.reserve(CORES_NUM);
    vector<double> comp_result_set;
    comp_result_set.reserve(DATA_SIZE);

    for (size_t i=0; i<CORES_NUM; ++i){
        if (i%2==0) comp_stages[i] = new SinStage();
        else comp_stages[i] = new CosStage();
        comp.add_stage(comp_stages[i]);
    }

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

    while (!comp_stages.empty()) {
        delete comp_stages.back();
        comp_stages.pop_back();
    }

    // pipeline test

    ff_pipeline pipeline;
    Emitter emitter(data_set);
    Collector odd_collector(true), even_collector(false);
    size_t internal_stages = CORES_NUM - 2; // we already have an emitter and a collector
    vector<ff_node*> pipe_stages;
    pipe_stages.reserve(internal_stages);

    pipeline.add_stage(&emitter);
    for (size_t i=0; i<internal_stages; ++i) {
        if(i%2==0) pipe_stages[i] = new CosStage();
        else pipe_stages[i] = new SinStage();
        pipeline.add_stage(pipe_stages[i]);
    }
    if (CORES_NUM%2 == 0) pipeline.add_stage(&even_collector);
    else pipeline.add_stage(&odd_collector);

    cout << "Running pipelined computation..." << endl;

    chrono_start = chrono::system_clock::now();
    if(pipeline.run_and_wait_end()<0) error("Running pipeline\n");
    chrono_stop = chrono::system_clock::now();
    vector<double> pipe_result_set;
    pipe_result_set.reserve(DATA_SIZE);
    if (CORES_NUM%2 == 0) pipe_result_set = even_collector.get_output_stream();
    else pipe_result_set = odd_collector.get_output_stream();

    auto pipe_time = ((std::chrono::duration<double, std::milli>) (chrono_stop - chrono_start)).count();
    cout << "Done! [Elapsed time: " << pipe_time << "(ms)]" << endl;

    while (!pipe_stages.empty()) {
        delete pipe_stages.back();
        pipe_stages.pop_back();
    }

    // farm test
    // NOTE: this part of the benchmark needs more test and a review and it may be useless for the final results,
    // at this time is no more than an exercise

    Emitter femitter(data_set);
    Collector feven_collector(false), fodd_collector(true);

    if (CORES_NUM%2 == 0) {
        size_t nworkers = CORES_NUM - 2;
        ff_Farm<> farm( [nworkers]() {
            vector<unique_ptr<ff_node>> fworkers;
            for (size_t i=0; i<nworkers; ++i) fworkers.push_back(make_unique<FarmWorker>(RUNS,CORES_NUM-2));
            return fworkers;
        }(), femitter, feven_collector );
        cout << "Running farmed computation..." << endl;
        chrono_start = chrono::system_clock::now();
        if (farm.run_and_wait_end()<0) error("Running farm test\n");
        chrono_stop = chrono::system_clock::now();
    } else {
        size_t nworkers = CORES_NUM - 2;
        ff_Farm<> farm( [nworkers]() {
            vector<unique_ptr<ff_node>> fworkers;
            for (size_t i=0; i<nworkers; ++i) fworkers.push_back(make_unique<FarmWorker>(RUNS,CORES_NUM-2));
            return fworkers;
        }(), femitter, fodd_collector );
        cout << "Running farmed computation..." << endl;
        chrono_start = chrono::system_clock::now();
        if (farm.run_and_wait_end()<0) error("Running farm test\n");
        chrono_stop = chrono::system_clock::now();
    }

    vector<double> farm_result_set;
    farm_result_set.reserve(DATA_SIZE);
    if (CORES_NUM%2 == 0) farm_result_set = even_collector.get_output_stream();
    else farm_result_set = odd_collector.get_output_stream();
    auto farm_time = ((std::chrono::duration<double, std::milli>) (chrono_stop - chrono_start)).count();
    cout << "Done! [Elapsed time: " << farm_time << "(ms)]" << endl;

    // performance evaluation

    cout << fixed;
    cout << "-- Performance statistics --\n";
    cout << "Difference between sequential and comp:     " << diff(seq_time,comp_time) << "(ms) \t" << setprecision(2) << diff_perc(seq_time,comp_time) << "%\n";
    cout << "Difference between pipeline and comp:       " << setprecision(6) << diff(pipe_time,comp_time) << "(ms) \t" << setprecision(2) << diff_perc(pipe_time,comp_time) << "%\n";
    cout << "Difference between farm and comp:           " << setprecision(6) << diff(farm_time,comp_time) << "(ms) \t" << setprecision(2) << diff_perc(farm_time,comp_time) << "%\n";
    cout << "Difference between sequential and pipeline: " << setprecision(6) << diff(seq_time,pipe_time) << "(ms) \t" << setprecision(2) << diff_perc(seq_time,pipe_time) << "%\n";
    cout << "Difference between sequential and farm:     " << setprecision(6) << diff(seq_time,farm_time) << "(ms) \t" << setprecision(2) << diff_perc(seq_time,farm_time) << "%\n";
    cout << "Difference between pipeline and farm:       " << setprecision(6) << diff(pipe_time,farm_time) << "(ms) \t" << setprecision(2) << diff_perc(pipe_time,farm_time) << "%\n";

    // consistency check (farm result are checked only in size because they aren't ordered)

    cout << "Checking consistency between the result sets...\n";
    size_t i=0;
    bool consistence = true;
    if(farm_result_set.size() != seq_result_set.size()) consistence = false;
    while (i<comp_result_set.size() && i<seq_result_set.size() && i<pipe_result_set.size() && consistence) {
        if (comp_result_set[i] != seq_result_set[i] || comp_result_set[i] != pipe_result_set[i])
            consistence = false;
        i++;
    }
    if (consistence) cout << "The results are consistent" << endl;
    else cout << "The results are NOT consistent" << endl; 

    return EXIT_SUCCESS;

}


// definition of the helper functions

double sequentializer (double input, unsigned long runs, std::function<double(double)> fun) {
    for (size_t i=0; i<runs; ++i) input = fun(input);
    return input;
}