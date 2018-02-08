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
 * This file contains the definition of the stages used into ffvideo test suite
 *
*/

#ifndef FF_VIDEO_HPP
#define FF_VIDEO_HPP

#include <opencv2/opencv.hpp>
#include <ff/pipeline.hpp>
#include <unistd.h>
#include <chrono>
#include "../comp.hpp"

using namespace ff;
using namespace std;
using namespace cv;

// Reads frames and sends them to the next stage
struct Source : ff_node_t<Mat> {
    
    const string filename;
    int frames;

    Source(const string filename) : filename(filename) { }

    int svc_init() {
		frames = 0;
		return 0;
    }

    Mat *svc(Mat *) {
		VideoCapture cap(filename.c_str());
		if (!cap.isOpened()) {
	    	cerr << "Error: opening input file" << endl;
	    	return EOS;
		}
		for (;;) {
	    	Mat *frame = new Mat();
	    	frames++;
	    	if (cap.read(*frame)) ff_send_out(frame);
	    	else {
				cout << "End of stream in input" << endl;
				break;
	    	}
		}
		return EOS;
    }

public:
    int get_processed_frames() { return frames; }

};

// This stage applies the Gaussian Blur filter and sends the result to the next stage
struct Stage1 : ff_node_t<Mat> {

    Mat *svc(Mat *frame) {
		Mat frame1;
		GaussianBlur(*frame, frame1, Size(0,0), 3);
		addWeighted(*frame, 1.5, frame1, -0.5, 0, *frame);
		return frame;
    }

};

// This stage applies the Sobel filter and sends the result to the next stage
struct Stage2 : ff_node_t<Mat> {

    Mat *svc(Mat *frame) {
		Sobel(*frame, *frame, -1, 1, 0, 3);
		return frame;
    }
};

// This stage shows the output
struct Drain : ff_node_t<Mat> {

    Drain(bool ovf) : outvideo(ovf) { }

    int svc_init() {
		if (outvideo) namedWindow("edges", 1);
		return 0;
    }

    Mat *svc(Mat *frame) {
		if (outvideo) {
	    	imshow("edges", *frame);
	    	waitKey(30);
		}
		delete frame;
		return GO_ON;
    }

protected:
    const bool outvideo;

};

// This node includes both Gaussian and Sobel filter and it is used for the sequential part of the test
struct SeqNode : ff_node_t<Mat> {

	int svc_init() {
		time_elapsed = 0;
		return 0;
	}

	Mat *svc(Mat *frame) {
		using namespace std::chrono;
		time_point<chrono::system_clock> cstart = system_clock::now();
		Mat frame1;
		GaussianBlur(*frame, frame1, Size(0,0), 3);
		addWeighted(*frame, 1.5, frame1, -0.5, 0, *frame);
		Sobel(*frame, *frame, -1, 1, 0, 3);
		time_point<chrono::system_clock> cend = system_clock::now();
		time_elapsed += ((duration<double, std::milli>) (cend-cstart)).count();
		return frame;
	}

	private:
	double time_elapsed;

	public:
	double ff_time() { return time_elapsed; } // returns total runtime

};

#endif // FF_VIDEO_HPP
