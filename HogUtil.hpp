#ifndef __HOGUTIL_HPP__
#define __HOGUTIL_HPP__
#include <opencv2/opencv.hpp>
cv::Size getHOGWinSize() {
	return cv::Size(128, 64);
}

cv::HOGDescriptor getDefaultHOGDescriptor() {
	return cv::HOGDescriptor(
		::getHOGWinSize(),
		cv::Size(16, 16),
		cv::Size(8, 8),
		cv::Size(8, 8),
		9
	);
}
#endif