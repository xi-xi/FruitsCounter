#ifndef __TIMELAPASE_HPP__
#define __TIMELAPASE_HPP__
#include <string>
#include <opencv2/core.hpp>
class TimeLapse {
public:
	TimeLapse();
	TimeLapse(const std::string& dirname);
	~TimeLapse();
	bool open(const std::string& dirname);
	bool isOpened() const;
	TimeLapse& operator >>(cv::Mat& image) {
		this->read(image);
		return *this;
	}
	bool read(cv::Mat& image);
};
#endif