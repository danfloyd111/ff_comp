/*
 *  Author: Daniele Paolini, daniele.paolini@hotmail.it
 * 
 *  Inner comp test:
 *  Pipelining a comp constuct:
 *  Pipe(Source, Comp, Drain) where Comp = Stage1(Stage2())
 * 
 *  Tested with valgrind http://valgrind.org/info/about.html
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

    Source source;
    Drain drain;
    Stage1 stage1;
    Stage2 stage2;
    ff_pipeline pipe;
    ff_comp comp;
    pipe.add_stage(&source);
    comp.add_stage(&stage2);
    comp.add_stage(&stage1);
    pipe.add_stage(&comp);    
    pipe.add_stage(&drain);
    
    // pipeline test

    cout << "Executing inner comp test with a pipeline..." << endl;
    if (pipe.run_and_wait_end()<0) {
        error("running pipeline\n");
        return EXIT_FAILURE;
    }
    vector<int> results = drain.get_data();
    drain.clear_data();
    for (int i=0; i<results.size(); i++) assert(results[i]==(i+1)*2+1);
    cout << "PASSED [Elapsed time: " << pipe.ffTime() << "(ms)]" << endl;
    
    /* TODO: farm test (?)

    vector<ff_node *> workers;
    workers.push_back(new ff_comp());
    workers.push_back(new ff_comp());
    Emitter emitter;
    Stage1 stageA;
    Stage2 stageB;
    ((ff_comp*)workers[0])->add_stage(&stage2);
    ((ff_comp*)workers[1])->add_stage(&stageB);
    ((ff_comp*)workers[0])->add_stage(&stage1);
    ((ff_comp*)workers[1])->add_stage(&stageA);
    ff_farm<> farm(workers);
    farm.cleanup_all();
    ff_pipeline pipe2;
    pipe2.add_stage(&emitter);
    //pipe2.add_stage(&farm);
    pipe2.add_stage(&drain);        
    cout << "Executing inner comp test with a farm..." << endl;
    if (pipe2.run_and_wait_end()<0) {
        error("running pipeline\n");
        return EXIT_FAILURE;
    }
    results.clear();
    results = drain.get_data();
    sort(results.begin(), results.end());
    drain.clear_data();
    for (int i=0; i<results.size(); i++) cout << results[i] << endl;//assert(results[i]==(i+1)*2+1);
    cout << "PASSED [Elapsed time: " << pipe2.ffTime() << "(ms)]" << endl;

    */

    return EXIT_SUCCESS;

}