#ifndef FF_COMP_HPP
#define FF_COMP_HPP

#include <ff/node.hpp>
// DEBUG ONLY
#include <iostream>
#include <ff/pipeline.hpp>
#include <ff/farm.hpp> 

#endif

namespace ff {

    class ff_comp: public ff_node {

    private:
        svector<ff_node *> nodes_list;
        svector<ff_node *> decompose(ff_node* node);

    protected:
        void *svc(void *) { return nullptr; }
        int svc_init() { return -1; }
        void svc_end() { };

    public:
        ff_comp() = default;
        virtual ~ff_comp() = default; 
        int add_stage(ff_node *stage);
        const svector<ff_node *>& get_stages() const { return nodes_list; };
         // init task is the inital task submitted to comp, ex: f(g(h(init_task))), if init_task is null h (in this example) is a function that
         // takes no input (emitter, constant function, ...)
        void *run(void *init_task=nullptr);

    };

    int ff_comp::add_stage(ff_node *stage) {
        if (!stage) return -1;
        // stage can't be nullptr because of previous statement
        //if (typeid(*stage) == typeid(ff_node)) std::cout << "it's a node" << std::endl;
        //if (std::is_same<typeid(*stage).name(),ff_node>::value) std::cout << "it's a node" << std::endl; 
        //else std::cout << "isn't a node" << std::endl;
        
        /* THIS WORKS
        if (ff_pipeline *p  = dynamic_cast<ff_pipeline*>(stage)) {
            std::cout << "PIPELINE" << std::endl;
        } else if (ff_farm<> *f  = dynamic_cast<ff_farm<>*>(stage)) {
            std::cout << "FARM" << std::endl;            
        } else if (ff_node *n  = dynamic_cast<ff_node*>(stage)) {
            std::cout << "NODE" << std::endl;
        } else {
            error("only ff_pipeline, ff_farm and ff_node can be composed\n");
            return -1;
        }
        nodes_list.push_back(stage);
        */

        svector<ff_node *> nlist = decompose(stage);
        for (ff_node *n : nlist) nodes_list.push_back(n);
        
        return 0;
    }

    void *ff_comp::run(void *init_task) {
        void *_in=nullptr, *_out=nullptr;
        if (nodes_list.empty()) error("comp has no stages to execute\n");
        for(size_t i=0; i<nodes_list.size(); ++i) {
            if (i == 0) _out = nodes_list[i]->svc(init_task); // first call
            else _out = nodes_list[i]->svc(_in);
            _in = _out;
        }
        return _out;
    }

    // free helper function used to analyze nodes into the run method
    svector<ff_node *> ff_comp::decompose(ff_node* node) {
        svector<ff_node *> n_list;
        if (ff_pipeline *p  = dynamic_cast<ff_pipeline*>(node)) {
            std::cout << "PIPELINE" << std::endl;
            // needs to be recursive and check the type of the nodes into the pipeline
            svector<ff_node *> pipe_list = p->getStages();
            svector<ff_node *> nested_list;
            for (ff_node *n: pipe_list) nested_list = decompose(n);
            for (ff_node *n: nested_list) n_list.push_back(n);
        } else if (ff_farm<> *f  = dynamic_cast<ff_farm<>*>(node)) {
            // not yet implemented !
            std::cout << "FARM" << std::endl;
            error("farm case not yet implemented\n");
            // exit(EXIT_FAILURE);            
        } else if (ff_node *n  = dynamic_cast<ff_node*>(node)) {
            std::cout << "NODE" << std::endl;
            n_list.push_back(n);
        } else {
            error("only ff_pipeline, ff_farm and ff_node can be composed\n");
            // exit(EXIT_FAILURE);
        }
        return n_list;
    }

} // namespace