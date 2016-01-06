#pragma once
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
//	OpenGL/GLSL Simple Object Rendering Engine
//===========================================
#define GLSL_LOCATION_VERTEX	0
#define GLSL_LOCATION_UV		1
#define GLSL_LOCATION_NORMAL	2
//===========================================
//	Includes
//===========================================
//	Library Linker Scripts
#include "OpenCV3Linker.h"
#include "OpenGLHeader.h"
//	Original Libraries
#include "Shader.h"			//	Shader Class
#include "objloader.hpp"	//	Load OBJ file

class OBJRenderingEngine
{
protected:
	////	uniform IDs
	//	in vertex.glsl
	GLuint mvpID;			//	uniform mat4 MVP;
	GLuint mvID;			//	uniform mat4 MV;
	//	in fragment.glsl
	GLuint textureSamplerID;		//	uniform sampler2D myTextureSampler;
	GLuint lightDirectionID;		//	uniform vec3 LightDirection;
	GLuint lightColorID;			//	uniform vec3 LightColor;
	////	object buffers
	GLuint vertexArray;		//	���_����ێ�����z��
	GLuint vertexBuffer;	//	location = 0
	GLuint uvBuffer;		//	location = 1
	GLuint normalBuffer;	//	location = 2
	GLuint textureObject;	//	�e�N�X�`���ɃA�N�Z�X���邽�߂̃I�u�W�F�N�g

public:
	//	public variables
	Object obj;
	Shader shader;
	cv::Mat texImg;
	//	uniform variables
	glm::mat4 MVP;
	glm::mat4 MV;
	glm::vec3 lightDirection;
	glm::vec3 lightColor;

	OBJRenderingEngine()
	{
	}

	~OBJRenderingEngine()
	{
	}

	//	�R�s�[�R���X�g���N�^
	//	obj, shader, texImg�����ɓǂݍ��܂�Ă��邱�Ƃ��O��
	OBJRenderingEngine &operator=(OBJRenderingEngine &r)
	{
		obj = r.obj;
		shader = r.shader;
		texImg = r.texImg.clone();
		init();
		
		return *this;
	}

	//	Get GLSL Uniform variable IDs
	//	�V�F�[�_�v���O������Uniform�ϐ���ǉ�����ꍇ�͂����œo�^
	void getUniformID()
	{
		mvpID = glGetUniformLocation(shader.program, "MVP");
		mvID = glGetUniformLocation(shader.program, "MV");
		textureSamplerID = glGetUniformLocation(shader.program, "myTextureSampler");
		lightDirectionID = glGetUniformLocation(shader.program, "LightDirection");
		lightColorID = glGetUniformLocation(shader.program, "LightColor");
	}

	void setObjectTexture()
	{
		//	�e�N�X�`���摜��ǂݍ���
		glGenTextures(1, &textureObject);
		glBindTexture(GL_TEXTURE_2D, textureObject);
		//	OpenGL�ɉ摜��n��
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
			texImg.cols, texImg.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, texImg.data);
		//	�e�N�X�`���̌J��Ԃ��ݒ�
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		//	�摜���g��(MAGnifying)����Ƃ��͐��`(LINEAR)�t�B���^�����O���g�p
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//	�摜���k��(MINifying)����Ƃ��A���`(LINEAR)�t�B���^�����A��̃~�b�v�}�b�v����`(LINEARYLY)�ɍ��������̂��g�p
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		//	�~�b�v�}�b�v���쐬
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	void setObjectVertices()
	{
		//	���_�z��I�u�W�F�N�g��ݒ�
		glGenVertexArrays(1, &vertexArray);
		glBindVertexArray(vertexArray);

		//	���_�o�b�t�@��OpenGL�ɓn��
		glGenBuffers(1, &vertexBuffer);							//	�o�b�t�@��1�쐬
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);			//	�ȍ~�̃R�}���h��vertexbuffer�o�b�t�@�Ɏw��
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * obj.vertices.size(), &(obj).vertices[0], GL_STATIC_DRAW);		//	���_��OpenGL��vertexbufer�ɓn��

		//	UV���W�o�b�t�@
		glGenBuffers(1, &uvBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * obj.uvs.size(), &(obj).uvs[0], GL_STATIC_DRAW);

		//	�@���o�b�t�@
		glGenBuffers(1, &normalBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * obj.normals.size(), &(obj).normals[0], GL_STATIC_DRAW);
	}

	//	���ɓǂݍ��܂�Ă���obj, shader, texImg���g���ď�����
	void init()
	{
		getUniformID();
		setObjectVertices();
		setObjectTexture();
	}

	//	�`�施��
	void render()
	{
		////	Execute Rendering
		// ���݃o�C���h���Ă���V�F�[�_��uniform�ϐ��ɕϊ��𑗂�
		// �����_�����O���郂�f�����ƂɎ��s
		glUniformMatrix4fv(mvpID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(mvID, 1, GL_FALSE, &MV[0][0]);
		glUniform3fv(lightDirectionID, 1, &lightDirection[0]);
		glUniform3fv(lightColorID, 1, &lightColor[0]);

		//	�e�N�X�`�����j�b�g0��textureBuffer���o�C���h
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureObject);
		//	0�Ԗڂ̃e�N�X�`�����j�b�g��"myTextureSampler"�ɃZ�b�g
		glUniform1i(textureSamplerID, 0);

		//	�ŏ��̑����o�b�t�@�F���_
		glEnableVertexAttribArray(GLSL_LOCATION_VERTEX);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glVertexAttribPointer(
			GLSL_LOCATION_VERTEX,	// shader����location
			3,						// �v�f�T�C�Y
			GL_FLOAT,				// �v�f�̌^
			GL_FALSE,				// ���K���H
			0,						// �X�g���C�h
			(void*)0				// �z��o�b�t�@�I�t�Z�b�g
			);
		//	2�Ԗڂ̑����o�b�t�@ : UV
		glEnableVertexAttribArray(GLSL_LOCATION_UV);
		glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
		glVertexAttribPointer(GLSL_LOCATION_UV, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
		//	3�Ԗڂ̑����o�b�t�@ : �@��
		glEnableVertexAttribArray(GLSL_LOCATION_NORMAL);
		glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
		glVertexAttribPointer(GLSL_LOCATION_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		//	�O�p�`�|���S����`��
		glDrawArrays(GL_TRIANGLES, 0, obj.vertices.size());
		//	�`���Ƀo�b�t�@���N���A
		glDisableVertexAttribArray(GLSL_LOCATION_VERTEX);
		glDisableVertexAttribArray(GLSL_LOCATION_UV);
		glDisableVertexAttribArray(GLSL_LOCATION_NORMAL);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindTexture(GL_TEXTURE_3D, 0);
	}

	
};

//	Convert OpenCV Camera Paramator to OpenGL Projection Matrix
//	@Param
//		camMat: OpenCV camera matrix, from OpenCV camera calibration
//		camSz:  camera window size
//		znear:  near z point of frustum clipping
//		zfar:   far z point of frustum clipping
glm::mat4 cvtCVCameraParam2GLProjection(cv::Mat camMat, cv::Size camSz, double znear, double zfar)
{
	//	Load camera parameters
	double fx = camMat.at<double>(0, 0);
	double fy = camMat.at<double>(1, 1);
	double s = camMat.at<double>(0, 1);
	double cx = camMat.at<double>(0, 2);
	double cy = camMat.at<double>(1, 2);
	double w = camSz.width, h = camSz.height;

	//	�Q�l:https://strawlab.org/2011/11/05/augmented-reality-with-OpenGL
	//	With window_coords=="y_down", we have:
	//	[2 * K00 / width,	-2 * K01 / width,	(width - 2 * K02 + 2 * x0) / width,		0]
	//	[0,					2 * K11 / height,	(-height + 2 * K12 + 2 * y0) / height,	0]
	//	[0,					0,					(-zfar - znear) / (zfar - znear),		-2 * zfar*znear / (zfar - znear)]
	//	[0,					0,					-1,										0]

	glm::mat4 projection(
		-2.0 * fx / w, 0, 0, 0,
		0, -2.0 * fy / h, 0, 0,
		1.0 - 2.0 * cx / w,  -1.0 + 2.0 * cy / h, -(zfar + znear) / (zfar - znear), -1.0,
		0, 0, -2.0 * zfar * znear / (zfar - znear), 0);

	return projection;
}
