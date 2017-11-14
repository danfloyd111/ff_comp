/*
 *  Author: Daniele Paolini, daniele.paolini@hotmail.it
 * 
 *  Cleanup test:
 *  Composing three nodes with "cleanup_nodes" set to see if there are any memory leaks
 *  n.b. use valgrind
*/

#include <iostream>
#include "../comp.hpp"

using namespace std;
using namespace ff;

struct Node1: ff_node {
    void* svc(void *t){
        if (t) return t;
        return new int(42);
    }
};

struct Node2: ff_node {
    void* svc(void *t){
        *((int*)t)+=2;
        return t;
    }
};

struct Node3: ff_node {
    void* svc(void *t){
        *((int*)t)+=3;
        return t;
    }
};

int main() {
    Node1 *node1 = new Node1();
    Node2 *node2 = new Node2();
    Node3 *node3 = new Node3();
    ff_comp comp;
    comp.add_stage(node1);
    comp.add_stage(node2);
    comp.add_stage(node3);
    comp.set_cleanup();
    return EXIT_SUCCESS;
}