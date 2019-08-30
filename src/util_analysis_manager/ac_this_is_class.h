/*
* Copyright 2019 the original author or authors.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef __AC_THIS_IS_CLASS_H__
#define __AC_THIS_IS_CLASS_H__


/**
@file
Anddy's Custumization���C�u�����B
C����ŃN���X���ۂ����Ƃ����邽�߂̃��C�u�����ł��B
���̃t�@�C����APR�Ɉˑ����܂���B�Ɨ����Ă��܂��B

@~english
Anddy's Custumization libraries.
This is library whitch do class-like programming in c-language.
This file dose not depend on apr libraries.

*/

/**
@struct AcCThisIsClass
@brief �\���̂��N���X�Ƃ��Ĉ������߂̃}�[�J�[�ł���A��{���������邽�߂̃N���X�B
@detail <pre>
�E1��̌p�����\�i������͂ł��Ȃ��j
�E���d�p���͂ł��Ȃ��B
�E���Ԑ��͎���������B

�y�������[���z
�E�K���\���̂̐擪�ɐ錾����B
�E�h���N���X�ł͐e�N���X��*_Super���\���̂̐擪�ɐ錾����B

�y�������[���z
�N���X���F
�@�E�擪�啶��
�@�E�h���N���X�͌��ɖ��O��t����B��F�uTestSimple�v
�@�Eprivate�����o�[���܂߂��e�N���X�͌���_Super������B��F�uTest_Super�v
���\�b�h��
�@�E�N���X���ƃ��\�b�h�����A���_�[�o�[�łȂ��B��F�uTest_calc�v�uTestSimple_calc�v
�@�E���\�b�h���̐擪�͏�����
��F
 �@�ETestSimple �ːe�N���XTest��Simple�h���N���X
 �@�ETest_calc�@��Test�N���X�̃����o�[��calc�֐�
 �@�ETest_HHH�@�˃N���X�Ƃ͖��֌W�B
</pre>
@code
�y�g�����̗�z
//Test�N���X-----------------------
//�N���X�̐錾������
typedef struct CTest {
	AcCThisIsClass* thisIsClass;
} CTest;

//private�̃����o�[��ǉ������e�N���X���쐬����B
typedef struct CTest_Super {
	CTest publicMember;
	int (*calc)(int h);
	int len;
	AcDeleteFunc_T childDeleteFunc;
} CTest_Super;


//public���\�b�h���쐬����B
static const int CTest_calc(Test* p_this, int h) {
	CTestSimple* self = (CTestSimple*)p_this;
	return self->calc((CTest*)self, h);
};

//�e�N���X�̃f�X�g���N�^���B��(static)�ō쐬����B
//thisIsClass�̔j���deleteClass���ōs���̂ł��̒��ł͋L�q�s�v
static void CTest_delete(void* p_this) {
	CTest_Super* self = (CTest_Super*)p_this;
	self->childDeleteFunc(self);
	//���̌�A�e�N���X�̃f�X�g���N�g����������
}

//�e�N���X�̏������֐����쐬����B���̊֐��͔h���N���X�̃R���X�g���N�^����Ăяo���B
static void CTest_init(void* p_this, const char* className, DeleteFunc deleteFunc, int (*calc)(int)){
	CTest_Super* self = (CTest_Super*)p_this;
	self->publicMember.thisIsClass = AcCThisIsClass_new(className, Test_delete);
	self->len = 0;
	self->calc = calc;
	self->childDeleteFunc = deleteFunc;
}


//�h���N���X------------------------
//��̒�`������
typedef struct CTestSimple {
	CTest publicMember;
}CTestSimple;

//�h���N���X��private���쐬����B�K���e�N���X��擪�ɏ����B
typedef struct CTestSimple_Super {
	CTest_Super super;
	char* str;
}CTestSimple_Super;

//�h���N���X�̃I�[�o�[���C�h���郁�\�b�h���쐬����B
static const int CTestSimple_calc(CTest* p_this, int h) {
	CTestSimple_Super* self = (CTestSimple_Super*)p_this;
	return self->super.len * h;
};

//�f�X�g���N�^���쐬����B�h���N���X���̔j�󏈗������s���B�e�N���X�̔j��͐e�N���X���s���̂ł����ł͏����Ȃ��B
static void CTestSimple_delete(void* p_this) {
	CTestSimple_Super* self = (CTestSimple_Super*)p_this;
	//�f�o�b�O�̂��߈����̃N���X�����̃N���X���`�F�b�N���Ă��悢�B
	ac_checkClass("TestSimple", self->super.publicMember.thisIsClass->name, "in CTestSimple_delete()", AC_FALSE);
	free(self->str);
	free(self);
}

//�h���N���X�̃R���X�g���N�^���쐬����B
CTest* CTestSimple_new(cost char* str) {
	CTestSimple_Super* self = (CTestSimple_Super*)malloc(sizeof(CTestSimple_Super));
	if (self == NULL) return NULL;
	//�N���X�����쐬
	CTest_init(self, "CTestSimple", CTestSimple_delete,  CTestSimple_calc);
	//�h���N���X�̃����o�̏�����
	self->str = malloc(strlen(str)+1);
	strcpy(self->str, str);
	//
	return ((CTest*)self);
}

//�g������------------
//�h���N���X�𐶐�
CTest* t = CTestSimple_new("aaa");
//�e�̃��\�b�h�ɔh���N���X�������ɂ��ČĂяo���B
int i = CTest_calc(t, 10);

@endcode
*/



#define AC_TRUE 1
#define AC_FALSE 0

typedef int AcBool;



typedef struct AcCThisIsClass {
	const char* name;
} AcCThisIsClass;

/** �폜�֐��|�C���^���^��` */
typedef void (*AcDeleteFunc_T)(void*);

/**
�N���X��ՂƂȂ�N���X�̐���
*/
AcCThisIsClass* AcCThisIsClass_new(const char* name, void (*deleteFunc)(void*));

/**
AcCThisIsClass�𓋍ڂ����N���X��j������B
���̃��\�b�h�ȊO��free�ȂǂŔj�����Ȃ����ƁB
*/
void AcClass_delete(void*);

/**
�N���X�����擾����B AcCThisIsClass ���p�����Ă���K�v����B�p�����Ă��Ȃ��ꍇ�͓���s��B
*/
const char* AcClass_getName(void*);

/** �N���X�����`�F�b�N����B
@param ecpected [in]���҂���N���X���B
@param real [in]���ۂ̃N���X���B
@param errmsg [in]�Ԃ�l��AC_FALSE�̂Ƃ��A�W���G���[�o�͂���G���[���b�Z�[�W�B
@oaram isExitIfErr [in]AC_TRUE�̏ꍇ�A�Ԃ�l��AC_FALSE�̂Ƃ��Aexit();���Ăяo���B
@return ���҂���N���X���ƈႤ�ꍇ�AAC_FALSE
*/
const AcBool ac_checkClass(const char* ecpected, const char* real, const char* errmsg, const AcBool isExitIfErr);

#endif  //end __AC_THIS_IS_CLASS_H__

