#pragma once

#include <AR\ar.h>
//#include <AR\gsub_lite.h>

#ifdef _DEBUG
#define AR_EXT "d.lib"
#else
#define AR_EXT ".lib"
#endif

#pragma comment(lib, "libAR" AR_EXT)
//#pragma comment(lib, "libARgsub_lite" AR_EXT)

#pragma comment(lib, "opengl32.lib")

//	ARToolKit Marker Information Structure
typedef struct ARTKMarker
{
	char *patt_name;			//	�p�^�[���t�@�C����
	int patt_id;				//	�p�^�[��ID
	int mark_id;				//	�}�[�J�[ID
	int visible;				//	���o�t���O
	double patt_width;			//	�p�^�[���T�C�Y(mm)
	double patt_center[2];		//	�p�^�[���������W
	double patt_trans[3][4];	//	���W�ϊ��s��
};