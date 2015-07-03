#include <AR\ar.h>
#include <AR\video.h>
#include <AR\config.h>
//#include <AR\gsub.h>
#include <AR\gsub_lite.h>
#include <gl\glut.h>

#ifdef _DEBUG
#define AR_EXT "d.lib"
#else
#define AR_EXT ".lib"
#endif

#pragma comment(lib, "libAR" AR_EXT)
#pragma comment(lib, "libARgsub_lite" AR_EXT)
#pragma comment(lib, "libARvideo" AR_EXT)

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "glut32.lib")

//	�}�[�J�[���
typedef struct
{
	char *patt_name;		//	�p�^�[���t�@�C��
	int patt_id;			//	�p�^�[��ID
	int mark_id;			//	�}�[�J�[ID
	int visible;			//	���o�t���O
	double patt_width;		//	�p�^�[���T�C�Y(mm)
	double patt_center[2];	//	�p�^�[���̒��S���W
	double patt_trans[3][4];	//	���W�ϊ��s��
}Marker;