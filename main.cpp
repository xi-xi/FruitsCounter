#include <iostream>
#include <opencv2/opencv.hpp>

int main(int argc, char** argv){
	cv::VideoCapture video_capture;
	bool flg = video_capture.open("C:/Users/Takumi/Videos/ichigo.avi");
	std::cout << flg << std::endl;
	cv::namedWindow("TEST");
	cv::Mat frame;
	while (true) {
		video_capture >> frame;
		if (frame.empty()) {
			break;
		}
		cv::imshow("TEST", frame);
		cv::waitKey(33);
	}
    return 0;
}