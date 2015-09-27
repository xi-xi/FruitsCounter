#ifndef __COLORREDUCTER_HPP__
#define __COLORREDUCTER_HPP__
#include <opencv2/core.hpp>
class ColorReducter {
public:
	cv::Mat operator()(const cv::Mat& src) {
		return this->apply(src);
	}

	cv::Mat apply(const cv::Mat& src);
};
#endif