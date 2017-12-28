/*  
 *  Author: Daniele Paolini, daniele.paolini@hotmail.it
 * 
 *  This file implements the composition construct, see the docs for further informations.
 *  NOTE: It hasn't node cleanup utility because the nodes that user needs to compose are 
 *  likely to be "owned" by a pipeline or a farm, so deleting them may be leading to double 
 *  deletions if "node_cleanup" flag is set on these objects.
 *
*/

/* ***************************************************************************
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License version 3 as 
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 ****************************************************************************
 */

#ifndef FF_COMP_HPP
#define FF_COMP_HPP

#include <ff/pipeline.hpp>
#include <ff/farm.hpp>
#include <ff/utils.hpp>
#include <chrono>

namespace ff {

    class ff_comp: public ff_node {

    private:
        svector<ff_node *> nodes;
        svector<ff_node *> decompose(ff_node* node);
        std::chrono::time_point<std::chrono::system_clock> cstart;
        std::chrono::time_point<std::chrono::system_clock> cend;
        double time_elapsed;


    protected:
        void *svc(void *t) { return run(t); }
        int svc_init() { return 0; }
        void svc_end() { }

    public:
        ff_comp() { time_elapsed = 0; }
        ~ff_comp() = default; 
        int add_stage(ff_node *stage);
        const svector<ff_node *>& get_stages() const { return nodes; };
         // init task is the inital task submitted to comp, ex: f(g(h(init_task))), if init_task is null h (in this example) is a function that
         // takes no input (single emitter, constant function, ...)
        void *run(void *init_task=nullptr);
        double ff_time() { return time_elapsed;  } // Returns total run time

    };

    int ff_comp::add_stage(ff_node *stage) {
        if (!stage) return -1;
        svector<ff_node *> nested = decompose(stage);
        for (ff_node *n : nested) nodes.push_back(n);
        return 0;
    }

    void* ff_comp::run(void *init_task) {
        
        cstart = std::chrono::system_clock::now();
        void *_in=nullptr, *_out=nullptr;
        if (nodes.empty()) error("comp has no stages to execute\n");
        for(size_t i=0; i<nodes.size(); ++i) {
            if (i == 0) _out = nodes[i]->svc(init_task); // first call
            else _out = nodes[i]->svc(_in);
            _in = _out;
        }
        cend = std::chrono::system_clock::now();
        time_elapsed += ((std::chrono::duration<double, std::milli>) (cend-cstart)).count();
        return _out;
    }

    // free helper function used to decompose nodes into the add_stage method
    svector<ff_node *> ff_comp::decompose(ff_node* node) {
        svector<ff_node *> n_list;
        if (ff_pipeline *p  = dynamic_cast<ff_pipeline*>(node)) {
            svector<ff_node *> pipe_list = p->getStages();
            if (pipe_list.empty()) error("Decomposing an empty pipeline\n");
            for (ff_node *n: pipe_list) {
                svector<ff_node *> temp = decompose(n);
                n_list += temp;
            }
        } else if (ff_farm<> *f  = dynamic_cast<ff_farm<>*>(node)) {
            svector<ff_node*> workers = f->getWorkers();
            if(workers.empty()) error("Decomposing an empty farm\n");
            else {
                svector<ff_node *> temp = decompose(workers[0]); // decomposing the first, we assume they're all executing the SAME task
                n_list += temp;
            }            
        } else if (ff_node *n  = dynamic_cast<ff_node*>(node)) {
            n_list.push_back(n);
        } else {
            error("only ff_pipeline, ff_farm and ff_node can be composed\n");
        }
        return n_list;
    }

} // namespace ff

#endif // FF_COMP_HPP
