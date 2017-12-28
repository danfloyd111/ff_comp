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
 * be tested a pipeline with an inner comp: pipe(Source, Comp(Stage1, Stage2), Drain).
 * (-v option: visualize output video)
 *
*/

#include "ffvideo.hpp" // definition of ff stages are in this header, please have a look

using namespace ff;
using namespace cv;
using namespace std;

// fixed number of parameters, run this program with: ffcompvideo input.avi [-o output.avi]
int main(int argc, char *argv[]) {

  Mat edges;
  
  string in_video_path;
  bool out_video_flag = false;
  
  if (argc < 2) {
    cerr << "Error: you must provide a video input" << endl;
    cout << "Usage: ./ffcompvideo INPUT_VIDEO [-v]" << endl;
    return EXIT_FAILURE;
  }
    
  int param;
  const char *pattern = "hv";
  while ((param = getopt(argc, argv, pattern)) != -1) {
    switch (param) {
    case 'h':
      cout << "Usage: ./ffcompvideo INPUT_VIDEO [-v]" << endl;
      return EXIT_SUCCESS;
    case 'v':
      out_video_flag = true;
      cout << "Visualizing output video..." << endl;
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

  in_video_path = argv[optind];

  cout << "Applying both enhance and emboss filters (it may take a while...)" << endl;
  
  ff_comp comp;
  ff_pipeline pipe;
  Source source(in_video_path);
  Stage1 stage1;
  Stage2 stage2;
  Drain drain(out_video_flag);
  comp.add_stage(&stage1);
  comp.add_stage(&stage2);
  pipe.add_stage(&source);
  pipe.add_stage(&comp);
  pipe.add_stage(&drain);
  
  chrono::time_point<chrono::system_clock> chrono_start = chrono::system_clock::now();
  if (pipe.run_and_wait_end()<0) {
    error("running pipeline");
    return EXIT_FAILURE;
  }
  chrono::time_point<chrono::system_clock> chrono_stop = chrono::system_clock::now();

  double frames = (double) source.get_processed_frames();
  auto elapsed_time = ((chrono::duration<double, std::milli>) (chrono_stop - chrono_start)).count();

  cout << "Elapsed time: " << elapsed_time << "(ms)" << endl;
  cout << "Average time per frame: " << elapsed_time / frames << "(ms)" << endl; 
  cout << "(with " << frames << " frames)" << endl;
  cout << "Inner Comp Time: " << comp.ff_time() << "(ms)" <<  endl;
  cout << "Done!" << endl;
  return EXIT_SUCCESS;

}
