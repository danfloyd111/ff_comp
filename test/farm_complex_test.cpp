/*  
 *  Author: Daniele Paolini, daniele.paolini@hotmail.it
 * 
 * Farm complex test:
 * Composing a farm of pipelines
 * Farm[Standard Emitter -> Pipeline(s) -> Standard Collector] where Pipeline = [Increment -> Double]
 * Expected Double(Increment(x)) where x is the input
 * 
 */

#include <cassert>
#include <iostream>
#include "../comp.hpp"

using namespace std;
using namespace ff;

struct Incr : ff_node {
    void* svc(void *t){
        *((int*)t)+=1;
        return t;
    } 
};

struct Doub : ff_node {
    void* svc(void *t){
        *((int*)t)*=2;
        return t;
    }
};

int main() {
    ff_comp comp;
    int num_pipes = 5;
    vector<ff_node *> pipelines;
    vector<ff_node *> incrementers;
    vector<ff_node *> doublers;
    for (auto i=0; i<num_pipes; ++i) pipelines.push_back(new ff_pipeline());
    for (auto i=0; i<num_pipes; ++i) incrementers.push_back(new Incr());
    for (auto i=0; i<num_pipes; ++i) doublers.push_back(new Doub());
    for (auto i=0; i<num_pipes; ++i) {
        ((ff_pipeline*)pipelines[i])->add_stage(incrementers[i]);
        ((ff_pipeline*)pipelines[i])->add_stage(doublers[i]);
    }
    ff_farm<> farm(pipelines);
    comp.add_stage(&farm);
    comp.set_cleanup();
    cout << "Executing complex farm test with input..." << endl;
    assert(*((int*)comp.run(new int(2)))==6);
    cout << "-> PASSED" << endl;
    return EXIT_SUCCESS;
}