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

using namespace ff;
using namespace cv;
using namespace std;

// fixed number of components, run this program with ./ffvideofarm input skeleton [-v]
int main(int argc, char *argv[]) {

    Mat* edges;

    const int seq_workers_num = 8;
    const int comp_workers_num = 8;
    const int pipe_workers_num = 16;

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

    vector<SeqNode> seqs;
    vector<ff_comp> comps;
    vector<ff_pipeline> pipes, inner_pipes;

    return EXIT_SUCCESS;

}