#include "OpenCVAdapter.hpp"
#include "ARTKLinker.h"

#include <gl\glut.h>

using namespace cv;
using namespace std;
//	OpenCV val
VideoCapture cap;
//	ARToolKit Marker Info
#define MARKER_SIZE 26.0f
#define MARKER_NUM	5
ARParam cparam;
ARGL_CONTEXT_SETTINGS_REF gArglSettings;
Marker marker[MARKER_NUM] = { 
	{ "data/marker_0000.pat", -1, 0, 0, MARKER_SIZE, { 0.0, 0.0 } },
	{ "data/marker_0001.pat", -1, 0, 1, MARKER_SIZE, { 0.0, 0.0 } },
	{ "data/marker_0002.pat", -1, 0, 2, MARKER_SIZE, { 0.0, 0.0 } },
	{ "data/marker_0003.pat", -1, 0, 3, MARKER_SIZE, { 0.0, 0.0 } },
	{ "data/marker_0004.pat", -1, 0, 4, MARKER_SIZE, { 0.0, 0.0 } } };

void display();
int setup();
void drawObject(int mark_id, double arTransMat[3][4]);
void keyEvent(unsigned char key, int x, int y);
void reshapeEvent(int w, int h);
void visibilityEvent(int visible);
void idleEvent();

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(640, 480);
	glutCreateWindow(argv[0]);
	// Setup argl library for current context.
	if ((gArglSettings = arglSetupForCurrentContext()) == NULL) {
		fprintf(stderr, "main(): arglSetupForCurrentContext() returned error.\n");
		exit(-1);
	}
	glutDisplayFunc(display);
	glutKeyboardFunc(keyEvent);
	glutReshapeFunc(reshapeEvent);
	glutVisibilityFunc(visibilityEvent);
	if(setup() != 0) return -1;
	glutMainLoop();

	return 0;
}

int setup()
{
	glClearColor(0.0, 0.0, 0.0, 1.0);
	//	�J�����̗p��
	cap = VideoCapture(0);
	if (!cap.isOpened()) return -1;
	//	ARTK�@�p�^�[���t�@�C���̃��[�h�C�e�}�[�J�[�Ƀp�^�[��ID�����蓖��
	for (int i = 0; i < MARKER_NUM; i++)
	{
		if ((marker[i].patt_id = arLoadPatt(marker[i].patt_name)) < 0)
		{
			cout << i << "�Ԗڂ̃p�^�[���t�@�C���̓ǂݍ��݂Ɏ��s" << endl;
			cout << marker[i].patt_name << endl;
			return -1;
		}
	}
	ARParam wparam;
	if (arParamLoad("data/webcam_param.dat", 1, &wparam) < 0)
	{
		cout << "�J�����p�����[�^�̓Ǎ��Ɏ��s���܂���" << endl;
		return -1;
	}
	arParamChangeSize(&wparam, 640, 480, &cparam);
	arInitCparam(&cparam);

	return 0;
}
void display()
{
	Mat frame, srcImg, dstImg, rgbaImg;
	cap >> frame;
	frame.copyTo(srcImg);
	cvtColor(srcImg, rgbaImg, CV_BGR2BGRA);
	ARUint8 *imgData = (ARUint8*)(rgbaImg.data);

	glDrawBuffer(GL_BACK);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	arglDispImage(imgData, &cparam, 1.0, gArglSettings);
	//	�L���v�`���C���[�W��GLUT�ɕ`��
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//	�J�����p�����[�^�����Ƃɓ��e�s������[�h
	GLdouble glProjMat[16];
	arglCameraFrustumRH(&cparam, 0.01, 5000, glProjMat);
	glLoadMatrixd(glProjMat);
	glMatrixMode(GL_MODELVIEW);

	//	AR�}�[�J�[�̔F��
	Mat binary;
	cvtColor(srcImg, binary, CV_BGR2GRAY);
	int thresh = (int)threshold(binary, binary, 100, 255, THRESH_BINARY | THRESH_OTSU);
	ARMarkerInfo *marker_info;
	int marker_num;				//	���������}�[�J�[���̐�

	double f = 1000.0 / cv::getTickFrequency();
	int64 time = cv::getTickCount();

	if (arDetectMarker(imgData, thresh, &marker_info, &marker_num) < 0)
	{
		exit(-1);
	}
	//	�}�[�J�[�̈�v�x��r�@��ނ��ƂɌJ��Ԃ�
	for (int i = 0; i < MARKER_NUM; i++)
	{
		int k = -1;
		for (int j = 0; j < marker_num; j++)
		{
			if (marker[i].patt_id == marker_info[j].id)	//	marker[i]�ƍł���v�xcf�������}�[�J�[�𒊏o
			{
				if (k == -1) k = j;
				else if (marker_info[k].cf < marker_info[j].cf) k = j;
			}
		}
		if (k == -1)		//	�}�[�J�[��������Ȃ�����
		{
			marker[i].visible = 0;
			continue;
		}
		//	���W�ϊ��s��
		if (marker[i].visible == 0)
			arGetTransMat(&marker_info[k], marker[i].patt_center, marker[i].patt_width, marker[i].patt_trans);
		else
			arGetTransMatCont(&marker_info[k], marker[i].patt_trans, marker[i].patt_center, marker[i].patt_width, marker[i].patt_trans);
		marker[i].visible = 1;

		drawObject(marker[i].patt_id, marker[i].patt_trans);
	}
	std::cout << ", time = " << getTickFrequency()/(getTickCount() - time) << " [fps]\r";
	glutSwapBuffers();
}
void idleEvent()
{
	glutPostRedisplay();
}
void visibilityEvent(int visible)
{
	if (visible == GLUT_VISIBLE) {
		glutIdleFunc(idleEvent);
	}
	else {
		glutIdleFunc(NULL);
	}
}
void keyEvent(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27:
		exit(0);
		break;
	}
}
void drawObject(int mark_id, double arTransMat[3][4])
{
	double glTransMat[16];

	glEnable(GL_DEPTH_TEST);	//	�f�v�X�e�X�g�L����
	glDepthFunc(GL_LEQUAL);		//	�f�v�X�e�X�g��r��@��LEQUAL�ɕύX
	
	glMatrixMode(GL_MODELVIEW);
	arglCameraViewRH(arTransMat, glTransMat, 1.0);
	glLoadMatrixd(glTransMat);		//	�}�[�J�[�ʒu�ֈړ�

	switch (mark_id)
	{
	case 0:
		glColor3d(1.0, 0.0, 0.0);
		glTranslatef(0.0, 0.0, 13.0);
		glutSolidCube(26.0);
		break;
	case 1:
		glColor3d(0.0, 1.0, 0.0);
		glTranslatef(0.0, 0.0, 13.0);
		glutSolidCube(26.0);
		break;
	case 2:
		glColor3d(1.0, 1.0, 0.0);
		glTranslatef(0.0, 0.0, 13.0);
		glutSolidCube(26.0);
		break;
	case 3:
		glColor3d(0.0, 0.0, 1.0);
		glTranslatef(0.0, 0.0, 13.0);
		glutSolidCube(26.0);
		break;
	case 4:
		glColor3d(1.0, 0.0, 1.0);
		glTranslatef(0.0, 0.0, 13.0);
		glutSolidCube(26.0);
		break;
	}
	glDisable(GL_DEPTH_TEST);
}

void reshapeEvent(int w, int h)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Call through to anyone else who needs to know about window sizing here.
}