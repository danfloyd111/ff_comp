#include <iostream>
#include <ff/node.hpp>
#include "comp.hpp"

using namespace std;
using namespace ff;

struct N1: ff_node {
    void* svc(void * t) {
        cout << "Hello, I'm node 1" << endl;
        if (t) return t;
        else return new int(42);
    }
};

struct N2: ff_node {
    void* svc(void * t) {
        cout << "Hello, I'm node 2. Received " << *((int*)t) << endl;
        return t;
    }
};

int main() {
    N1 n1;
    N2 n2;
    ff_comp comp;
    comp.add_stage(&n1);
    comp.add_stage(&n2);
    cout << "First run:" << endl;
    cout << "Comp collected: " << *((int*)comp.run()) << endl;
    cout << "\n";
    cout << "Comp collected: " << *((int*)comp.run(new int(100))) << endl;
    return EXIT_SUCCESS;
}