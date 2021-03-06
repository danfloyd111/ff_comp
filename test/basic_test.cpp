/*
 *  Author: Daniele Paolini, daniele.paolini@hotmail.it
 * 
 *  Basic test of the comp construct:
 *  Composing two nodes, either with and without input
 *  
 *  Tested with valgrind http://valgrind.org/info/about.html
 *
 */

#include <cassert>
#include <iostream>
#include "../comp.hpp"

using namespace std;
using namespace ff;

struct N1: public ff_node {
    void* svc(void *t){
        if (t) return t;
        else return new int(42);
    }
};

struct N2: public ff_node {
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
    int* result = (int*) comp.run();
    assert(*result==42);
    cout << "-> PASSED [Elapsed time: " << comp.ff_time() << "(ms)]" << endl;
    cout << "Executing basic test with input..." << endl;
    int *foo = new int(100);
    int res = *(int*)(comp.run(foo));
    assert(res==100);
    cout << "-> PASSED [Elapsed time: " << comp.ff_time() << "(ms)]" << endl;
    delete foo;
    delete result;
    return EXIT_SUCCESS;
}