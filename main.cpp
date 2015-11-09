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
#include "TimeLapse.hpp"

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

cv::Size getHOGWinSize() {
	return cv::Size(128, 64);
}

cv::HOGDescriptor getDefaultHOGDescriptor() {
	return cv::HOGDescriptor(
		::getHOGWinSize(),
		cv::Size(16, 16),
		cv::Size(8, 8),
		cv::Size(8, 8),
		9
	);
}

void calcHOGDescripter(const cv::Mat& img, std::vector<float>& desc) {
	static auto hog = ::getDefaultHOGDescriptor();
	hog.winSize = ::getHOGWinSize();
	cv::Mat gray;
	cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
	cv::resize(gray, gray, ::getHOGWinSize());
	hog.compute(gray, desc, cv::Size(8,8), cv::Size(0,0));
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

void createTraindataLabel(const boost::filesystem::path& positive_path, const boost::filesystem::path& negative_path, cv::Mat& train_data, std::vector<int>& labels) {
	namespace bf = boost::filesystem;
	const int POSITIVE_LABEL = 1;
	const int NEGATIVE_LABEL = -1;
	std::vector<std::vector<float>> features;
	labels.clear();
	for (const auto& entry : bf::directory_iterator(positive_path)) {
		std::cout << "POS_IMAGE:" << entry << std::endl;
		cv::Mat img = cv::imread(entry.path().string());
		if (img.empty())
			continue;
		std::vector<float> desc;
		::calcHOGDescripter(img, desc);
		features.push_back(desc);
		labels.push_back(POSITIVE_LABEL);
	}
	for (const auto& entry : bf::directory_iterator(negative_path)) {
		std::cout << "NEG_IMAGE:" << entry << std::endl;
		cv::Mat img = cv::imread(entry.path().string());
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
	cv::HOGDescriptor detector = ::getDefaultHOGDescriptor();
	std::vector<float> hog_detector;
	::get_svm_detector(svm, hog_detector);
	std::cout << detector.getDefaultPeopleDetector().size() << std::endl;
	std::cout << hog_detector.size() << " " << detector.getDescriptorSize() << std::endl;
	detector.setSVMDetector(hog_detector);
	auto frame = cv::imread("C:/Users/Takumi/Source/FruitsCounter/build/tomato/03600.png");
	std::vector<cv::Rect> rects;
	detector.detectMultiScale(frame, rects);
	for (const auto& rect : rects) {
		cv::rectangle(frame, rect, cv::Scalar(0, 0, 255), 3);
	}
	std::cout << "tomato:" << rects.size() << std::endl;
	cv::namedWindow("T");
	cv::imshow("T", frame);
	cv::waitKey();
	cv::imwrite("detect_test.png", frame);
}

int main(int argc, char** argv){
	namespace bp = boost::program_options;
	namespace bf = boost::filesystem;
	bp::options_description general_opt("Genral Options");
	general_opt.add_options()
		("help,h", "Show help")
		("extract,e", "Use Extract Mode")
		("training,t", "Use Training Mode");
	bp::options_description extract_option("Extract Options");
	extract_option.add_options()
		("input,i", bp::value<bf::path>(), "Input image or directory.")
		("dir,d", bp::value<bf::path>(), "Output directory.");
	bp::options_description train_option("Training Options");
	train_option.add_options()
		("positive,p", bp::value<bf::path>(), "Positive image directory.")
		("negative,n", bp::value<bf::path>(), "Negative image directory.")
		("output,o", bp::value<bf::path>(), "Train data output path.");
	bp::options_description all("Allowed Options");
	all.add(general_opt).add(extract_option).add(train_option);
	bp::variables_map map;
	try {
		bp::store(bp::parse_command_line(argc, argv, all), map);
		bp::notify(map);
	}
	catch (const bp::error& e) {
		std::cerr << "ERROR:" << e.what() << std::endl;
		return -1;
	}
	if (map.count("help")) {
		std::cout << all;
	}
	else if (map.count("extract")) {
		if (map.count("input") && map.count("dir")) {
			::extract_objects(map["input"].as<bf::path>(), map["dir"].as<bf::path>());
		}
		else {
			std::cout << "In Extract mode, you must be set 'input' and 'output' options!!." << std::endl;
		}
	}
	else if (map.count("training")) {
		if (map.count("positive") && map.count("negative") && map.count("output")) {
			::train(map["positive"].as<bf::path>(), map["negative"].as<bf::path>(), map["output"].as<bf::path>());
		}
		else {
			std::cout << "In Training mode, you must be set 'positve', 'negative' and 'output' options!!." << std::endl;
		}
	}
	else {
		std::cout << "Please set a mode !!\n" << all;
	}
    return 0;
}