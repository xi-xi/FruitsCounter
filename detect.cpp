#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include "HogUtil.hpp"

void get_svm_detector(const cv::Ptr<cv::ml::SVM>& svm, std::vector< float > & hog_detector)
{
	cv::Mat sv = svm->getSupportVectors();
	const int sv_total = sv.rows;
	// get the decision function
	cv::Mat alpha, svidx;
	double rho = svm->getDecisionFunction(0, alpha, svidx);
	CV_Assert(alpha.total() == 1 && svidx.total() == 1 && sv_total == 1);
	CV_Assert((alpha.type() == CV_64F && alpha.at<double>(0) == 1.) ||
		(alpha.type() == CV_32F && alpha.at<float>(0) == 1.f));
	CV_Assert(sv.type() == CV_32F);
	hog_detector.clear();
	hog_detector.resize(sv.cols + 1);
	memcpy(&hog_detector[0], sv.ptr(), sv.cols*sizeof(hog_detector[0]));
	hog_detector[sv.cols] = (float)-rho;
}

void detect(const boost::filesystem::path& input, const boost::filesystem::path& cascade, const boost::filesystem::path& output) {
	auto detector = getDefaultHOGDescriptor();
	cv::Ptr<cv::ml::SVM> svm = cv::ml::SVM::load<cv::ml::SVM>(cascade.string());
	std::vector<float> hog_detector;
	::get_svm_detector(svm, hog_detector);
	detector.setSVMDetector(hog_detector);
	auto frame = cv::imread(input.string());
	std::vector<cv::Rect> rects;
	detector.detectMultiScale(frame, rects);
	std::size_t index = 0;
	for (const auto& rect : rects) {
		std::stringstream ss;
		ss << (output / "res_").string() << index << ".png";
		cv::imwrite(ss.str(), frame(rect));
		index++;
	}
	for (const auto& rect : rects) {
		cv::rectangle(frame, rect, cv::Scalar(0, 0, 255), 3);
	}
}

int main(int argc, char** argv) {
	namespace bp = boost::program_options;
	namespace bf = boost::filesystem;
	bp::options_description general_opt("Allowed Options");
	general_opt.add_options()
		("help,h", "Show help")
		("input,i", bp::value<bf::path>(), "Input image path.")
		("cascade,c", bp::value<bf::path>(), "Cascade file(.yaml format)")
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
	if (map.count("input") && map.count("output") && map.count("cascade")) {
		::detect(map["input"].as<bf::path>(), map["cascade"].as<bf::path>(), map["output"].as<bf::path>());
	}
	else {
		std::cerr << "ERROR: You must be set 'input' and 'output' options!!." << std::endl;
	}
	return 0;
}