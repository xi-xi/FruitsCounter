#include "TomatoInformation.hpp"
#include <cmath>

double TomatoInformation::distance(const TomatoInformation& a, const TomatoInformation& b) {
	return std::abs(a.area() - b.area()) + cv::norm(a.center() - b.center());
}

TomatoInformation::TomatoInformation()
	:_area(.0), _center(.0){

}

TomatoInformation::TomatoInformation(const double& area, const cv::Point2d& center)
	:_area(area), _center(center){
}

const double& TomatoInformation::area() const{
	return this->_area;
}

void TomatoInformation::setArea(const double& val) {
	this->_area = val;
}

const cv::Point2d& TomatoInformation::center()const {
	return this->_center;
}

void TomatoInformation::setCenter(const cv::Point2d& center) {
	this->_center = center;
}

TomatoInformation::~TomatoInformation() {

}