/*
 *  Author: Daniele Paolini, daniele.paolini@hotmail.it
 * 
 *  Basic test of the comp construct:
 *  Composing two nodes, either with and without input
 */

#include <cassert>
#include <iostream>
#include "../comp.hpp"

using namespace std;
using namespace ff;

struct N1: ff_node {
    void* svc(void *t){
        if (t) return t;
        else return new int(42);
    }
};

struct N2: ff_node {
    void* svc(void *t){
        return t;
    }
};

int main() {
    N1 n1;
    N2 n2;
    ff_comp comp;
    comp.add_stage(&n1);
    comp.add_stage(&n2);
    cout << "Executing basic test without input..." << endl;
    assert(*((int*)comp.run())==42);
    cout << "-> PASSED" << endl;
    cout << "Executing basic test with input..." << endl;
    assert(*((int*)comp.run(new int(100)))==100);
    cout << "-> PASSED" << endl;
    return EXIT_SUCCESS;
}