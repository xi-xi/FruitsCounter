#include <iostream>
#include <opencv2/opencv.hpp>
#include "TimeLapse.hpp"
#include "MJpegStream.hpp"

void showHelp() {
	std::cout << "Usage:\n"
		<< "\tmain <input_directory>"
		<< std::endl;
}

int main(int argc, char** argv){
	MJpegStream stream;
	auto code = stream.connect("172.16.0.254", "", "9176");
	if (code != 0) {
		std::cout << stream.getLastErrorMessage() << std::endl;
	}
	else {
		std::cout << "No Error" << std::endl;
		cv::namedWindow("N");
		cv::Mat frame;
		while (stream.isConnected())
		{
			//std::cout << stream.getLastErrorMessage() << std::endl;
			stream >> frame;
			if (!frame.empty())
			{
				cv::imshow("N", frame);
			}
			if (cv::waitKey(33) == 'q') {
				break;
			}
		}
		std::cout << stream.getLastErrorMessage();
	}
    return 0;
}