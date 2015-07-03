
/************************************************************************************************************/
/*										OpenCV Adapter Ver. 1.4												*/
/*																											*/
/*									viz���C�u�����ɑΉ�(2015/5/12)											*/
/*								�s�N�Z���l�擾�}�N���̕s�����C��(2015/5/7)									*/
/*								OpenCV�p�����J�X�N���v�g 2.2�n�Ή�(2014/4/18)								*/
/*			�g�p���@�Finclude�p�X�Clib�p�X���w�肵���̂��COpenCV���g�������ꏊ�ł������C���N���[�h����	*/
/*																											*/
/************************************************************************************************************/
#pragma once

#include	<iostream>
#include	<opencv2/opencv.hpp>
#include	<opencv2/viz.hpp>

using namespace std;
using namespace cv;

// �o�[�W�����擾
#define CV_VERSION_STR CVAUX_STR(CV_MAJOR_VERSION) CVAUX_STR(CV_MINOR_VERSION) CVAUX_STR(CV_SUBMINOR_VERSION)

// �r���h���[�h
#ifdef _DEBUG
#define CV_EXT_STR "d.lib"
#else
#define CV_EXT_STR ".lib"
#endif

// ���C�u�����̃����N�i�s�v�ȕ��̓R�����g�A�E�g�j
#pragma comment(lib, "opencv_calib3d"		CV_VERSION_STR CV_EXT_STR)		//	�J�����L�����u���[�V�����C�X�e���I�J�����Ȃ�
#pragma comment(lib, "opencv_contrib"		CV_VERSION_STR CV_EXT_STR)		//	�����o�C��F���CMeanShift�Ȃ�
#pragma comment(lib, "opencv_core"			CV_VERSION_STR CV_EXT_STR)		//	�������m�ہE����C�s�񉉎Z�C�`��Ȃ�
#pragma comment(lib, "opencv_features2d"	CV_VERSION_STR CV_EXT_STR)		//	�����ʒ��o
//#pragma comment(lib, "opencv_flann"			CV_VERSION_STR CV_EXT_STR)		//	�����ŋߖT�����iFLANN�j�Ȃ�
//#pragma comment(lib, "opencv_gpu"			CV_VERSION_STR CV_EXT_STR)		//	GPU�𗘗p�����s�񉉎Z�E�摜����
#pragma comment(lib, "opencv_highgui"		CV_VERSION_STR CV_EXT_STR)		//	GUI�C�t�@�C�������C�J�����Ȃ�
#pragma comment(lib, "opencv_imgproc"		CV_VERSION_STR CV_EXT_STR)		//	�摜�����i�q�X�g�O�����C�t�B���^�����O�C�􉽊w�ϊ��C���̒ǐՁC�������o�Ȃǁj
#pragma comment(lib, "opencv_legacy"		CV_VERSION_STR CV_EXT_STR)		//	���֐��Ƃ̌݊��p
//#pragma comment(lib, "opencv_ml"			CV_VERSION_STR CV_EXT_STR)		//	SVM���̋@�B�w�K
//#pragma comment(lib, "opencv_nonfree"		CV_VERSION_STR CV_EXT_STR)		//	SIFT�CSURF���̓����Ɋ֌W�̂������
//#pragma comment(lib, "opencv_objdetect"		CV_VERSION_STR CV_EXT_STR)		//	Haar-Like�CLBP�CHOG���ފ퓙�̃I�u�W�F�N�g���o
//#pragma comment(lib, "opencv_ocl"			CV_VERSION_STR CV_EXT_STR)		//	OpenCL���C�u�������g�p�������񉉎Z
//#pragma comment(lib, "opencv_photo"			CV_VERSION_STR CV_EXT_STR)		//	Computational Photography����̃T�|�[�g�i�摜�C���ȂǁH�j
//#pragma comment(lib, "opencv_stitching"		CV_VERSION_STR CV_EXT_STR)		//	Stitching�i�p�m���}�B�e�Ȃǂ̉摜�����j
//#pragma comment(lib, "opencv_superres"		CV_VERSION_STR CV_EXT_STR)		//	Super-Resolution�i���𑜏����F�ڂ���m�C�Y�̏����Ɏg����j
//#pragma comment(lib, "opencv_ts"			CV_VERSION_STR CV_EXT_STR)		//	���[�U��`�\�ȃ��C�u�����i�e�X�g�쐬�p���W���[���j
#pragma comment(lib, "opencv_video"			CV_VERSION_STR CV_EXT_STR)		//	�r�f�I��́C���[�V�������
//#pragma comment(lib, "opencv_videostab"		CV_VERSION_STR CV_EXT_STR)		//	�r�f�I���艻
#pragma comment(lib, "opencv_viz"  CV_VERSION_STR CV_EXT_STR)				//	3�����f�[�^�CPointCloud

//IplImage�̃s�N�Z���l�E���p�}�N�� �s(Y)���Ƃ̗�(X)�Ԗڂ̔z��ɂȂ��Ă���
#define iplB(IMG,X,Y)		((uchar*)((IMG)->imageData+(IMG)->widthStep*(Y)))[(X)*3]
#define iplG(IMG,X,Y)		((uchar*)((IMG)->imageData+(IMG)->widthStep*(Y)))[(X)*3+1]
#define iplR(IMG,X,Y)		((uchar*)((IMG)->imageData+(IMG)->widthStep*(Y)))[(X)*3+2]
#define iplGRAY(IMG,X,Y)	((uchar*)((IMG)->imageData+(IMG)->widthStep*(Y)))[(X)]

//cv::Mat�̃s�N�Z���l�E���p�}�N��
#define matB(IMG,X,Y)		((IMG).data[((IMG).step*(Y) + (IMG).channels()*(X)) + 0])
#define matG(IMG,X,Y)		((IMG).data[((IMG).step*(Y) + (IMG).channels()*(X)) + 1])
#define matR(IMG,X,Y)		((IMG).data[((IMG).step*(Y) + (IMG).channels()*(X)) + 2])
#define matGRAY(IMG,X,Y)	matB(IMG,X,Y)
#define matBf(IMG,X,Y)		(((Point3f*)((IMG).data + (IMG).step.p[0] * Y))[X].x)
#define matGf(IMG,X,Y)		(((Point3f*)((IMG).data + (IMG).step.p[0] * Y))[X].y)
#define matRf(IMG,X,Y)		(((Point3f*)((IMG).data + (IMG).step.p[0] * Y))[X].z)

