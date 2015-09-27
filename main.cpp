#include <iostream>
#include <opencv2/opencv.hpp>

#include "TimeLapse.hpp"

int main(int argc, char** argv){
	TimeLapse timelapse;
	bool flg = timelapse.open("C:/Users/Takumi/Videos/ichigo");
	std::cout << flg << std::endl;
	cv::namedWindow("TEST");
	cv::Mat frame;
	while (true) {
		timelapse >> frame;
		if (frame.empty()) {
			break;
		}
		cv::imshow("TEST", frame);
		cv::waitKey(33);
	}
    return 0;
}