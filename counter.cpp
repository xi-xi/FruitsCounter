#include <opencv2/opencv.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include "TimeLapse.hpp"

int main(int argc, char** argv) {
	namespace bp = boost::program_options;
	namespace bf = boost::filesystem;
	bp::options_description general_opt("Genral Options");
	general_opt.add_options()
		("help,h", "Show help")
		("input,i", bp::value<bf::path>()->required(), "Input directory");
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
	cv::namedWindow("FRAME");
	std::size_t count = 0;
	while (lapce.isOpened()) {
		lapce >> frame;
		cv::resize(frame, frame, cv::Size(500, 500));
		cv::imshow("FRAME", frame);
		auto key = cv::waitKey(33);
		if (key == 'q') {
			break;
		}
		if (key == 'w') {
			count++;
			std::cout << count << std::endl;
		}
		else if (key == 's') {
			count--;
			std::cout << count << std::endl;
		}
	}
	std::cout << "TOMATO: " << count << std::endl;
	cv::waitKey();
	return 0;
}