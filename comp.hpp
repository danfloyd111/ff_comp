#ifndef FF_COMP_HPP
#define FF_COMP_HPP

#include <ff/pipeline.hpp>
#include <ff/farm.hpp>
#include <iostream> // debug

#endif

namespace ff {

    class ff_comp: public ff_node {

    private:
        svector<ff_node *> nodes_list;
        svector<ff_node *> decompose(ff_node* node);
        bool node_cleanup;

    protected:
        void *svc(void *) { return nullptr; }
        int svc_init() { return -1; }
        void svc_end() { }

    public:
        ff_comp():node_cleanup(false) { };
        ~ff_comp(); 
        int add_stage(ff_node *stage);
        const svector<ff_node *>& get_stages() const { return nodes_list; };
         // init task is the inital task submitted to comp, ex: f(g(h(init_task))), if init_task is null h (in this example) is a function that
         // takes no input (emitter, constant function, ...)
        void *run(void *init_task=nullptr);
        void set_cleanup() { node_cleanup = true; }

    };

    ff_comp::~ff_comp() {
        if(node_cleanup) {
            while (nodes_list.size()>0) {
                ff_node *n = nodes_list.back();
                nodes_list.pop_back();
                delete n;
                std::cout << "clean" << std::endl; // debug
            }
        }
    }

    int ff_comp::add_stage(ff_node *stage) {
        if (!stage) return -1;
        svector<ff_node *> nested = decompose(stage);
        for (ff_node *n : nested) nodes_list.push_back(n);
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

    // free helper function used to decompose nodes into the add_stage method
    svector<ff_node *> ff_comp::decompose(ff_node* node) {
        svector<ff_node *> n_list;
        if (ff_pipeline *p  = dynamic_cast<ff_pipeline*>(node)) {
            // needs to be recursive and check the type of the nodes into the pipeline
            svector<ff_node *> pipe_list = p->getStages();
            for (ff_node *n: pipe_list) {
                svector<ff_node *> temp = decompose(n);
                n_list += temp;
            }
        } else if (ff_farm<> *f  = dynamic_cast<ff_farm<>*>(node)) {
            // not yet implemented !
            error("farm case not yet implemented\n");
            // exit(EXIT_FAILURE);            
        } else if (ff_node *n  = dynamic_cast<ff_node*>(node)) {
            // every ff_node derived class which hasn't fall in the previous clauses will fall into this one,
            // not sure if this is the correct behaviour...
            n_list.push_back(n);
        } else {
            error("only ff_pipeline, ff_farm and ff_node can be composed\n");
            // exit(EXIT_FAILURE);
        }
        return n_list;
    }

} // namespace