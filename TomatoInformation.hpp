#ifndef __TOMATO_INFORMATION_HPP__
#define __TOMATO_INFORMATION_HPP__
#include <opencv2/core.hpp>
class TomatoInformation
{
public:
	static double distance(const TomatoInformation& a, const TomatoInformation& b);
	TomatoInformation();
	TomatoInformation(const double& area, const cv::Point2d& center);
	~TomatoInformation();
	const double& area()const;
	void setArea(const double& val);
	const cv::Point2d& center()const;
	void setCenter(const cv::Point2d& val);
private:
	double _area;
	cv::Point2d _center;
};
#endif