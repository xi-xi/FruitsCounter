#include <iostream>
#include <vector>
#include <cmath>
#include <sstream>
#include <opencv2/opencv.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include "TimeLapse.hpp"

typedef cv::Vec3b Pixel;

double gaussian(double squared_norm, double variance) {
	const double PI = 3.141592653589793238463;
	return std::exp((-1.0) * squared_norm / variance / 2.0) / std::sqrt(2.0 * PI * variance);
}

double squared_norm(const ::Pixel& a, const ::Pixel& b) {
	return std::pow(static_cast<double>(a[0]) - static_cast<double>(b[0]), 2)
		+ std::pow(static_cast<double>(a[1]) - static_cast<double>(b[1]), 2)
		+ std::pow(static_cast<double>(a[2]) - static_cast<double>(b[2]), 2);
}

double tomatoProb(const Pixel& pixel) {
	Pixel tomato_color = Pixel(0, 0, 255);
	double tomato_variance = 0.1;
	return 60.0 / (std::sqrt(::squared_norm(pixel, tomato_color)) + 1);
	//return gaussian(::squared_norm(pixel, tomato_color) / 255.0 / 3.0, tomato_variance);
	/*return std::exp(-(1.0 / 90.0) * static_cast<double>(std::max({
		std::abs(static_cast<int>(pixel[0]) - static_cast<int>(tomato_color[0])),
		std::abs(static_cast<int>(pixel[1]) - static_cast<int>(tomato_color[1])),
		std::abs(static_cast<int>(pixel[2]) - static_cast<int>(tomato_color[2])),
	})));*/
}

void calcTomatoProbability(const cv::Mat& img, cv::Mat& dst) {
	dst.create(img.size(), CV_8UC1);
	for (std::size_t y = 0; y < img.rows; ++y) {
		for (std::size_t x = 0; x < img.cols; ++x) {
			dst.at<unsigned char>(y, x) = static_cast<unsigned char>(
				255 * ::tomatoProb(img.at<Pixel>(y, x))
			);
			//std::cout << ::tomatoProb(img.at<Pixel>(y, x)) << std::endl;
		}
	}
}

static void onMouseMove(int event, int x, int y, int flags, void *userdata) {
	if (cv::EVENT_MOUSEMOVE != event) {
		return;
	}
	cv::Mat* frame_ptr = static_cast<cv::Mat*>(userdata);
	std::stringstream ss;
	ss << static_cast<int>(frame_ptr->at<unsigned char>(y, x));
	cv::setWindowTitle("P", ss.str());
}

int main(int argc, char** argv){
	namespace bp = boost::program_options;
	namespace bf = boost::filesystem;
	bp::options_description general_opt("Genral Options");
	general_opt.add_options()
		("help,h", "Show help")
		("input,i", bp::value<bf::path>()->required(), "Input directory")
		("output,o", bp::value<bf::path>(), "Output directory");
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

	auto input_path = map["input"].as<bf::path>();
	TimeLapse lapce;
	lapce.open(input_path.string());
	cv::Mat frame;
	cv::Mat prob;
	if (!map.count("output")) {
		cv::namedWindow("W");
		cv::namedWindow("P");
		cv::setMouseCallback("P", ::onMouseMove, &prob);
	}
	std::size_t mul = 1;
	while (lapce.isOpened()) {
		lapce >> frame;
		lapce.setCurrentFrame(lapce.currentFrame() + mul);
//		lapce.read(3600, frame);
		::calcTomatoProbability(frame, prob);
		cv::threshold(prob, prob, 80, 255, CV_THRESH_BINARY);
//		std::cout << prob << std::endl;
		if (map.count("output")) {
			auto output_dir = map["output"].as<bf::path>();
			std::stringstream ss;
			ss << lapce.currentFrame() << ".png";
			auto output_path = output_dir / ss.str();
			cv::imwrite(output_path.string(), prob);
		}
		else {
			cv::resize(frame, frame, cv::Size(500, 500));
			cv::resize(prob, prob, cv::Size(500, 500));
			cv::imshow("W", frame);
			cv::imshow("P", prob);
		}
		auto key = cv::waitKey(33);
		if (key == 'q') {
			break;
		}
		if (0 <= key - '0' && key - '0' < 10) {
			mul = key - '0';
		}
	}

    return 0;
}