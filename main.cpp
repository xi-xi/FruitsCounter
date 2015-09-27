#include <iostream>
#include <opencv2/opencv.hpp>
#include "TimeLapse.hpp"

void showHelp() {
	std::cout << "Usage:\n"
		<< "\tmain <input_directory>"
		<< std::endl;
}

int main(int argc, char** argv){
	if (argc <= 1) {
		showHelp();
		return 1;
	}
	TimeLapse timelapse(argv[1]);
	cv::namedWindow("TEST");
	cv::Mat frame;
	while (timelapse.isOpened()) {
		timelapse >> frame;
		cv::imshow("TEST", frame);
		cv::waitKey(33);
	}
    return 0;
}