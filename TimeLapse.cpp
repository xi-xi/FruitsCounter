#include "TimeLapse.hpp"

#include <string>
#include <boost/filesystem.hpp>
#include <opencv2/imgcodecs.hpp>

TimeLapse::TimeLapse(){
}

TimeLapse::TimeLapse(const std::string& dirname):TimeLapse(){
	this->open(dirname);
}

TimeLapse::~TimeLapse(){

}

bool TimeLapse::open(const std::string& dirname){
	using namespace boost::filesystem;
	auto dirpath = path(dirname);
	if (!is_directory(dirpath)) {
		return false;
	}
	frame_paths_.clear();
	for (const auto& file : directory_iterator(dirpath)) {
		if (is_regular(file)) {
			this->frame_paths_.push_back(file);
		}
	}
	if (this->frame_paths_.empty()) {
		return false;
	}
	std::sort(this->frame_paths_.begin(), this->frame_paths_.end());
	return true;
}

bool TimeLapse::isOpened()const{
	return this->frame_paths_.size() > current_frame_;
}

bool TimeLapse::read(cv::Mat& image){
	image = cv::imread(this->frame_paths_[this->current_frame_].string());
	this->current_frame_++;
	return !image.empty();
}

bool TimeLapse::read(const std::size_t& frame, cv::Mat& image) {
	image = cv::imread(this->frame_paths_[frame].string());
	return !image.empty();
}

std::size_t TimeLapse::totalFrames()const {
	return this->frame_paths_.size();
}

std::size_t TimeLapse::currentFrame()const {
	return this->current_frame_ - 1;
}

void TimeLapse::setCurrentFrame(const std::size_t& value) {
	this->current_frame_ = value;
}
