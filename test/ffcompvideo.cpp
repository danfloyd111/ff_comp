/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
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
 *

/*
 * Author: Marco Danelutto <marcod@di.unipi.it> - Original author
 *         Daniele Paolini <daniele.paolini@hotmail.it> - ffcomp related modifications
 * 
 * Date  : December 2017
 *
 * It's basically the same program placed under fastflow/examples/OpenCVvideo/ffvideo.cpp
 * but instead of testing a "pure pipeline": pipe(Source, Stage1, Stage2, Drain) it will
 * be tested a pipeline with an inner comp: pipe(Source, Comp(Stage1, Stage2), Drain).
 *
*/

#include <opencv2/opencv.hpp>
#include <ff/pipeline.hpp>

using namespace ff;
using namespace cv;
using namespace std;

// Reads frames and sends them to the next stage
// TODO: consider to change this into a plain old ff_node if this version do not works
struct Source : ff_node_t<Mat> {

  const string filename;

  Source(const string filename) : filename(filename) { }

  Mat *svc(Mat *) {
    VideoCapture cap(filename.c_str());
    if (!cap.isOpened()) {
      cerr << "Error: opening input file" << endl;
      return EOS;
    }
    for(;;) {
      Mat *frame = new Mat();
      if(cap.read(*frame)) ff_send_out(frame);
      else {
	cout << "End of stream in input" << endl;
	break;
      }
    }
    return EOS;
  }

};

// This stage applies the GaussianBlur filter and sends the result to the next stage
//TODO: consider to change this into a plain old ff_node if this version do not works
struct Stage1 : ff_node_t<Mat> {

  Mat *svc(Mat *frame) {
    Mat frame1;
    GaussianBlur(*frame, frame1, Size(0,0), 3);
    addWeigthed(*frame, 1.5, frame1, -0.5, 0, *frame);
    return frame;
  }

};

// This stage applies the Sobel filter and sends the result to the next stage
// TODO: consider to change this into a plain old ff_node if this version do not works
struct Stage2 : ff_node_t<Mat> {

  Mat *svc(Mat *frame) {
    Sobel(*frame, *frame, -1, 1, 0, 3);
    return frame;
  }

};

// This stage shows the output
// TODO: consider to change this into a plain old ff_node if this version do not works
// TODO: instead of showing the output in a frame consider to write the video into a file
//       which name is chosen by the user with an option
struct Drain : ff_node_t<Mat> {

  Drain(bool ovf) : outvideo(ovf) { }

  int svc_init() {
    if (outvideo) namedWindow("edges",1);
    return 0;
  }

  Mat *svc(Mat *frame) {
    if (outvideo) {
      imshow("edges", *frame);
      waitkey(30);
    }
    delete frame;
    return GO_ON;
  }
  
protected:
  const bool outvideo;

};

/********************************************************************************************/

// fixed number of parameters, run this program with: ffcompvideo input.avi [-o output.avi]
int main(int argc, char *argv[]) {

  Mat edges;

  return 0;

}
