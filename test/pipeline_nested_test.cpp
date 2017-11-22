/*
 *  Author: Daniele Paolini, daniele.paolini@hotmail.it
 * 
 *  Nested pipeline test:
 *  Composing two nodes with a pipeline in between and one more pipeline nested into the previous one
 *  Comp(Node1, Pipeline, Node2) where Pipeline = [Node3 -> InnerPipeline -> Node4] and
 *  InnerPipeline = [Increment -> Double], Node-i will add i to his input (except for node 1)
 *  Expected: Node2(Node4(Double(Increment(Node3(Node1(x)))))) where x is the input
 * 
 *  Tested with valgrind http://valgrind.org/info/about.html
 * 
*/

#include <cassert>
#include <iostream>
#include "../comp.hpp"

using namespace ff;
using namespace std;

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

struct Node4: ff_node {
    void* svc(void *t){
        *((int*)t)+=4;
        return t;
    }
};

struct Incr: ff_node {
    void* svc(void *t){
        *((int*)t)+=1;
        return t;
    }
};

struct Doub: ff_node {
    void* svc(void *t){
        *((int*)t)*=2;
        return t;
    }
};

int main() {
    Node1 node1;
    Node2 node2;
    Node3 node3;
    Node4 node4;
    Incr incr;
    Doub doub;
    ff_pipeline pipeline, inner_pipeline;
    ff_comp comp;
    inner_pipeline.add_stage(&incr);
    inner_pipeline.add_stage(&doub);
    pipeline.add_stage(&node3);
    pipeline.add_stage(&inner_pipeline);
    pipeline.add_stage(&node4);
    comp.add_stage(&node1);
    comp.add_stage(&pipeline);
    comp.add_stage(&node2);
    cout << "Executing nested pipelines test without input..." << endl;
    int* result = (int*) comp.run();
    assert(*result==98);
    cout << "-> PASSED [Elapsed time: " << comp.ff_time() << "(ms)]" << endl;
    cout << "Executing nested pipelines test with input..." << endl;
    int *foo = new int(2);
    assert(*((int*)comp.run(foo))==18);
    cout << "-> PASSED [Elapsed time: " << comp.ff_time() << "(ms)]" << endl;
    delete foo;
    delete result;
    return EXIT_SUCCESS;
}