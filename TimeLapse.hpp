#ifndef __TIMELAPASE_HPP__
#define __TIMELAPASE_HPP__
#include <string>
#include <vector>
#include <boost/filesystem.hpp>
#include <opencv2/core.hpp>
class TimeLapse {
private:
	std::vector<boost::filesystem::path> frame_paths_;
	std::size_t current_frame_ = 0;
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