#include <iostream>
#include <vector>
#include <algorithm>
#include <opencv2/opencv.hpp>
#include <dlib/opencv.h>
#include <dlib/gui_widgets.h>
#include <dlib/image_processing/generic_image.h>
#include <dlib/image_transforms.h>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

template<class Functor>
void findObjectRectangle(const cv::Mat& input, std::vector<cv::Rect>& rects, Functor func) {
	dlib::cv_image<dlib::bgr_pixel> dlibimg(input);
	std::vector<dlib::rectangle> dlibrects;
	dlib::find_candidate_object_locations(
		dlibimg,
		dlibrects,
		dlib::linspace(50, 200, 3),
		20 * 20
		);
	for (const auto& rect : dlibrects) {
		if (func(rect, input)) {
			rects.push_back(cv::Rect(rect.left(), rect.top(), rect.width(), rect.height()));
		}
	}
}

void extract_objects(const boost::filesystem::path& input_path, const boost::filesystem::path& output_path) {
	cv::Mat frame = cv::imread(input_path.string());
	std::vector<cv::Rect> rects;
	::findObjectRectangle(
		frame,
		rects,
		[](const dlib::rectangle& rect, const cv::Mat& frame) {
		return rect.width() > 20
			&& rect.height() > 20
			&& rect.width() < 200
			&& rect.height() < 200;
	}
	);
	std::size_t num = 0;
	for (const auto& rect : rects) {
		std::stringstream ss;
		ss << num << ".png";
		cv::imwrite((output_path / ss.str()).string(), frame(rect));
		num++;
	}
}

int main(int argc, char** argv) {
	namespace bp = boost::program_options;
	namespace bf = boost::filesystem;
	bp::options_description general_opt("Allowed Options");
	general_opt.add_options()
		("help,h", "Show help")
		("input,i", bp::value<bf::path>(), "Input image or directory.")
		("output,o", bp::value<bf::path>(), "Output directory.");
	bp::variables_map map;
	try {
		bp::store(bp::parse_command_line(argc, argv, general_opt), map);
		bp::notify(map);
	}
	catch (const bp::error& e) {
		std::cerr << "ERROR:" << e.what() << std::endl;
		return -1;
	}
	if (map.count("help")) {
		std::cout << general_opt;
	}
	if (map.count("input") && map.count("output")) {
		::extract_objects(map["input"].as<bf::path>(), map["output"].as<bf::path>());
	}
	else {
		std::cerr << "ERROR: You must be set 'input' and 'output' options!!." << std::endl;
	}
	return 0;
}