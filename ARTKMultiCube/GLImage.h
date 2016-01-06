//	OpenGL/GLSL�𗘗p�����摜�`��
//	�V�F�[�_�\�v���O�����͈ȉ��̂��̂��g�p����

/* 
//--- in shader/vertex_drawpix.glsl
#version 330 core
layout(location = 0) in vec4 pv;
void main(void)
{
	gl_Position = pv;
}

//--- in shader/fragment_drawpix.glsl
#version 330 core
uniform sampler2DRect image;
layout(location = 0) out vec4 fc;
void main(void)
{
	fc = texture(image, gl_FragCoord.xy);
}

*/
#pragma once

#include "OpenGLHeader.h"
#include "Shader.h"
#include <opencv2\opencv.hpp>

class GLImage
{
private:
	GLFWwindow *imgWindow;
	GLuint vao;		//	���_�z��I�u�W�F�N�g
	GLuint vbo;		//	���_�o�b�t�@�I�u�W�F�N�g
	GLuint image;	//	�e�N�X�`���I�u�W�F�N�g
	GLuint imageLoc;
	Shader s;
	int vertices;
public:
	GLImage()
	{
	}
	void init(GLFWwindow *window)
	{
		int w, h;
		glfwMakeContextCurrent(window);
		glfwGetWindowSize(window, &w, &h);
		imgWindow = window;

		//glew�̏�����
		GLenum err = glewInit();
		if (err != GLEW_OK){
			printf("Error: %s\n", glewGetErrorString(err));
		}
		// ���_�z��I�u�W�F�N�g
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		// ���_�o�b�t�@�I�u�W�F�N�g
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		// [-1, 1] �̐����`
		static const GLfloat position[][2] =
		{
			{ -1.0f, -1.0f },
			{ 1.0f, -1.0f },
			{ 1.0f, 1.0f },
			{ -1.0f, 1.0f }
		};
		vertices = sizeof(position) / sizeof (position[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(position), position, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		//	�e�N�X�`��
		glGenTextures(1, &image);
		glBindTexture(GL_TEXTURE_RECTANGLE, image);
		glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGB, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

		//	�V�F�[�_�̃��[�h
		s.initGLSL("./shader/vertex_drawpix.glsl", "./shader/fragment_drawpix.glsl");
		imageLoc = glGetUniformLocation(s.program, "image");
	}
	void draw(cv::Mat frame)
	{
		glfwMakeContextCurrent(imgWindow);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// �؂�o�����摜���e�N�X�`���ɓ]������
		cv::flip(frame, frame, 0);
		glBindTexture(GL_TEXTURE_RECTANGLE, image);
		glTexSubImage2D(GL_TEXTURE_RECTANGLE, 0, 0, 0, frame.cols, frame.rows, GL_BGR, GL_UNSIGNED_BYTE, frame.data);

		// �V�F�[�_�v���O�����̎g�p�J�n
		s.enable();

		// uniform �T���v���̎w��
		glUniform1i(imageLoc, 0);

		// �e�N�X�`�����j�b�g�ƃe�N�X�`���̎w��
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_RECTANGLE, image);

		// �`��Ɏg�����_�z��I�u�W�F�N�g�̎w��
		glBindVertexArray(vao);

		// �}�`�̕`��
		glDrawArrays(GL_TRIANGLE_FAN, 0, vertices);

		// ���_�z��I�u�W�F�N�g�̎w�����
		glBindVertexArray(0);

		// �V�F�[�_�v���O�����̎g�p�I��
		s.disable();
	}
};