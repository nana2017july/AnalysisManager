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

#ifndef __ANALYSIS_EXECUTOR_H__
#define __ANALYSIS_EXECUTOR_H__


#include "apr_buckets.h"
#include "ac_this_is_class.h"

#include "analysis_manager.h"


//------------------------------------------------------
//----    CAnalysisExecutor_PartialMatch Class       ---------------------
//------------------------------------------------------
/**
@brief CAnalysisExecutor �̔h���N���X�B������v����������u������N���X�B
@see CAnalysisManager_new()
*/
typedef struct CAnalysisExecutor_PartialMatch {
	CAnalysisExecutor parentMember;
	//
} CAnalysisExecutor_PartialMatch;


CAnalysisExecutor_PartialMatch* CAnalysisExecutor_PartialMatch_new(const char* targetStr, const char* replaceStr);



//------------------------------------------------------
//----    CAnalysisExecutor_DoNothing Class       ---------------------
//------------------------------------------------------
/**
@~japanese
@brief CAnalysisExecutor �̔h���N���X�B�e�X�g�p�B�������Ȃ��N���X�B������g�����Ƃ�Excutor�ȊO�̕����̏����ɂ����鎞�Ԃ������B

@~english
@brief Derived class of CAnalysisExecutor for test. Ex. this is used to measure performance time without Executor class process. 

@~
@see CAnalysisManager_new()
*/
typedef struct CAnalysisExecutor_DoNothing {
	CAnalysisExecutor parentMember;
	//
} CAnalysisExecutor_DoNothing;


CAnalysisExecutor_DoNothing* CAnalysisExecutor_DoNothing_new();



//------------------------------------------------------
//----    CAnalysisExecutor_HtmlTagReplace Class       ---------------------
//------------------------------------------------------
/**
@brief CAnalysisExecutor �̔h���N���X�B�w���HTML�^�O��u������B
@n ���̃N���X�̓�����e�B
@n �^�O�̊J�n�i���j����������AAC_POS�Ń^�O�̏I���i���j��������B
@n ���̌�A�^�O���̂�u������ꍇ��AC_REPLACE�����s�B
@n �^�O�̌���u������ꍇ��AC_FORWARD�Ń^�O�̏I���ʒu�܂ňړ����Ă���AAC_REPLACE�����s�B
@see CAnalysisExecutor_HtmlTagReplace_new()
*/
typedef struct CAnalysisExecutor_HtmlTagReplace {
	CAnalysisExecutor parentMember;
	//
} CAnalysisExecutor_HtmlTagReplace;

/**
�R���X�g���N�^
@param targetTagName [in]�u���Ώۂ̃^�O��
@param replaceStr [in]�u��������
@param isTagReplace [in]�^�O���̂�u������̂��HFALSE�̂Ƃ��A�^�O���̂͒u�������A���������^�O�̌��ɒu��������}������B
*/
CAnalysisExecutor_HtmlTagReplace* CAnalysisExecutor_HtmlTagReplace_new(const char* targetTagName, const char* replaceStr, AcBool isTagReplace);






//------------------------------------------------------
//----    CAnalysisExecutor_Multipart Class       ---------------------
//------------------------------------------------------
/**
@brief CAnalysisExecutor �̔h���N���X�B�}���`�p�[�g�̃��N�G�X�g����͂��ăp�����^���擾����B
@n     ����ł̓t�@�C���ifilename���w�b�_�ɑ��݂�����́j�͎擾���Ȃ��B�������𑽂�����Ă��܂����߂ł���B
@n     multipart/mixed�̂悤�ȓ�d�\���ɂ͑Ή����Ă��Ȃ��B
@n     �擾�����p�����^�̖��O�ƒl�̕�����̒����ɂ͍ő包��������A�ő包�� MULTIPART_NAME_MAX_LEN , MULTIPART_VALUE_LEN �Ɠ��������̏ꍇ�A
@n     �����ƒ����\��������B
@see CAnalysisExecutor_Multipart_new()
*/
typedef struct {
	CAnalysisExecutor parentMember;
	//
} CAnalysisExecutor_Multipart;

/**
�R���X�g���N�^
@param table       [in]��͂��ăp�����^���ƒl��ݒ�Bapr_table_t �ɕۑ������̂Œl�̎擾�̎d����apr�̃��t�@�����X���Q�Ƃ̂��ƁB
@n                     �j��͗��p�ґ��ōs���B�������A�ʏ�table��pool�ō쐬���Ǘ������̂ŁA�j���pool�ɔC����΂悢�B
@param targetParamName [in]���󖢎g�p�B""��ݒ�B
@param boundaryStr [in]boundary������B"\r\n--" + boundary�@��ݒ肷�邱�ƁB
*/
CAnalysisExecutor_Multipart* CAnalysisExecutor_Multipart_new(apr_table_t* table, const char* targetParamStr, const char* boundaryStr);

/**
��͌�Ƀ}���`�p�[�g�̃p�����^���擾����B�p�����^�����L�[�ɂ��āA�p�����^�l��o�^����Ă���B
@return ��͂������ʂœ���ꂽ�p�����^
*/
const apr_table_t* CAnalysisExecutor_Multipart_getParams(CAnalysisExecutor_Multipart* p_this);




#endif  //end __ANALYSIS_EXECUTOR_H__