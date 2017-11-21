/*
 *  Author: Daniele Paolini, daniele.paolini@hotmail.it
 * 
 *  Basic pipeline test:
 *  Composing two nodes with a pipeline in between
 *  Comp(Node1, Pipeline, Node2) where Pipeline = [Increment -> Double]
 *  Expected Node2(Double(Increment(Node1(x)))) where x is the input
 */

#include <cassert>
#include <iostream>
#include "../comp.hpp"

using namespace std;
using namespace ff;

struct N1: ff_node {
    void* svc(void * t) {
        if (t) return t;
        else return new int(42);
    }
};

struct N2: ff_node {
    void* svc(void *t) {
        *((int*)t)*=3;
        return t;
    }
};

struct Incr: ff_node {
    void* svc(void *t) {
        if (t) {
            *((int*)t)+=1;
            return t;
        }
        else return new int(1);
    }
};

struct Doub: ff_node {
    void* svc(void *t) {
        if (t) {
            *((int*)t)*=2;
            return t;
        }
        else return new int(1);
    }
};

int main() {
    N1 n1;
    N2 n2;
    ff_pipeline pipeline;
    ff_comp comp;
    Incr incr;
    Doub doub;
    pipeline.add_stage(&incr);
    pipeline.add_stage(&doub);
    comp.add_stage(&n1);
    comp.add_stage(&pipeline);
    comp.add_stage(&n2);
    cout << "Executing basic pipeline test without input..." << endl;
    assert(*((int*)comp.run())==258);
    cout << "-> PASSED [Elapsed time: " << comp.ff_time() << "(ms)]" << endl;
    cout << "Executing basic pipeline test with input..." << endl;
    int *foo = new int(2);
    assert(*((int*)comp.run(foo))==18);
    delete foo;
    cout << "-> PASSED [Elapsed time: " << comp.ff_time() << "(ms)]" << endl;
    return EXIT_SUCCESS;
}