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
 * Author: Daniele Paolini <daniele.paolini@hotmail.it>
 * 
 * Date  : January 2018
 *
 * It is basically the same program of ffcompvideo, except that we use Pipeline,
 * Seq and Comp as workers into a Farm in order to see how the newly implemented
 * Comp construct can act as an inner component of a graph of parallel executors.
 * 
 * The "skeleton" CLI argument it's an integer used to determine wich inner stage has to be
 * used:
 *   skeleton == 0 -> Farm(pipe(Source, Comp(Stage1, Stage2), Drain)) with 16 workers
 *   skeleton == 1 -> Farm(pipe(Source, Seq(Stage1, Stage2), Drain)) with 16 workers
 *   skeleton == 2 -> Farm(pipe(Source, Pipe(Stage1, Stage2), Drain)) with 8 workers
 * where Seq is a ff_node_t that executes in sequence the code contained into Stage1 and
 * Stage2 svc methods.
 * 
 * Note: for further information please see the ffcompvideo.cpp file.
 *
*/

#include "ffvideo.hpp" // definition of ff stages are in this header, please have a look
#include <ff/farm.hpp>

using namespace ff;
using namespace cv;
using namespace std;

// fixed number of components, run this program with ./ffvideofarm input skeleton [-v]
int main(int argc, char *argv[]) {

    Mat* edges;

    const int seq_workers_num = 16;
    const int comp_workers_num = 16;
    const int pipe_workers_num = 8;

    bool out_video_flag = false;

    int param;
    const char *pattern = "hv";
    while ((param = getopt(argc, argv, pattern)) != -1) {
        switch (param) {
            case 'h':
                cout << "Usage: ./ffvideofarm input skeleton [-v]" << endl;
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
        cout << "Usage: ./ffvideofarm input skeleton [-v]" << endl;
        return EXIT_FAILURE;
    }

    const string in_video_path = argv[optind];
    int skeleton_type;
    try {
        skeleton_type = stoi(argv[optind+1]);
    } catch (exception) {
        cerr << "Error: skeleton type must be an integer (0 for comp, 1 for sequential, 2 for pipeline)" << endl;
        cout << "Usage: ./ffvideofarm input skeleton [-v]" << endl;
        return EXIT_FAILURE;
    }

    vector<ff_node*> pipes, seqs, comps;
    vector<Stage1*> s1s;
    vector<Stage2*> s2s;
    Source source(in_video_path);
    Drain drain(out_video_flag);
    ff_pipeline main_pipe;
    // using a normal farm instead of an ordered one should decrease the completion time, but the frames would be processed not in order and the
    // result would be a flickering horrible video, so I prefer to use an ordered farm and pay a very little overhead
    ff_ofarm farm; 

    main_pipe.add_stage(&source);

    switch (skeleton_type) {
        case 0:
            // farm of comps
            cout << "Using comp skeleton" << endl;
            for (int i=0; i<comp_workers_num; ++i) {
                Stage1* temp_s1 = new Stage1();
                Stage2* temp_s2 = new Stage2();
                ff_comp* temp_comp = new ff_comp();
                temp_comp->add_stage(temp_s1);
                temp_comp->add_stage(temp_s2);
                s1s.push_back(temp_s1);
                s2s.push_back(temp_s2);
                comps.push_back(temp_comp);
            }
            if (farm.add_workers(comps)<0) {
                error("adding comp nodes to the farm\n");
                return EXIT_FAILURE;
            }
            break;
        case 1:
            // farm of seqs
            cout << "Using seq nodes" << endl;
            for (int i=0; i<seq_workers_num; ++i) seqs.push_back(new SeqNode());
            if (farm.add_workers(seqs)<0) {
                error("adding seq nodes to the farm\n");
                return EXIT_FAILURE;
            }
            break;
        case 2:
            // farm of pipelines
            cout << "Using pipeline skeleton" << endl;
            for (int i=0; i<pipe_workers_num; ++i) {
                Stage1* temp_s1 = new Stage1();
                Stage2* temp_s2 = new Stage2();
                ff_pipeline* temp_pipe = new ff_pipeline();
                temp_pipe->add_stage(temp_s1);
                temp_pipe->add_stage(temp_s2);
                s1s.push_back(temp_s1);
                s2s.push_back(temp_s2);
                pipes.push_back(temp_pipe);
            }
            if (farm.add_workers(pipes)<0) {
                error("adding pipe nodes to the farm\n");
                return EXIT_FAILURE;
            }
            break;
        default:
            cerr << "Error: skeleton type must one of these values: 0 (comp), 1 (sequential) or 2(pipeline)" << endl;
            cout << "Usage: ./ffvideofarm input skeleton [-v]" << endl;
            return EXIT_FAILURE;
    }

    main_pipe.add_stage(&farm);
    main_pipe.add_stage(&drain);

    cout << "Applying both enhance and emboss filters (it may take a while...)" << endl;
    if (out_video_flag) cout << "Visualizing output video..." << endl;

    chrono::time_point<chrono::system_clock> chrono_start = chrono::system_clock::now();
    if (main_pipe.run_and_wait_end()<0) {
        error("running main pipeline\n");
        return EXIT_FAILURE;
    }
    chrono::time_point<chrono::system_clock> chrono_stop = chrono::system_clock::now();

    // printing statistics

    double frames = (double) source.get_processed_frames();
    auto elapsed_time = ((chrono::duration<double, std::milli>) (chrono_stop - chrono_start)).count();

    cout << "Completion time: " << elapsed_time << " (ms)" << endl;
    cout << "Average time per frame: " << elapsed_time / frames << " (ms)" << endl; 
    cout << "(with " << frames << " frames)" << endl;

    double sum=0, avg=0;

    switch (skeleton_type) {
        case 0:
            for (int i=0; i<comps.size(); ++i) sum += ((ff_comp*)comps[i])->ff_time();
            break;
        case 1:
            for (int i=0; i<seqs.size(); ++i) sum += ((SeqNode*)seqs[i])->ff_time();
            break;
        case 2:
            for (int i=0; i<pipes.size(); ++i) sum += ((ff_pipeline*)pipes[i])->ffTime();
            break;
        default:
            cerr << "Error: this point should be inaccesible!" << endl;
            return EXIT_FAILURE;
    }

    avg = sum / seq_workers_num;
    cout << "Average branch completion time: " << avg << " (ms)\nDone!" << endl;

    // cleaning

    while (!s1s.empty()) {
        delete s1s.back();
        s1s.pop_back();
    }
    while (!s2s.empty()) {
        delete s2s.back();
        s2s.pop_back();
    }
    while (!pipes.empty()) {
        delete pipes.back();
        pipes.pop_back();
    }
    while (!seqs.empty()) {
        delete seqs.back();
        seqs.pop_back();
    }
    while (!comps.empty()) {
        delete comps.back();
        comps.pop_back();
    }

    return EXIT_SUCCESS;

}