/*
 *  Author: Daniele Paolini, daniele.paolini@hotmail.it
 * 
 *  Inner comp test:
 *  Pipelining a comp constuct:
 *  Pipe(Source, Comp, Drain) where Comp = Stage1(Stage2())
 * 
 * 
*/

#include <cassert>
#include <iostream>
#include "../comp.hpp"

using namespace ff;
using namespace std;

struct Source : ff_node {
    int counter;
    int svc_init() {
        counter = 0;
        return 0;
    }
    void *svc(void *){
        if (++counter>3) return EOS;
        return new int(counter);
    }
};

struct Stage1 : ff_node {
    void *svc(void *t){
        *((int*)t)+=1;
        return t;
    }
};

struct Stage2 : ff_node {
    void *svc(void *t){
        *((int*)t)*=2;
        return t;
    }
};

struct Drain : ff_node {
    vector<int> data;
    const vector<int>& get_data() const { return data; }
    void clear_data() { data.clear(); }
    void *svc(void *t){ 
        data.push_back(*((int*)t));
        delete t;
        return GO_ON;
    }
};

int main() {

    Source pipe_source, farm_source;
    Drain pipe_drain, farm_drain;
    Stage1 stage1;
    Stage2 stage2;
    ff_pipeline pipe;
    ff_comp comp;
    pipe.add_stage(&pipe_source);
    comp.add_stage(&stage2);
    comp.add_stage(&stage1);
    pipe.add_stage(&comp);    
    pipe.add_stage(&pipe_drain);
    
    // pipeline test

    cout << "Executing inner comp test with a pipeline..." << endl;
    if (pipe.run_and_wait_end()<0) {
        error("running pipeline\n");
        return EXIT_FAILURE;
    }
    vector<int> results = pipe_drain.get_data();
    pipe_drain.clear_data();
    for (int i=0; i<results.size(); i++) assert(results[i]==(i+1)*2+1);
    cout << "PASSED [Elapsed time: " << pipe.ffTime() << "(ms)]" << endl;

    // farm test

    vector<ff_node*> workers, s1s, s2s;
    ff_ofarm farm;
    ff_pipeline main_pipe;
    main_pipe.add_stage(&farm_source);
    for (int i=0; i<4; ++i) {
        ff_comp* temp_comp = new ff_comp();
        Stage1* temp_s1 = new Stage1();
        Stage2* temp_s2 = new Stage2();
        temp_comp->add_stage(temp_s2);
        temp_comp->add_stage(temp_s1);
        workers.push_back(temp_comp);
    }
    if (farm.add_workers(workers)<0) {
        error("adding workers to the farm\n");
        return EXIT_FAILURE;
    }
    main_pipe.add_stage(&farm);
    main_pipe.add_stage(&farm_drain);
    cout << "Executing inner comp test with a farm..." << endl;
    if (main_pipe.run_and_wait_end()<0) {
        error("running main pipeline\n");
        return EXIT_FAILURE;
    }
    vector<int> farm_results = farm_drain.get_data();
    farm_drain.clear_data();
    for (int i=0; i<farm_results.size(); i++) assert(farm_results[i]==(i+1)*2+1);
    cout << "PASSED [Elapsed time: " << main_pipe.ffTime() << "(ms)]" << endl;

    // cleaning

    while (!s1s.empty()) {
        delete s1s.back();
        s1s.pop_back();
    }
    while (!s2s.empty()) {
        delete s2s.back();
        s2s.pop_back();
    }
    while (!workers.empty()) {
        delete workers.back();
        workers.pop_back();
    }

    return EXIT_SUCCESS;

}
