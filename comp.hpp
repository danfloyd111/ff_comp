#ifndef FF_COMP_HPP
#define FF_COMP_HPP

#include <ff/node.hpp>

#endif

namespace ff {

    class ff_comp: public ff_node {

    private:
        svector<ff_node *> nodes_list;

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
        nodes_list.push_back(stage);
        return 0;
    }

    void *ff_comp::run(void *init_task) {
        void *_in=nullptr, *_out=nullptr;
        for(size_t i=0; i<nodes_list.size(); ++i) {
            if (i == 0) _out = nodes_list[i]->svc(init_task); // first call
            else _out = nodes_list[i]->svc(_in);
            _in = _out;
        }
        return _out;
    }

} // namespace