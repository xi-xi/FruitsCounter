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

	/**
	 * デストラクタ。何もしてないけど
	 */
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
	 * >> オペレータと同等
	 * \param[out] image 出力先。読み出しに失敗してればempty
	 */
	bool read(cv::Mat& image);

	/**
	 * フレームを指定してフレームを読み出します。
	 * この際、現在フレームは更新されず >> オペレータによって読み出される順には影響しません
	 * \param[in] frame 読みだすフレーム番号0から始まります
	 * \param[out] image 出力先。読み出しに失敗してればempty
	 */
	bool read(const std::size_t& frame, cv::Mat& image);

	/**
	 * 全体のフレーム数
	 */
	std::size_t totalFrames()const;

	/**
	 * 次の >> オペレータによって読み出される
	 * 現在のフレーム番号
	 */
	std::size_t currentFrame()const;

	/**
	 * 次の >> オペレータによって読みだされる番号を変更します。
	 */
	void setCurrentFrame(const std::size_t& value);
};
#endif