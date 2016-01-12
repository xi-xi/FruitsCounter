#include <iostream>
#include <vector>
#include <algorithm>
#include <opencv2/opencv.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include "HogUtil.hpp"

void calcHOGDescripter(const cv::Mat& img, std::vector<float>& desc) {
	static auto hog = ::getDefaultHOGDescriptor();
	hog.winSize = ::getHOGWinSize();
	cv::Mat gray;
	cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
	cv::resize(gray, gray, ::getHOGWinSize());
	hog.compute(gray, desc, cv::Size(8, 8), cv::Size(0, 0));
}

void descriptorToTraindata(const std::vector<std::vector<float>>& descs, cv::Mat& train_data) {
	const std::size_t rows = descs.size();
	const std::size_t cols = descs[0].size();
	train_data = cv::Mat(rows, cols, CV_32FC1);
	auto it = descs.begin();
	for (std::size_t i = 0; it != descs.end(); ++i, ++it) {
		for (std::size_t c = 0; c < (*it).size(); ++c) {
			train_data.row(i).col(c) = (*it)[c];
		}
	}
}

void listDirectoryContents(const boost::filesystem::path& dir, std::vector<boost::filesystem::path>& dst) {
	namespace bf = boost::filesystem;
	for (const auto& entry : bf::directory_iterator(dir)) {
		if (bf::is_directory(entry.path())) {
			::listDirectoryContents(entry.path(), dst);
		}
		if (bf::is_regular_file(entry.path())) {
			dst.push_back(entry.path());
		}
	}
}

void createTraindataLabel(const boost::filesystem::path& positive_path, const boost::filesystem::path& negative_path, cv::Mat& train_data, std::vector<int>& labels) {
	namespace bf = boost::filesystem;
	const int POSITIVE_LABEL = 1;
	const int NEGATIVE_LABEL = -1;
	std::vector<std::vector<float>> features;
	labels.clear();
	std::vector<bf::path> pos_files;
	::listDirectoryContents(positive_path, pos_files);
	for (const auto& f : pos_files) {
		std::cout << "POS_IMAGE:" << f << std::endl;
		cv::Mat img = cv::imread(f.string());
		if (img.empty())
			continue;
		std::vector<float> desc;
		::calcHOGDescripter(img, desc);
		features.push_back(desc);
		labels.push_back(POSITIVE_LABEL);
	}
	std::vector<bf::path> neg_files;
	::listDirectoryContents(negative_path, neg_files);
	for (const auto& f : neg_files) {
		std::cout << "NEG_IMAGE:" << f << std::endl;
		cv::Mat img = cv::imread(f.string());
		if (img.empty())
			continue;
		std::vector<float> desc;
		::calcHOGDescripter(img, desc);
		features.push_back(desc);
		labels.push_back(NEGATIVE_LABEL);
	}
	::descriptorToTraindata(features, train_data);
}

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

void train(const boost::filesystem::path& positive_path, const boost::filesystem::path& negative_path, const boost::filesystem::path& output_path) {
	cv::Mat train_data;
	std::vector<int> labels;
	std::cout << "POS:" << positive_path << std::endl;
	std::cout << "NEG:" << negative_path << std::endl;
	::createTraindataLabel(positive_path, negative_path, train_data, labels);
	cv::Ptr<cv::ml::SVM> svm = cv::ml::SVM::create();
	svm->setCoef0(0.0);
	svm->setDegree(3);
	svm->setTermCriteria(cv::TermCriteria(CV_TERMCRIT_ITER + CV_TERMCRIT_EPS, 1000, 1e-3));
	svm->setGamma(0);
	svm->setKernel(cv::ml::SVM::LINEAR);
	svm->setNu(0.5);
	svm->setP(0.1); // for EPSILON_SVR, epsilon in loss function?
	svm->setC(0.01); // From paper, soft classifier
	svm->setType(cv::ml::SVM::EPS_SVR); // C_SVC; // EPSILON_SVR; // may be also NU_SVR; // do regression task
	svm->train(train_data, cv::ml::ROW_SAMPLE, cv::Mat(labels));
	std::vector<float> result;
	svm->predict(train_data, result);
	for (const auto& r : result) {
		std::cout << r << std::endl;
	}
	svm->save(output_path.string());
}

int main(int argc, char** argv) {
	namespace bp = boost::program_options;
	namespace bf = boost::filesystem;
	bp::options_description general_opt("Allowed Options");
	general_opt.add_options()
		("help,h", "Show help")
		("positive,p", bp::value<bf::path>(), "Positive image directory.")
		("negative,n", bp::value<bf::path>(), "Negative image directory.")
		("output,o", bp::value<bf::path>(), "Train data output path.");
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
	if (map.count("positive") && map.count("negative") && map.count("output")) {
		::train(map["positive"].as<bf::path>(), map["negative"].as<bf::path>(), map["output"].as<bf::path>());
	}
	else {
		std::cerr << "ERROR: You must be set 'positve', 'negative' and 'output' options!!." << std::endl;
	}
	return 0;
}