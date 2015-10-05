#include "MJpegStream.hpp"
#include <iostream>
#include <sstream>
#include <opencv2/highgui.hpp>

MJpegStream::MJpegStream() 
	: socket_(io_service_), resolver_(io_service_)
{
}

void MJpegStream::buildRequest(const std::string& host, const std::string& file) {
	std::ostream request_ostream(&this->request_);
	request_ostream << "GET /" << file << " HTTP/1.1\r\n"
		<< "Host: " << host << "\r\n"
		<< "Connection: keep-alive\r\n"
		<< "Cache-Control: max-age=0\r\n"
		<< "Accept: image/webp,image/*,*/*;q=0.8\r\n"
		<< "User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/45.0.2454.101 Safari/537.36\r\n"
		<< "Referer: http://" << host << "/" << file << "\r\n"
		<< "Accept-Encoding: gzip, deflate, sdch\r\n"
		<< "Accept-Language: ja,en-US;q=0.8,en;q=0.6\r\n\r\n";
	//request_ostream << "GET /count.mjpeg HTTP/1.1\r\n"
	//	<< "Host: mjpeg.sanford.io\r\n"
	//	<< "Connection: keep-alive\r\n"
	//	<< "Cache-Control: max-age=0\r\n"
	//	<< "Accept: image/webp, image/*,*/*;q=0.8\r\n"
	//	<< "User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/45.0.2454.101 Safari/537.36\r\n"
	//	<< "Referer: http://mjpeg.sanford.io/count.mjpeg\r\n"
	//	<< "Accept-Encoding: gzip, deflate, sdch\r\n"
	//	<< "Accept-Language: ja,en-US;q=0.8,en;q=0.6\r\n\r\n";
}

int MJpegStream::connect(const std::string& host, const std::string& file, const std::string& port) {
	this->buildRequest(host, file);
	boost::asio::ip::tcp::resolver::query query(
		boost::asio::ip::tcp::v4(),
		host,
		port
		);
	auto it = resolver_.resolve(query, this->last_error_code_);
	if (this->last_error_code_.value()) {
		return this->last_error_code_.value();
	}
	boost::asio::connect(this->socket_, it, this->last_error_code_);
	if (this->last_error_code_.value()) {
		return this->last_error_code_.value();
	}
	boost::asio::write(this->socket_, this->request_);
	if (this->last_error_code_.value()) {
		return this->last_error_code_.value();
	}
	return this->last_error_code_.value();
}

int MJpegStream::read() {
	if (this->last_error_code_.value()) {
		return this->last_error_code_.value();
	}
	std::vector<unsigned char> buf(this->REQUEST_SIZE);
	boost::asio::read(
		this->socket_,
		boost::asio::buffer(buf),
		boost::asio::transfer_exactly(this->REQUEST_SIZE),
		this->last_error_code_
		);
	if (this->last_error_code_
		&& this->last_error_code_ != boost::asio::error::eof) {
	}
	else {
		this->image_buf_.insert(
			this->image_buf_.end(),
			buf.begin(),
			buf.end());
	}
	return this->last_error_code_.value();
}

void MJpegStream::close(){

}

bool MJpegStream::isConnected()const {
	return this->last_error_code_ != 0;
}

cv::Mat MJpegStream::readImage() {
	if (this->image_buf_.size() <= 0) {
		return cv::Mat();
	}
	std::size_t begin_index = 0;
	std::size_t pre_begin_index = 0;
	std::size_t end_index = 0;
	std::size_t pre_end_index = 0;
	for (std::size_t i = 0; i < this->image_buf_.size() - 1; ++i) {
		if (this->image_buf_[i] == 0xff && this->image_buf_[i + 1] == 0xd8) {
			pre_begin_index = i;
		}
		else if (this->image_buf_[i] == 0xff && this->image_buf_[i + 1] == 0xd9) {
			pre_end_index = i;
		}
		if (pre_begin_index < pre_end_index && begin_index < pre_begin_index && end_index < pre_end_index) {
			begin_index = pre_begin_index;
			end_index = pre_end_index;
		}
	}
	if (begin_index == 0 || end_index == 0) {
		return cv::Mat();
	}
	std::vector<unsigned char> buf;
	for (auto i = begin_index; i <= end_index; ++i) {
		buf.push_back(this->image_buf_[i]);
	}
	this->image_buf_.erase(this->image_buf_.begin(), this->image_buf_.begin() + begin_index);
	std::cout << this->image_buf_.size() << std::endl;
	return cv::imdecode(cv::Mat(buf), CV_LOAD_IMAGE_COLOR);
}

std::string MJpegStream::getLastErrorMessage()const {
	return this->last_error_code_.message();
}
