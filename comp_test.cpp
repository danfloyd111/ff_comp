#include <iostream>
#include <ff/node.hpp>
#include "comp.hpp"

using namespace std;
using namespace ff;

struct N1: ff_node {
    void* svc(void *) {
        cout << "Hello, I'm node 1" << endl;
        return EOS;
    }
};

struct N2: ff_node {
    void* svc(void *) {
        cout << "Hello, I'm node 2" << endl;
        return EOS;
    }
};

int main() {
    N1 n1;
    N2 n2;
    ff_comp comp;
    comp.add_stage(&n1);
    comp.add_stage(&n2);
    if(comp.run()<0) error("running comp");
    return EXIT_SUCCESS;
}