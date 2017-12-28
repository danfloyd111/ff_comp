/* ***************************************************************************
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as 
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  As a special exception, you may use this file as part of a free software
 *  library without restriction.  Specifically, if other files instantiate
 *  templates or use macros or inline functions from this file, or you compile
 *  this file and link it with other files to produce an executable, this
 *  file does not by itself cause the resulting executable to be covered by
 *  the GNU General Public License.  This exception does not however
 *  invalidate any other reasons why the executable file might be covered by
 *  the GNU General Public License.
 *
 ****************************************************************************
 */

/*
 * Author: Marco Danelutto <marcod@di.unipi.it> - Original author
 *         Daniele Paolini <daniele.paolini@hotmail.it> - ffcomp related modifications
 * 
 * Date  : December 2017
 *
 * It's basically the same program placed under fastflow/examples/OpenCVvideo/ffvideo.cpp
 * but instead of testing a "pure pipeline": pipe(Source, Stage1, Stage2, Drain) it will
 * be tested a pipeline with a custom inner stage based on user selection.
 * The "skeleton" CLI argument it's an integer used to determine wich inner stage has to be
 * used:
 *   skeleton == 0 -> pipe(Source, Comp(Stage1, Stage2), Drain)
 *   skeleton == 1 -> pipe(Source, Seq(Stage1, Stage2), Drain)
 *   skeleton == 2 -> pipe(Source, Pipe(Stage1, Stage2), Drain)
 * where Seq is a ff_node_t that executes in sequence the code contained into Stage1 and
 * Stage2 svc methods.
 * 
 * We expect to not find any notable difference in completion time between Seq and Comp version,
 * on the other side we expect to see an huge speedup between Pipe and Seq / Comp.
 * 
 * (-v option: visualize output video)
 *
*/

#include "ffvideo.hpp" // definition of ff stages are in this header, please have a look

using namespace ff;
using namespace cv;
using namespace std;

// fixed number of stages, run this program with: ffcompvideo input skeleton [-v]
int main(int argc, char *argv[]) {
  
    Mat edges;
  
    bool out_video_flag = false;
    
    int param;
    const char *pattern = "hv";
    while ((param = getopt(argc, argv, pattern)) != -1) {
        switch (param) {
            case 'h':
                cout << "Usage: ./ffcompvideo input skeleton [-v]" << endl;
                return EXIT_SUCCESS;
            case 'v':
                out_video_flag = true;
                break;
            case '?':
                if (optopt == 'v')
	                  cerr << "Error: option -" << optopt << " requires an argument" << endl;
                else if (isprint(optopt))
	                  cerr << "Error: unknown option -" << (char) optopt << endl;
                else
	                  cerr << "Error: unkonw option character" << endl;
                    return EXIT_FAILURE;
            default:
                cerr << "Error: parsing command line" << endl;
                return EXIT_FAILURE;
        }
    }

    if (argc < 3) {
        cerr << "Error: you must provide a video input and select a valid skeleton type (0 for comp, 1 for sequential, 2 for pipeline)" << endl;
        cout << "Usage: ./ffcompvideo input skeleton [-v]" << endl;
        return EXIT_FAILURE;
    }

    const string in_video_path = argv[optind];
    int skeleton_type;
    try {
        skeleton_type = stoi(argv[optind+1]);
    } catch (exception) {
        cerr << "Error: skeleton type must be an integer (0 for comp, 1 for sequential, 2 for pipeline)" << endl;
        cout << "Usage: ./ffcompvideo input skeleton [-v]" << endl;
        return EXIT_FAILURE;
    }

    ff_comp comp;
    ff_pipeline pipe, inner_pipe;
    Source source(in_video_path);
    Stage1 stage1;
    Stage2 stage2;
    Drain drain(out_video_flag);
    
    pipe.add_stage(&source);

    switch (skeleton_type) {
        case 0:
            cout << "Selected comp inner stage: Pipe(Source, Comp(Stage1, Stage2), Drain)" << endl;
            comp.add_stage(&stage1);
            comp.add_stage(&stage2);
            pipe.add_stage(&comp);
            break;
        case 1:
            cout << "Selected sequential inner stage: Pipe(Source, Seq(Stage1, Stage2), Drain)" << endl;
            cerr << "NOT YET IMPLEMENTED!" << endl;
            return EXIT_SUCCESS;
            break;
        case 2:
            cout << "Selected pipeline inner stage: Pipe(Source, Pipe(Stage1, Stage2, Drain)" << endl;
            inner_pipe.add_stage(&stage1);
            inner_pipe.add_stage(&stage2);
            pipe.add_stage(&inner_pipe);
            break;
        default:
            cerr << "Error: skeleton type must one of these values: 0 (comp), 1 (sequential) or 2(pipeline)" << endl;
            cout << "Usage: ./ffcompvideo input skeleton [-v]" << endl;
            return EXIT_FAILURE;
    }

    pipe.add_stage(&drain);

    cout << "Applying both enhance and emboss filters (it may take a while...)" << endl;
    if (out_video_flag) cout << "Visualizing output video..." << endl;
  
    chrono::time_point<chrono::system_clock> chrono_start = chrono::system_clock::now();
    if (pipe.run_and_wait_end()<0) {
        error("running pipeline");
        return EXIT_FAILURE;
    }
    chrono::time_point<chrono::system_clock> chrono_stop = chrono::system_clock::now();

    double frames = (double) source.get_processed_frames();
    auto elapsed_time = ((chrono::duration<double, std::milli>) (chrono_stop - chrono_start)).count();

    cout << "Completion time: " << elapsed_time << "(ms)" << endl;
    cout << "Average time per frame: " << elapsed_time / frames << "(ms)" << endl; 
    cout << "(with " << frames << " frames)" << endl;
    
    switch (skeleton_type) {
        case 0:
            cout << "Inner Comp elapsed time: " << comp.ff_time() << "(ms)\nDone!" <<  endl;
            break;
        case 1:
            // TODO: print seq.ff_time() when its developed
            //cout << "Inner Sequential elapsed time: " << seq.ff_time() << "(ms)\nDone!" << endl;
            break;
        case 2:
            cout << "Inner Pipeline elapsed time: " << inner_pipe.ffTime() << "(ms)\nDone!" << endl;
            break;
        default:
            cerr << "Error: this point should be inaccesible!" << endl;
            return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;

}
