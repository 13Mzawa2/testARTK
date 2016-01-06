
#pragma region Disable Warning C4996
//
// Disable Warning C4996
//
#ifndef _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#endif
#ifndef _CRT_SECURE_CPP_OVERLOAD_SECURE_NAMES
#define _CRT_SECURE_CPP_OVERLOAD_SECURE_NAMES 1
#endif
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS 1
#endif
#pragma endregion
//===========================================
//	Includes
//===========================================
//	Library Linker Scripts
#include "OpenCV3Linker.h"
#include "OpenGLHeader.h"
#include <AR/ar.h>
#include <AR/arMulti.h>
#ifdef _DEBUG
#define AR_EXT "d.lib"
#else
#define AR_EXT ".lib"
#endif
#pragma comment(lib, "AR" AR_EXT)
#pragma comment(lib, "ARICP" AR_EXT)
#pragma comment(lib, "ARMulti" AR_EXT)
//	Original Libraries
#include "OBJRenderingEngine.h"		//	my simple rendering engine
#include "GLImage.h"		//	Draw OpenCV images in GLFW window

using namespace cv;
using namespace std;

//===========================================
//	import file path
//===========================================
//	Shader file
const char vertexDir[] = "./shader/vertex.glsl";
const char fragmentDir[] = "./shader/fragment.glsl";
//	.obj Wavefront Object
const char objDir[] = "./data/CalibBox/CalibBox.obj";
const char textureDir[] = "./data/CalibBox/textures/txt_001_diff.bmp";
//	Camera Calibration File
//	OpenCVのキャリブレーションデータを用いる
const char calibDir[] = "./data/calibdata.xml";
const char dummyCalibDir[] = "./data/LogitechPro4000.dat";
//	MultiMarker Setting File
const char markerConfigDir[] = "./data/CubeMarker/cubemarker_artk5.dat";

//===========================================
//	for ARToolKit Variables
//===========================================
#define MARKER_SIZE 40.0f
//	for OpenCV camera calibration
Mat cameraMatrix, distCoeffs;
Size cameraSize;
Mat mapC1, mapC2;
VideoCapture cap;
Mat frameImg;

//===========================================
//	OpenGL/GLSL Simple Object Rendering Engine
//===========================================
OBJRenderingEngine mainRenderer;
GLImage glImg;
glm::mat4 Model, View, Projection;
GLFWwindow *mainWindow;

//===========================================
//	Prototypes
//===========================================



//===========================================
//	Main Program
//===========================================

int main(void)
{
	//----------------------
	//	Inititalization
	//----------------------
	//	カメラの初期化
	cap = VideoCapture(0);
	if (!cap.isOpened()) return -1;
	cameraSize = Size(cap.get(CV_CAP_PROP_FRAME_WIDTH), cap.get(CV_CAP_PROP_FRAME_HEIGHT));
	//	カメラパラメータのロード
	FileStorage fs(calibDir, FileStorage::READ);
	FileNode Camera = fs["Camera"];
	Camera["size"] >> cameraSize;
	Camera["CameraMatrix"] >> cameraMatrix;
	Camera["DistCoeffs"] >> distCoeffs;
	//	Undistort Map
	initUndistortRectifyMap(
		cameraMatrix, distCoeffs,
		Mat(), cameraMatrix, cameraSize, CV_32FC1,
		mapC1, mapC2);

	//	GLFWの初期化
	if (glfwInit() != GL_TRUE)
	{
		cerr << "GLFWの初期化に失敗しました．\n";
		return EXIT_FAILURE;
	}
	//	Window設定
	glfwWindowHint(GLFW_SAMPLES, 4);								//	4x アンチエイリアス
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);						//	リサイズ不可
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);					//	OpenGLバージョン3.3を利用
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);					//	
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);	//	古いOpenGLを使わない

	// Main Windowの用意
	mainWindow = glfwCreateWindow(cameraSize.width, cameraSize.height, "Main Window", NULL, NULL);
	if (mainWindow == NULL){
		cerr << "GLFWウィンドウの生成に失敗しました. Intel GPUを使用している場合は, OpenGL 3.3と相性が良くないため，2.1を試してください．\n";
		glfwTerminate();
		return EXIT_FAILURE;
	}
	//	Main Window Setting
	glfwMakeContextCurrent(mainWindow);				//	main windowをカレントにする
	glfwSwapInterval(1);				//	SwapBufferのインターバル
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_LESS);				//	カメラに近い面だけレンダリングする
	//	キー入力を受け付けるようにする
	glfwSetInputMode(mainWindow, GLFW_STICKY_KEYS, GL_TRUE);

	//----------------------
	//	Launch Rendering Engine
	//----------------------
	loadOBJ(objDir, mainRenderer.obj);	//	OBJファイルの準備
	mainRenderer.shader.initGLSL(vertexDir, fragmentDir);	//	プログラマブルシェーダをロード
	mainRenderer.texImg = imread(textureDir);	//	テクスチャ画像を読み込む
	mainRenderer.init();

	//	背景画像描画準備
	glImg.init(mainWindow);

	//----------------------
	//	Initialize ARToolKit
	//----------------------
	//	Set camera parameters and make ARToolKit handles
	ARParam cparam;
	arParamLoad(dummyCalibDir, 1, &cparam);
	arParamChangeSize(&cparam, cameraSize.width, cameraSize.height, &cparam);
	cparam.xsize = cameraSize.width;
	cparam.ysize = cameraSize.height;
	cparam.mat[0][0] = cameraMatrix.at<double>(0, 0);
	cparam.mat[1][1] = cameraMatrix.at<double>(1, 1);
	cparam.mat[0][2] = cameraMatrix.at<double>(0, 2);
	cparam.mat[1][2] = cameraMatrix.at<double>(1, 2);
	cparam.mat[2][2] = 1.0;
	ARParamLT *cparamLT = arParamLTCreate(&cparam, AR_PARAM_LT_DEFAULT_OFFSET);
	ARHandle *arhandle = arCreateHandle(cparamLT);
	arSetPixelFormat(arhandle, AR_PIXEL_FORMAT_BGR);
	arSetDebugMode(arhandle, AR_DEBUG_DISABLE);
	AR3DHandle *ar3dhandle = ar3DCreateHandle(&cparam);
	//	Setup Cube Marker
	ARPattHandle *pattHandle = arPattCreateHandle();
	ARMultiMarkerInfoT *multiConfig = arMultiReadConfigFile(markerConfigDir, pattHandle);
	if (multiConfig->patt_type == AR_MULTI_PATTERN_DETECTION_MODE_TEMPLATE) {
		arSetPatternDetectionMode(arhandle, AR_TEMPLATE_MATCHING_COLOR);
	}
	else if (multiConfig->patt_type == AR_MULTI_PATTERN_DETECTION_MODE_MATRIX) {
		arSetPatternDetectionMode(arhandle, AR_MATRIX_CODE_DETECTION);
	}
	else { // AR_MULTI_PATTERN_DETECTION_MODE_TEMPLATE_AND_MATRIX
		arSetPatternDetectionMode(arhandle, AR_TEMPLATE_MATCHING_COLOR_AND_MATRIX);
	}
	arPattAttach(arhandle, pattHandle);
	arSetBorderSize(arhandle, 0.125);

	//	タイマー設定
	double currentTime = 0.0, processTime = 0.0;
	glfwSetTime(0.0);

	//----------------------
	//	Main Loop
	//----------------------
	while (1)
	{

		//	Get camera image
		cap >> frameImg;
		remap(frameImg, frameImg, mapC1, mapC2, INTER_LINEAR);
		//------------------------------
		//	Start timer
		currentTime = glfwGetTime();
		//------------------------------
		//	Detect Cube Marker
		//------------------------------
		//	カメラからマーカーまで
		static glm::mat4 markerTransMat = glm::mat4(1.0f);
		static int patt_found = FALSE;
		if(arDetectMarker(arhandle, (ARUint8*)frameImg.data)<0)break;
		arGetTransMatMultiSquare(ar3dhandle, arGetMarker(arhandle), arGetMarkerNum(arhandle), multiConfig);
		if (multiConfig->prevF != 0)
		{
			patt_found = TRUE;
			double m_modelview[16];
			double para[3][4];
			for (int k = 0; k < 3; k++) {
				for (int j = 0; j < 4; j++) {
					para[k][j] = multiConfig->trans[k][j];
				}
			}
			m_modelview[0 + 0 * 4] = para[0][0]; // R1C1
			m_modelview[0 + 1 * 4] = para[0][1]; // R1C2
			m_modelview[0 + 2 * 4] = para[0][2];
			m_modelview[0 + 3 * 4] = para[0][3];
			m_modelview[1 + 0 * 4] = para[1][0]; // R2
			m_modelview[1 + 1 * 4] = para[1][1];
			m_modelview[1 + 2 * 4] = para[1][2];
			m_modelview[1 + 3 * 4] = para[1][3];
			m_modelview[2 + 0 * 4] = para[2][0]; // R3
			m_modelview[2 + 1 * 4] = para[2][1];
			m_modelview[2 + 2 * 4] = para[2][2];
			m_modelview[2 + 3 * 4] = para[2][3];
			m_modelview[3 + 0 * 4] = 0.0;
			m_modelview[3 + 1 * 4] = 0.0;
			m_modelview[3 + 2 * 4] = 0.0;
			m_modelview[3 + 3 * 4] = 1.0;
			markerTransMat = glm::make_mat4(m_modelview);
		}
		else
		{
			patt_found = FALSE;
		}
		//	End of timer
		//-------------------------------
		processTime = glfwGetTime() - currentTime;
		cout << "FPS : " << 1.0 / processTime << "\r";

		//------------------------------
		//	Draw Main Winodw
		//------------------------------
		glfwMakeContextCurrent(mainWindow);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//	Draw Image
		glImg.draw(frameImg);
		glClear(GL_DEPTH_BUFFER_BIT);

		//	Draw OBJ file
		//	Projection Matrix
		Projection = cvtCVCameraParam2GLProjection(cameraMatrix, cameraSize, 0.1, 5000);
		//	Camera View Matrix
		View = glm::mat4(1.0)
			* glm::lookAt(
			glm::vec3(0, 0, 0), // カメラの原点
			glm::vec3(0, 0, 1), // 見ている点
			glm::vec3(0, 1, 0))  // カメラの上方向
			;
		//	Model Matrix
		glm::mat4 marker2model = glm::mat4(1.0)
			* glm::translate(glm::vec3(0.0f, 0.0, 20.0))
			* glm::rotate(glm::mat4(1.0), (float)(180.0f*CV_PI / 180.0f), glm::vec3(0.0, 1.0, 0.0))
			//* glm::scale(glm::vec3(1.0, 1.0, 1.0))
			;
		Model = glm::mat4(1.0)
			* markerTransMat
			* marker2model;

		//	Render Object
		//	Our ModelViewProjection : multiplication of our 3 matrices
		mainRenderer.shader.enable();
		mainRenderer.MV = View * Model;
		mainRenderer.MVP = Projection * mainRenderer.MV;
		mainRenderer.lightDirection = glm::vec3(markerTransMat[3]);
		mainRenderer.lightColor = glm::vec3(1.0, 1.0, 1.0);
		mainRenderer.render();

		glfwSwapBuffers(mainWindow);

		//------------------------------
		//	Key Events
		//------------------------------
		if (glfwGetKey(mainWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS		//	Escキー
			|| glfwWindowShouldClose(mainWindow))			//	ウィンドウの閉じるボタン
		{
			arPattDetach(arhandle);
			arPattDeleteHandle(pattHandle);
			ar3DDeleteHandle(&ar3dhandle);
			arDeleteHandle(arhandle);
			arParamLTFree(&cparamLT);
			glfwTerminate();
			break;
		}


		glfwPollEvents();
	}
	return EXIT_SUCCESS;
}