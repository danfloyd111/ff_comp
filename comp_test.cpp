#include <cassert>
#include <iostream>
#include "comp.hpp"

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
    // basic test
    N1 n1;
    N2 n2;
    ff_comp comp;
    comp.add_stage(&n1);
    comp.add_stage(&n2);
    cout << "Basic test with no input..." << endl;
    assert(*((int*)comp.run())=42);
    cout << "-> PASSED" << endl;
    cout << "Basic test with input..." << endl;
    assert(*((int*)comp.run(new int(100)))=100);
    cout << "-> PASSED" << endl;
    // simple pipeline test
    // composing two nodes with a pipeline in between N1 - Pipe - N2 where Pipe = [Increment -> Double]
    // expected N2(Pipe(N1(x)))
    ff_pipeline pipe1;
    ff_comp comp2;
    Incr incr;
    Doub doub;
    pipe1.add_stage(&incr);
    pipe1.add_stage(&doub);
    comp2.add_stage(&n1);
    comp2.add_stage(&pipe1);
    comp2.add_stage(&n2);
    cout << "Basic pipeline test..." << endl;
    assert(*((int*)comp.run(new int(2)))=6);
    cout << "-> PASSED" << endl;
    return EXIT_SUCCESS;
}
