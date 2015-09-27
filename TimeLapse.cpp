#include "TimeLapse.hpp"

#include <string>
#include <filesystem>
#include <opencv2/imgcodecs.hpp>

TimeLapse::TimeLapse(){
}

TimeLapse::TimeLapse(const std::string& dirname) {
}

TimeLapse::~TimeLapse() {

}

bool TimeLapse::open(const std::string& dirname) {
	return true;
}

bool TimeLapse::isOpened()const {
	return true;
}

bool TimeLapse::read(cv::Mat& image) {
	return true;
}
