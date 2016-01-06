
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
#include <ARToolKitPlus\TrackerSingleMarkerImpl.h>
#include <ARToolKitPlus\TrackerMultiMarkerImpl.h>
#pragma comment(lib, "ARToolKitPlus.lib")
//	Original Libraries
#include "OBJRenderingEngine.h"		//	my simple rendering engine
#include "GLImage.h"		//	Draw OpenCV images in GLFW window
#include "OpenCVCamera.h"	//	OpenCV cameraMatrix -> ARToolKitPlus::Camera

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
const char markerDir[] = "./data/cubemarker_0-4.cfg";

//===========================================
//	for ARToolKitPlus Variables
//===========================================
#define MARKER_SIZE 48.0f
//	for OpenCV camera calibration
Mat cameraMatrix, distCoeffs;
Size cameraSize;
Mat mapC1, mapC2;
ARToolKitPlus::TrackerMultiMarker *tracker;
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
	//	Initialize ARToolKitPlus
	//	 6, 6 - 6x6のパターンドットを使用
	//	 12 - パターン解析時の最大解像度，パターンドットの縦と横の公倍数である必要がある
	//	 8 - ARToolKitPlus内にロードするパターンファイルの最大数
	//	 3 - カメラ内で認識対象とする最大個数
	//----------------------
	ARToolKitPlus::Camera *param = OpenCVCamera::fromOpenCV(cameraMatrix, distCoeffs, cameraSize);
	tracker = new ARToolKitPlus::TrackerMultiMarkerImpl<6, 6, 12, 1, 3>(cameraSize.width, cameraSize.height);
	//tracker->init("data/LogitechPro4000.dat", 0.1f, 5000.0f);	
	//tracker->changeCameraSize(cameraSize.width, cameraSize.height);
	tracker->init(dummyCalibDir, markerDir, 0.1f, 5000.0f);	//	最初のキャリブレーションファイルはダミー
	tracker->setCamera(param);
	tracker->activateAutoThreshold(true);							//	2値化処理の自動閾値を有効化
	tracker->setNumAutoThresholdRetries(3);							//	自動閾値のリトライ数
	tracker->setMarkerMode(ARToolKitPlus::MARKER_ID_BCH);
	tracker->setBorderWidth(0.125f);								//	BCH boader width = 12.5%
	tracker->setPixelFormat(ARToolKitPlus::PIXEL_FORMAT_BGR);		//	With OpenCV
	tracker->setUndistortionMode(ARToolKitPlus::UNDIST_NONE);		//	UndistortionはOpenCV側で行う
	//tracker->setPoseEstimator(ARToolKitPlus::POSE_ESTIMATOR_RPP);


	//	タイマー設定
	double currentTime = 0.0, processTime = 0.0;
	glfwSetTime(0.0);

	//----------------------
	//	Main Loop
	//----------------------
	while (1)
	{
		//------------------------------
		//	Start timer
		currentTime = glfwGetTime();

		//	Get camera image
		cap >> frameImg;
		remap(frameImg, frameImg, mapC1, mapC2, INTER_LINEAR);
		//------------------------------
		//	Detect Marker
		//------------------------------
		//	カメラからマーカーまで
		static glm::mat4 markerTransMat = glm::mat4(1.0f);
		ARToolKitPlus::ARMarkerInfo *markers;
		int numDetected = tracker->calc(frameImg.data);
		markerTransMat = glm::make_mat4(tracker->getModelViewMatrix());
		int *markerIDs;
		tracker->getDetectedMarkers(markerIDs); 
		for (int i = 0; i < tracker->getNumDetectedMarkers(); i++)
		{
			cout << markerIDs[i] << " ";
		}

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
			if (tracker)
				delete tracker;
			tracker = NULL;
			glfwTerminate();
			break;
		}

		//	End of timer
		//-------------------------------
		processTime = glfwGetTime() - currentTime;
		cout << "FPS : " << 1.0 / processTime << "\r";

		glfwPollEvents();
	}
	return EXIT_SUCCESS;
}