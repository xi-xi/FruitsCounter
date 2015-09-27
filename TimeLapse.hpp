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
	 * �R���X�g���N�^�B�������Ă��Ȃ�����
	 */
	TimeLapse();

	/**
	 * �f�B���N�g�������w�肵�āA���̃f�B���N�g�����J���܂�
	 * \param[in] dirname �J���f�B���N�g����
	 */
	TimeLapse(const std::string& dirname);

	/**
	 * �f�X�g���N�^�B�������ĂȂ�����
	 */
	~TimeLapse();

	/**
	* �f�B���N�g�������w�肵�āA���̃f�B���N�g�����J���܂�
	* \param[in] dirname �J���f�B���N�g����
	*/
	bool open(const std::string& dirname);

	/**
	 * �f�B���N�g�����J���Ă���Ȃ�true��Ԃ�
	 */
	bool isOpened() const;
	TimeLapse& operator >>(cv::Mat& image) {
		this->read(image);
		return *this;
	}

	/**
	 * ���̃t���[���ƂȂ�t���[����ǂݏo���܂��B
	 * >> �I�y���[�^�Ɠ���
	 * \param[out] image �o�͐�B�ǂݏo���Ɏ��s���Ă��empty
	 */
	bool read(cv::Mat& image);

	/**
	 * �t���[�����w�肵�ăt���[����ǂݏo���܂��B
	 * ���̍ہA���݃t���[���͍X�V���ꂸ >> �I�y���[�^�ɂ���ēǂݏo����鏇�ɂ͉e�����܂���
	 * \param[in] frame �ǂ݂����t���[���ԍ�0����n�܂�܂�
	 * \param[out] image �o�͐�B�ǂݏo���Ɏ��s���Ă��empty
	 */
	bool read(const std::size_t& frame, cv::Mat& image);

	/**
	 * �S�̂̃t���[����
	 */
	std::size_t totalFrames()const;

	/**
	 * ���� >> �I�y���[�^�ɂ���ēǂݏo�����
	 * ���݂̃t���[���ԍ�
	 */
	std::size_t currentFrame()const;

	/**
	 * ���� >> �I�y���[�^�ɂ���ēǂ݂������ԍ���ύX���܂��B
	 */
	void setCurrentFrame(const std::size_t& value);
};
#endif