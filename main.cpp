#include <iostream>
#include <opencv2/opencv.hpp>

#include "TimeLapse.hpp"

int main(int argc, char** argv){
	TimeLapse timelapse("C:/Users/Takumi/Videos/ichigo");
	cv::namedWindow("TEST");
	cv::Mat frame;
	while (timelapse.isOpened()) {
		timelapse >> frame;
		cv::imshow("TEST", frame);
		cv::waitKey(16);
	}
    return 0;
}