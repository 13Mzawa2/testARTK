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

//	マーカー情報
typedef struct
{
	char *patt_name;		//	パターンファイル
	int patt_id;			//	パターンID
	int mark_id;			//	マーカーID
	int visible;			//	検出フラグ
	double patt_width;		//	パターンサイズ(mm)
	double patt_center[2];	//	パターンの中心座標
	double patt_trans[3][4];	//	座標変換行列
}Marker;