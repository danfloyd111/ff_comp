/*
 *  Author: Daniele Paolini, daniele.paolini@hotmail.it
 *  
 *  Farm test:
 *  Composing a simple farm of workers(incrementers)
 *  Comp(Farm) where Farm = [Default Emitter->Worker(s)->Default Collector]
 *  Expected Worker(x) where x is the input
 *
 *  Tested with valgrind http://valgrind.org/info/about.html
 *
*/

#include <cassert>
#include <iostream>
#include "../comp.hpp"

using namespace std;
using namespace ff;

struct Worker : ff_node {
    void *svc(void *t) {
        if (t) {
            *((int*)t)+=1;
            return t;
        }
        return new int(42);
    }
};

int main() {
    int num_workers = 5;
    vector<ff_node *> workers;
    for (auto i=0; i<num_workers; ++i) workers.push_back(new Worker);
    ff_farm<> farm(workers);
    farm.cleanup_all();
    ff_comp comp;
    comp.add_stage(&farm);
    cout << "Executing simple farm test without input..." << endl;
    int* result = (int*) comp.run();
    assert(*result==42);
    cout << "-> PASSED [Elapsed time: " << comp.ff_time() << "(ms)]" << endl;
    cout << "Executing simple farm test with input..." << endl;
    int *foo = new int(2);
    assert(*((int*)comp.run(foo))==3);
    cout << "-> PASSED [Elapsed time: " << comp.ff_time() << "(ms)]" << endl;
    delete foo;
    delete result;
    return EXIT_SUCCESS;
}