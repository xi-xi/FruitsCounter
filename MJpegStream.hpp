#ifndef __MJPEG_STREAM_HPP__
#define __MJPEG_STREAM__
#include <string>
#include <vector>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <opencv2/core.hpp>
/**
 \sa http://www.computer-vision-software.com/blog/2009/08/cross-platform-solution-for-getting-mjpeg-stream-from-axis-ip-camera-axis-211m/
 \sa http://nekko1119.hatenablog.com/entry/2013/10/02/145532
 \sa http://stackoverflow.com/questions/21702477/how-to-parse-mjpeg-http-stream-from-ip-camera
 */
class MJpegStream {
private:
	const std::size_t REQUEST_SIZE = 2048;
	boost::asio::io_service io_service_;
	boost::system::error_code last_error_code_;
	boost::asio::streambuf request_;
	boost::thread read_thread_;
	boost::mutex image_buf_mutex_;
	std::vector<unsigned char> image_buf_;
	std::size_t currentframe_end_index_ = 0;
	boost::mutex is_connecting_mutex_;
	bool is_connecting_ = false;
	boost::mutex end_init_mutex_;
	bool end_init_ = false;
	void buildRequest(const std::string& host, const std::string& file);
	void beginConnect(const std::string& host, const std::string& file, const std::string& port);
	void readRequestSize();
	int read(boost::asio::ip::tcp::socket& socket);
	void stripAImage();
	void endInit();
public:
	MJpegStream();
	int connect(const std::string& host, const std::string& file, const std::string& port);
	void close();
	bool isConnected();
	cv::Mat readImage();
	MJpegStream& operator >> (cv::Mat& img) {
		img = this->readImage();
		return *this;
	}
	std::string getLastErrorMessage()const;
};
#endif