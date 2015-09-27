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
	/**
	 * コンストラクタ。何もしていないけど
	 */
	TimeLapse();
	/**
	 * ディレクトリ名を指定して、そのディレクトリを開きます
	 * \param[in] dirname 開くディレクトリ名
	 */
	TimeLapse(const std::string& dirname);
	~TimeLapse();
	/**
	* ディレクトリ名を指定して、そのディレクトリを開きます
	* \param[in] dirname 開くディレクトリ名
	*/
	bool open(const std::string& dirname);
	/**
	 * ディレクトリを開けているならtrueを返す
	 */
	bool isOpened() const;
	TimeLapse& operator >>(cv::Mat& image) {
		this->read(image);
		return *this;
	}
	/**
	 * 次のフレームとなるフレームを読み出します。
	 * \param[out] image 出力先。読み出しに失敗してればempty
	 */
	bool read(cv::Mat& image);
};
#endif