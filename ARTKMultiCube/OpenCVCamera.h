#pragma once
#include <opencv2\opencv.hpp>
#include <ARToolKitPlus\TrackerSingleMarkerImpl.h>

/**
* A class that enables conversion from OpenCV calibration matrices. Derives from Camera
* in order to access the (protected) internals of Camera class.
*/
class OpenCVCamera : ARToolKitPlus::Camera
{
public:
	/**
	* Takes the OpenCV camera matrix and distortion coefficients, and generates
	* ARToolKitPlus compatible Camera.
	*/
	static Camera * fromOpenCV(const cv::Mat& cameraMatrix, const cv::Mat& distCoeffs, cv::Size size)
	{
		Camera *cam = new ARToolKitPlus::CameraImpl;

		cam->xsize = size.width;
		cam->ysize = size.height;

		for (int i = 0; i < 3; i++)
		for (int j = 0; j < 4; j++)
			cam->mat[i][j] = 0;

		float fx = (float)cameraMatrix.at<double>(0, 0);
		float fy = (float)cameraMatrix.at<double>(1, 1);
		float cx = (float)cameraMatrix.at<double>(0, 2);
		float cy = (float)cameraMatrix.at<double>(1, 2);

		cam->mat[0][0] = fx;
		cam->mat[1][1] = fy;
		cam->mat[0][2] = cx;
		cam->mat[1][2] = cy;
		cam->mat[2][2] = 1.0;

		//	OpenCV�̘c�ݕ␳��ARToolKit�̘c�ݕ␳�͑S���ʂ̌v�Z�����g���Ă��邽�߁C�P���ɒl���g���Ȃ�
		//	�����ł�ARToolKit�̘c�݃x�N�g�����[���Ƃ��COpenCV���ŕ␳���Ă�邱�Ƃɂ���
		/*for (int i = 0; i < 4; i++) {
			cam->dist_factor[i] = 0;
		}*/


		for (int i = 0; i < 3; i++){
			for (int j = 0; j < 4; j++)
				std::cout << cam->mat[i][j] << " ";
			std::cout << "\n";
		}
		std::cout << std::endl;
		for (int i = 0; i < 4; i++)
			std::cout << cam->dist_factor[i] << " ";
		std::cout << std::endl;
		std::cout << "(" << cam->xsize << ", " << cam->ysize << ")" << std::endl;

		return cam;
	}
};