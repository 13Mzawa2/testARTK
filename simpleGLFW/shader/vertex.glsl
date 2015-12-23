#version 330 core
 
// �C���v�b�g���_�f�[�^�B���̃V�F�[�_�̎��s���ƂɈقȂ�܂��B
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec3 vertexNormal;
 
// �A�E�g�v�b�g�f�[�^�B�e�t���O�����g�ŏ������܂�܂��B
out vec4 vertexPosition_cameraspace;
out vec2 UV;
out vec3 Normal;		//	�J�������W�n�ł̖@��
 
// ���ׂẴ��b�V���ň��̒l
uniform mat4 MVP;
uniform mat4 MV;		//	�@���v�Z�Ɏg��
 
void main()
{
    // �N���b�v��Ԃł̒��_�̏o�͈ʒu�BMVP�~�ʒu
    gl_Position =  MVP * vec4(vertexPosition_modelspace,1);
	vertexPosition_cameraspace = MV * vec4(vertexPosition_modelspace, 1);

    // ���_��UV���W�ł��B���ʂȋ�Ԃ͂���܂���B
    UV = vertexUV;

	//	�@���x�N�g�����J�����s��ɕϊ����ēn��
	Normal = vec3(normalize(MV * vec4(vertexNormal, 0)));
}