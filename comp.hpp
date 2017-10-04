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
        int run();

    }

}

int ff_comp::add_stage(ff_node *stage) {
    if (!stage) return -1;
    nodes_list.push_back(stage);
    return 0;
}