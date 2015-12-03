#include <iostream>
#include <vector>
#include <map>
#include <cmath>
#include <sstream>
#include <opencv2/opencv.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include "TimeLapse.hpp"
#include "TomatoInformation.hpp"
#include "TomatoCounter.hpp"

typedef cv::Vec3b Pixel;

double squared_norm(const ::Pixel& a, const ::Pixel& b) {
	return std::pow(static_cast<double>(a[0]) - static_cast<double>(b[0]), 2)
		+ std::pow(static_cast<double>(a[1]) - static_cast<double>(b[1]), 2)
		+ std::pow(static_cast<double>(a[2]) - static_cast<double>(b[2]), 2);
}

double tomatoProb(const Pixel& pixel) {
	Pixel tomato_color = Pixel(0, 0, 255);
	double tomato_variance = 0.1;
	return 60.0 / (std::sqrt(::squared_norm(pixel, tomato_color)) + 1);
}

void calcTomatoProbability(const cv::Mat& img, cv::Mat& dst) {
	dst.create(img.size(), CV_8UC1);
	for (std::size_t y = 0; y < img.rows; ++y) {
		for (std::size_t x = 0; x < img.cols; ++x) {
			dst.at<unsigned char>(y, x) = static_cast<unsigned char>(
				255 * ::tomatoProb(img.at<Pixel>(y, x))
			);
		}
	}
}

void opening(const cv::Mat& input, cv::Mat& output) {
	auto elem = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(15, 15));
	cv::morphologyEx(input, output, cv::MORPH_OPEN, elem);
}

int main(int argc, char** argv){
	const double TOMATO_AREA_THRESH = 400.0;
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
		std::cout << general_opt << std::endl;
		return -1;
	}
	if (map.count("help")) {
		std::cout << general_opt << std::endl;
	}

	auto input_path = map["input"].as<bf::path>();
	TimeLapse lapce;
	lapce.open(input_path.string());
	cv::Mat frame;
	cv::Mat prob;
	cv::Mat opened;
	std::vector<std::vector<cv::Point>> contours;
	std::vector<std::vector<TomatoInformation>> tomatos;
	auto optflow = cv::createOptFlow_DualTVL1();
	TomatoCounter counter;
	if (!map.count("output")) {
		cv::namedWindow("W");
		cv::namedWindow("P");
		cv::namedWindow("O");
	}
	std::size_t mul = 1;
	while (lapce.isOpened()) {
		lapce >> frame;
		lapce.setCurrentFrame(lapce.currentFrame() + mul);
		::calcTomatoProbability(frame, prob);
		cv::GaussianBlur(prob, prob, cv::Size(9, 9), 0.0);
		
		cv::threshold(prob, prob, 90, 255, CV_THRESH_BINARY);
		::opening(prob, opened);
		cv::findContours(opened.clone(), contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
		tomatos.push_back(std::vector<::TomatoInformation>());
		for (const auto& contour : contours) {
			cv::Point2d center(0.0, 0.0);
			for (const auto& c : contour) {
				center.x += static_cast<double>(c.x) / contour.size();
				center.y += static_cast<double>(c.y) / contour.size();
			}
			auto area = cv::contourArea(contour);
			if (area > TOMATO_AREA_THRESH) {
				tomatos[tomatos.size() - 1].push_back(
					::TomatoInformation(lapce.currentFrame(), area, std::move(center))
					);
			}
		}
		counter.update(tomatos[tomatos.size() - 1]);
		for (const auto& tomato : tomatos[tomatos.size() - 1]) {
			cv::circle(frame, tomato.center(), 2, cv::Scalar(255, 0, 0), 5);
		}
		cv::drawContours(frame, contours, -1, cv::Scalar(0, 0, 255), 3);
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
			cv::resize(opened, opened, cv::Size(500, 500));
			cv::imshow("W", frame);
			cv::imshow("P", prob);
			cv::imshow("O", opened);
		}
		auto key = cv::waitKey(33);
		if (key == 'q') {
			break;
		}
		if (0 <= key - '0' && key - '0' < 10) {
			mul = key - '0';
		}
		if (key == '-') {
			mul = -1;
		}
	}
	std::cout << counter.getCount() << std::endl;
//	cv::waitKey();
    return 0;
}