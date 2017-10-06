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

int main() {
    N1 n1;
    N2 n2;
    ff_comp comp;
    comp.add_stage(&n1);
    comp.add_stage(&n2);
    cout << "Testing comp with no input..." << endl;
    assert(*((int*)comp.run())=42);
    cout << "-> PASSED" << endl;
    cout << "Testing comp with input..." << endl;
    assert(*((int*)comp.run(new int(100)))=100);
    cout << "-> PASSED" << endl;
    // type testing
    ff_farm<> f;
    ff_pipeline p;
    ff_comp c;
    c.add_stage(&f);
    c.add_stage(&p);
    c.run();
    cout << "End of first typetest" << endl;
    ff_comp c2;
    ff_pipeline p1;
    ff_pipeline p2;
    N1 na, nb, nc, nd, ne;
    p2.add_stage(&nd);
    p2.add_stage(&ne);
    p1.add_stage(&nc);
    p1.add_stage(&p2);
    c2.add_stage(&na);
    cout << "first add" << endl;
    c2.add_stage(&p1);
    cout << "sec add" << endl;
    c2.add_stage(&nb);
    cout << "last add" << endl;
    return EXIT_SUCCESS;
}