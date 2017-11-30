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
#include <fstream>
#include <random>
#include <functional>
#include <vector>

using namespace std;

int main() {

    // creating an universal random big data set for the benchmark (~50MB)
    const long DATA_SIZE = 6553600;
    const string DATA_PATH = "test/bin/pipeline_benchmark_dataset"; 

    uniform_real_distribution<double> dist {1, 10000};
    default_random_engine engine { };
    auto next_value = bind(dist, engine);
    
    cout << "Generating random data set..." << endl;

    ofstream data_out;
    data_out.open(DATA_PATH, ios::out);
    if(!data_out.is_open()) {
        cerr << "ERROR: there was a problem opening the output file" << endl;
        return EXIT_FAILURE;
    }
    for (auto i=0; i<DATA_SIZE; ++i) data_out << next_value() << "\n";
    data_out.close();

    cout << "Done!" << endl;
    
    // beginning of the completely sequential test
    vector<double> data;
    data.reserve(DATA_SIZE);
    ifstream data_in;
    data_in.open(DATA_PATH, ios::in);
    if(!data_in.is_open()) {
        cerr << "ERROR: there was a problem opening the input file" << endl;
        return EXIT_FAILURE;
    }
    double tmp;
    while (data_in >> tmp) data.push_back(tmp);
    data_in.close();

    cout << data.size() << endl;
    return EXIT_SUCCESS;

}