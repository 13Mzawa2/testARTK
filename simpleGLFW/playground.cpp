
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
//	OpenCV�̃L�����u���[�V�����f�[�^��p����
const char calibDir[] = "./data/calibdata.xml";

//===========================================
//	for ARToolKitPlus Variables
//===========================================
#define MARKER_SIZE 48.0f
//	for OpenCV camera calibration
Mat cameraMatrix, distCoeffs;
Size cameraSize;
Mat mapC1, mapC2;
ARToolKitPlus::TrackerSingleMarker *tracker;
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
	//	�J�����̏�����
	cap = VideoCapture(0);
	if (!cap.isOpened()) return -1;
	cameraSize = Size(cap.get(CV_CAP_PROP_FRAME_WIDTH), cap.get(CV_CAP_PROP_FRAME_HEIGHT));
	//	�J�����p�����[�^�̃��[�h
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

	//	GLFW�̏�����
	if (glfwInit() != GL_TRUE)
	{
		cerr << "GLFW�̏������Ɏ��s���܂����D\n";
		return EXIT_FAILURE;
	}
	//	Window�ݒ�
	glfwWindowHint(GLFW_SAMPLES, 4);								//	4x �A���`�G�C���A�X
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);						//	���T�C�Y�s��
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);					//	OpenGL�o�[�W����3.3�𗘗p
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);					//	
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);	//	�Â�OpenGL���g��Ȃ�

	// Main Window�̗p��
	mainWindow = glfwCreateWindow(cameraSize.width, cameraSize.height, "Main Window", NULL, NULL);
	if (mainWindow == NULL){
		cerr << "GLFW�E�B���h�E�̐����Ɏ��s���܂���. Intel GPU���g�p���Ă���ꍇ��, OpenGL 3.3�Ƒ������ǂ��Ȃ����߁C2.1�������Ă��������D\n";
		glfwTerminate();
		return EXIT_FAILURE;
	}
	//	Main Window Setting
	glfwMakeContextCurrent(mainWindow);				//	main window���J�����g�ɂ���
	glfwSwapInterval(1);				//	SwapBuffer�̃C���^�[�o��
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_LESS);				//	�J�����ɋ߂��ʂ��������_�����O����
	//	�L�[���͂��󂯕t����悤�ɂ���
	glfwSetInputMode(mainWindow, GLFW_STICKY_KEYS, GL_TRUE);

	//----------------------
	//	Launch Rendering Engine
	//----------------------
	loadOBJ(objDir, mainRenderer.obj);	//	OBJ�t�@�C���̏���
	mainRenderer.shader.initGLSL(vertexDir, fragmentDir);	//	�v���O���}�u���V�F�[�_�����[�h
	mainRenderer.texImg = imread(textureDir);	//	�e�N�X�`���摜��ǂݍ���
	mainRenderer.init();

	//	�w�i�摜�`�揀��
	glImg.init(mainWindow);

	//----------------------
	//	Initialize ARToolKitPlus
	//----------------------
	ARToolKitPlus::Camera *param = OpenCVCamera::fromOpenCV(cameraMatrix, distCoeffs, cameraSize);
	tracker = new ARToolKitPlus::TrackerSingleMarkerImpl<6, 6, 6, 1, 10>(cameraSize.width, cameraSize.height);
	//tracker->init("data/LogitechPro4000.dat", 0.1f, 5000.0f);	
	//tracker->changeCameraSize(cameraSize.width, cameraSize.height);
	tracker->init(NULL, 0.1f, 5000.0f);	//	�t�@�C���͎g�p���Ȃ�
	tracker->setCamera(param);
	tracker->activateAutoThreshold(true);
	tracker->setNumAutoThresholdRetries(5);
	tracker->setBorderWidth(0.125f);			//	BCH boader width = 12.5%
	tracker->setPatternWidth(60.0f);			//	marker physical width = 60.0mm
	tracker->setPixelFormat(ARToolKitPlus::PIXEL_FORMAT_BGR);		//	With OpenCV
	tracker->setUndistortionMode(ARToolKitPlus::UNDIST_NONE);		//	Undistortion��OpenCV���ōs��
	tracker->setMarkerMode(ARToolKitPlus::MARKER_ID_BCH);
	tracker->setPoseEstimator(ARToolKitPlus::POSE_ESTIMATOR_RPP);


	//	�^�C�}�[�ݒ�
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
		//	�J��������}�[�J�[�܂�
		static glm::mat4 markerTransMat = glm::mat4(1.0f);
		ARToolKitPlus::ARMarkerInfo *markers;
		int markerID = tracker->calc(frameImg.data, -1, true, &markers);
		float conf = (float)tracker->getConfidence();		//	�M���x
		if (markerID == 4)
		{
			markerTransMat = glm::make_mat4(tracker->getModelViewMatrix());
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
			glm::vec3(0, 0, 0), // �J�����̌��_
			glm::vec3(0, 0, 1), // ���Ă���_
			glm::vec3(0, 1, 0))  // �J�����̏����
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
		if (glfwGetKey(mainWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS		//	Esc�L�[
			|| glfwWindowShouldClose(mainWindow))			//	�E�B���h�E�̕���{�^��
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
//
//#pragma comment(lib, "opengl32.lib")
//#pragma comment(lib, "glew32.lib")
//#pragma comment(lib, "glfw3.lib")
//#include <stdio.h>
//#include <stdlib.h>
//
//#include <GL/glew.h>
//
//#include <GL/glfw3.h>
//GLFWwindow* window;
//
//#include <glm/glm.hpp>
//using namespace glm;
//
//int main(void)
//{
//	// Initialise GLFW
//	if (!glfwInit())
//	{
//		fprintf(stderr, "Failed to initialize GLFW\n");
//		getchar();
//		return -1;
//	}
//
//	glfwWindowHint(GLFW_SAMPLES, 4);
//	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
//	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
//	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//
//	// Open a window and create its OpenGL context
//	window = glfwCreateWindow(1024, 768, "Playground", NULL, NULL);
//	if (window == NULL){
//		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
//		getchar();
//		glfwTerminate();
//		return -1;
//	}
//	glfwMakeContextCurrent(window);
//
//	// Initialize GLEW
//	if (glewInit() != GLEW_OK) {
//		fprintf(stderr, "Failed to initialize GLEW\n");
//		getchar();
//		glfwTerminate();
//		return -1;
//	}
//
//	// Ensure we can capture the escape key being pressed below
//	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
//
//	// Dark blue background
//	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
//
//	do{
//		// Draw nothing, see you in tutorial 2 !
//
//		// Swap buffers
//		glfwSwapBuffers(window);
//		glfwPollEvents();
//
//	} // Check if the ESC key was pressed or the window was closed
//	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
//	glfwWindowShouldClose(window) == 0);
//
//	// Close OpenGL window and terminate GLFW
//	glfwTerminate();
//
//	return 0;
//}
//