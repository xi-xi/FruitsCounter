#include <iostream>
#include <opencv2/opencv.hpp>
#include <dlib/opencv.h>
#include <dlib/gui_widgets.h>
#include "TimeLapse.hpp"

int main(int argc, char** argv){
	TimeLapse stream;
	if (argc >= 2) {
		stream.open(argv[1]);
	}
	else {
		stream.open("./tomato");
	}
	std::cout << "No Error" << std::endl;
	cv::namedWindow("N");
	cv::Mat frame;
	dlib::image_window dlibw;
	while(stream.isOpened())
	{
		stream >> frame;
		dlib::cv_image<dlib::bgr_pixel> dlibimg(frame);
		if (!frame.empty())
		{
			cv::imshow("N", frame);
		}
		dlibw.clear_overlay();
		dlibw.set_image(dlibimg);
		if (cv::waitKey(33) == 'q') {
			break;
		}
	}
    return 0;
}