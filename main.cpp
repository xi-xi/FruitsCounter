#include <vector>
#include <map>
#include <cmath>
#include <sstream>
#include <opencv2/opencv.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include "TimeLapse.hpp"
//#define USE_SHOW

typedef cv::Vec3b Pixel;

double tomatoProb(const Pixel& pixel) {
	return std::exp(
			(
				(std::abs(static_cast<double>(pixel[0]) - 90.0) / 90.0 - 1.0)
				- std::abs(static_cast<double>(pixel[1]) - 128.0) / 128.0
				- (255.0 - static_cast<double>(pixel[2])) / 255.0
			) / 3.0
		);
}

void calcTomatoProbability(const cv::Mat& img, cv::Mat& dst) {
	typedef cv::Point3_<uint8_t> Pixel;
	dst.create(img.size(), CV_64FC1);
	cv::Mat_<Pixel> pixmat;
	cv::cvtColor(img, pixmat, cv::COLOR_BGR2HLS);
	pixmat.forEach([&](Pixel& pix, const int location[2]) ->void {
		dst.at<double>(location[0], location[1]) = ::tomatoProb(pix);
	});
}

void resizeAndShow(cv::Mat& frame, const std::string& name, const cv::Size& size = cv::Size(300, 300)) {
	cv::resize(frame, frame, size);
	cv::imshow(name, frame);
}


cv::Point rect2point(const cv::Rect& rect) {
	return cv::Point(rect.x + rect.width / 2, rect.y + rect.height / 2);
}

template<typename COUNTUP_FUNC>
std::size_t getIncrementalTomato(const std::vector<cv::Rect>& previous_tomato, const std::vector<cv::Rect>& current_tomato, const COUNTUP_FUNC& coutup_func) {
	if (previous_tomato.empty() || current_tomato.empty()) {
		return 0;
	}
	std::size_t count = 0;
	for (const auto& cur : current_tomato) {
		double min_dist = std::numeric_limits<double>::infinity();
		std::size_t min_dist_index = 0;
		for (std::size_t i = 0; i < previous_tomato.size(); ++i) {
			const auto& pre = previous_tomato[i];
			const auto& pre_pos = ::rect2point(pre);
			const auto& cur_pos = ::rect2point(cur);
			double distance = std::sqrt(std::pow(pre_pos.x - cur_pos.x, 2) + std::pow(pre_pos.y - cur_pos.y, 2));
			if (min_dist > distance) {
				min_dist = distance;
				min_dist_index = i;
			}
		}
		if (!std::isinf(min_dist)){
			if (coutup_func(::rect2point(cur), ::rect2point(previous_tomato[min_dist_index]))) {
				count++;
			}
		}
	}
	return count;
}

int side(const std::pair<cv::Point, cv::Point>& seg1, const cv::Point& pos){
	cv::Vec3d v1 = cv::Vec3d((seg1.second - seg1.first).x, (seg1.second - seg1.first).y, 0);
	cv::Vec3d v2 = cv::Vec3d((pos - seg1.first).x, (pos - seg1.first).y, 0);
	return v1.cross(v2)[2] > 0 ? 1 : -1;
}

bool isCross(const std::pair<cv::Point, cv::Point>& seg1, const std::pair<cv::Point, cv::Point>& seg2){
	return side(seg1, seg2.first) * side(seg1, seg2.second) < 0
		&& side(seg2, seg1.first) * side(seg2, seg1.second) < 0;
}

double distance(const cv::Point& a, const cv::Point& b) {
	return std::sqrt(std::pow(a.x - b.x, 2) + std::pow(a.y - b.y, 2));
}

int main(int argc, char** argv) {
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
	cv::Mat converted_prob;
	cv::Mat thresh;
	std::vector<std::vector<cv::Point>> contours;
	std::vector<std::vector<cv::Rect>> tomato_rectangles;
	std::size_t tomato_count = 0;
	const double line_rad = 30.0 / 180.0 * 3.1415926535;
	if (!map.count("output")) {
#ifdef USE_SHOW
		cv::namedWindow("W");
		cv::namedWindow("P");
		cv::namedWindow("O");
		cv::namedWindow("F");
#endif
	}
	std::size_t mul = 1;
	while (lapce.isOpened()) {
		lapce >> frame;
		lapce.setCurrentFrame(lapce.currentFrame() + mul);
		::calcTomatoProbability(frame, prob);
		cv::blur(prob, prob, cv::Size(15, 15));
		//      cv::GaussianBlur(prob, prob, cv::Size(9, 9), 0.0);
		prob.convertTo(converted_prob, CV_8U, 255);
		cv::threshold(converted_prob, thresh, 255 * 0.73, 255, CV_THRESH_BINARY);
		cv::erode(thresh, thresh, cv::Mat(), cv::Point(-1, -1), 3);
		cv::dilate(thresh, thresh, cv::Mat(), cv::Point(-1, -1), 3);
		cv::findContours(thresh.clone(), contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
		//cv::drawContours(frame, contours, -1, cv::Scalar(0, 0, 255), 3);
		std::vector<cv::Rect> bounding_rects;
		for (const auto& contour : contours) {
			auto rect = cv::boundingRect(contour);
			bounding_rects.push_back(rect);
		}
		for (const auto& rect : bounding_rects) {
			cv::rectangle(frame, rect, cv::Scalar(255, 0, 0), 5);
		}
		tomato_rectangles.push_back(std::move(bounding_rects));
		if (tomato_rectangles.size() == 1) {
			const int width = frame.cols, height = frame.rows;
			const double radius = std::max(width, height) / 2.0;
			auto inrange_func = [width, height, line_rad, radius](const cv::Point& a) {
				const double x = a.x - width / 2.0;
				const double y = a.y - height / 2.0;
				const double theta = std::atan2(y, x);
				return theta <= -line_rad && theta >= -(3.14159265 - line_rad);
			};
			for (const auto& cur : tomato_rectangles[0]) {
				if (inrange_func(::rect2point(cur))) {
					tomato_count++;
				}
			}
		}
		if (tomato_rectangles.size() >= 2) {
			const int width = frame.cols, height = frame.rows;
			const double radius = std::max(width, height) / 2.0;
			auto countup_func = [width, height, line_rad, radius](const cv::Point& a, const cv::Point& b) {
				return (
					::isCross(
						std::pair<cv::Point, cv::Point>(a, b),
						std::pair<cv::Point, cv::Point>(
							cv::Point(width / 2, height / 2),
							cv::Point(width / 2 + radius * std::cos(line_rad), height / 2 - radius * std::sin(line_rad))
							)
						) || ::isCross(
							std::pair<cv::Point, cv::Point>(a, b),
							std::pair<cv::Point, cv::Point>(
								cv::Point(width / 2, height / 2),
								cv::Point(width / 2 - radius * std::cos(line_rad), height / 2 - radius * std::sin(line_rad))
								)
							)
					) && ::distance(a, b) < 50;
			};
			std::size_t incremt = ::getIncrementalTomato(
				tomato_rectangles[tomato_rectangles.size() - 1],
				tomato_rectangles[tomato_rectangles.size() - 2],
				countup_func
				);
			tomato_count += incremt;
			if (incremt != 0)
			{
				std::cout << lapce.currentFrame() << "," << tomato_count << std::endl;
			}
		}
		std::stringstream tomato_ss;
		tomato_ss << tomato_count;
		cv::putText(frame, tomato_ss.str(), cv::Point(20, 150), cv::FONT_HERSHEY_SIMPLEX, 6.0, cv::Scalar(255, 255, 255), 5);
		if (map.count("output")) {
			auto output_dir = map["output"].as<bf::path>();
			std::stringstream ss;
			ss << lapce.currentFrame() << ".png";
			auto th_output_path = output_dir / "th" / ss.str();
			auto prob_output_path = output_dir / "prob" / ss.str();
			auto frame_output_path = output_dir / "frame" / ss.str();
			prob.convertTo(prob, CV_8UC1, 255);
			cv::Mat prob_small, th_small, frame_small;
			cv::resize(prob, prob_small, cv::Size(), 0.5, 0.5);
			cv::resize(thresh, th_small, cv::Size(), 0.5, 0.5);
			cv::resize(frame, frame_small, cv::Size(), 0.5, 0.5);
			cv::imwrite(prob_output_path.string(), prob_small);
			cv::imwrite(th_output_path.string(), th_small);
			cv::imwrite(frame_output_path.string(), frame_small);
		}
		else {
#ifdef USE_SHOW
			::resizeAndShow(frame, "W", cv::Size(700, 700));
			::resizeAndShow(converted_prob, "P");
			::resizeAndShow(thresh, "O");
#endif
		}
#ifdef USE_SHOW
		auto key = cv::waitKey(33);
		if (key == 'q') {
			break;
		}
		else if (0 <= key - '0' && key - '0' < 10) {
			mul = key - '0';
		}
		else if (key == '-') {
			mul = -1;
		}
#endif
	}
	const int width = frame.cols, height = frame.rows;
	const double radius = std::max(width, height) / 2.0;

	auto inrange_func = [width, height, line_rad, radius](const cv::Point& a) {
		const double x = a.x - width / 2.0;
		const double y = a.y - height / 2.0;
		const double theta = std::atan2(y, x);
		return theta <= -line_rad && theta >= -(3.14159265 - line_rad);
	};
	for (const auto& cur : tomato_rectangles[tomato_rectangles.size() - 1]) {
		if (!inrange_func(::rect2point(cur))) {
			tomato_count++;
		}
	}
	std::cout << "TOMATO: " << tomato_count << std::endl;
	return 0;
}
