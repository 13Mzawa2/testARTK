#pragma once

#include "OpenGLHeader.h"

class Shader
{
private:
	GLuint vertexShader, fragmentShader;		//	�V�F�[�_�I�u�W�F�N�g
	const char *vertexFileName, *fragmentFileName;		//	�V�F�[�_�t�@�C����
public:
	GLuint program;			//	�V�F�[�_�v���O����
	Shader();
	~Shader();
	//	�R�s�[�R���X�g���N�^
	Shader &operator=(Shader &_s)
	{
		initGLSL(_s.vertexFileName, _s.fragmentFileName);
		return *this;
	}
	void readShaderCompile(GLuint shader, const char *file);	//	.shader�̃R���p�C��
	void link(GLuint prog);		//	�R���p�C������shader�������N����
	//	������
	//	�t���O�����g�V�F�[�_�[�̗L���ŕ�����
	void initGLSL(const char *vertexFile);
	void initGLSL(const char *vertexFile, const char *fragmentFile);
	//	�L����
	void enable(){ glUseProgram(program); }
	void disable(){ glUseProgram(0); }

};

