#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>

void createPanoramaMap(cv::Mat& map_x, cv::Mat& map_y, const cv::Point& center, double radius) {
	const double PI = 3.141592653589793238463;
	cv::Size dst_size = cv::Size(2.0 * PI * radius, radius);
	map_x.create(dst_size, CV_32FC1);
	map_y.create(dst_size, CV_32FC1);
	for (int y = 0; y < dst_size.height; ++y) {
		for (int x = 0; x < dst_size.width; ++x) {
			double theta = static_cast<double>(x) / radius - PI / 2.0;
			map_x.at<float>(y, x) = static_cast<float>(center.x) + y * std::cos(theta);
			map_y.at<float>(y, x) = static_cast<float>(center.y) + y * std::sin(theta);
		}
	}
}

void panorama(const boost::filesystem::path& input, const boost::filesystem::path& output) {
	namespace bf = boost::filesystem;
	typedef std::pair<bf::path, bf::path> IO;
	if (!bf::exists(input)) {
		std::cerr << "ERROR: input file is not exist!" << std::endl;
		return;
	}
	std::vector<IO> io;
	if (bf::is_directory(input)) {
		for (const auto& entry : bf::directory_iterator(input)) {
			auto i = entry.path();
			auto o = bf::path(boost::replace_all_copy(entry.path().string(), input.string(), output.string()));
			io.push_back(IO(i, o));
		}
	}
	else {
		io.push_back(IO(input, output));
	}
	auto sample_img = cv::imread(io[0].first.string());
	if (sample_img.empty()) {
		return;
	}
	auto center = cv::Point(sample_img.cols / 2, sample_img.rows / 2);
	double radius = center.x;
	cv::Mat map_x, map_y;
	::createPanoramaMap(map_x, map_y, center, radius);
	for (const auto& io_pair : io) {
		auto img = cv::imread(io_pair.first.string());
		cv::Mat dst;
		cv::remap(img, dst, map_x, map_y, CV_INTER_LINEAR);
		cv::imwrite(io_pair.second.string(), dst);
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
		::panorama(map["input"].as<bf::path>(), map["output"].as<bf::path>());
	}
	else {
		std::cerr << "ERROR: You must be set 'input' and 'output' options!!." << std::endl;
	}
	return 0;
}