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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
//
#include "ac_bucket_utils.h"
//
#include "apr.h"
#include "apr_general.h"
#include "apr_tables.h"
#include "apr_strings.h" 
#include "apr_buckets.h"

//
#include "analysis_executor_impl.h"
#include "analysis_manager_.h"
#include "analysis_parser_impl.h"


static const AnalysisCommand CAnalysisExecutor_Multipart_pos(CAnalysisExecutor* p_this,
	const AnalysisCommand cmd, const char c, const AcBool isEos);

static const char* gClassName_AnalysisExecutor_Multipart = "CAnalysisExecutor_Multipart";


//
#define MULTIPART_NAME_MAX_LEN 100
#define MULTIPART_VALUE_LEN 200



//------------------------------------------------------
//----  CAnalysisExecutor_Multipart Class    ---------------------
//------------------------------------------------------
//------------------------------------------------------

/**
���݁A��͂��Ă��镔����\���X�e�[�^�X�B
*/
typedef enum{
	MultipartStatus_Header,
	MultipartStatus_Body,
	MultipartStatus_Boundary
} MultipartStatus;

/**
private
*/
typedef struct {
	CAnalysisExecutor_Super parentMember;
	//�����g�p�\��
	char* targetParamName;
	///���E(boundary)�̃p�[�T(���E�܂ł���͂���)
	CAnalysisParser_Split* boundaryParser;
	///�w�b�_�̃p�[�T(�w�b�_1�s(\r\n�܂�)����͂���)
	CAnalysisParser_HttpHeader* headerParser;
	///���݂̉�͂��Ă���p�[�g�i�w�b�_�� or �{�f�B���j
	MultipartStatus status;
	///boundary�̕�����̒���
	size_t boundaryStrLen;
	///��͌��ʂ̃p�����^��o�^����e�[�u��
	apr_table_t* paramsTable;
	///true�̏ꍇ�A�����t�@�C���łȂ���΃p�����^�e�[�u���ɓo�^����
	AcBool isParamsAddedIfNotFile;
	///��͓r���Ɏg�p�B�ꎞ�I�Ƀp�����^����ݒ肷��̈�
	char name[MULTIPART_NAME_MAX_LEN + 1];
	///��͓r���Ɏg�p�B�ꎞ�I�Ƀp�����^�l��ݒ肷��̈�
	char value[MULTIPART_VALUE_LEN + 1];
	///��͓r���Ɏg�p�B����͒��̃p�[�g���t�@�C�����H
	AcBool isValueFile;
} CAnalysisExecutor_Multipart_Super;


static void CAnalysisExecutor_Multipart__debugPrint(const CAnalysisExecutor* p_this) {
	const CAnalysisExecutor_Multipart_Super* self = (const CAnalysisExecutor_Multipart_Super*)p_this;
	//
	ac_printf("boundaryStrLen=%u, isValueFile=%d, isParamsAddedIfNotFile=%d, name=%s, value=%s\n", 
		self->boundaryStrLen, self->isValueFile, self->isParamsAddedIfNotFile, self->name, self->value);
	if(self->boundaryParser != NULL){
		CANALYSIS_PARSER_DEBUG_PRINT((CAnalysisParser*)self->boundaryParser);
	}
	if(self->headerParser != NULL){
		CANALYSIS_PARSER_DEBUG_PRINT((CAnalysisParser*)self->headerParser);
	}
}


static void CAnalysisExecutor_Multipart_reset(CAnalysisExecutor_Multipart_Super* p_this){
	CAnalysisExecutor_Multipart_Super* self = (CAnalysisExecutor_Multipart_Super*)p_this;
	//
	CAnalysisParser_reset((CAnalysisParser*)self->boundaryParser);
	CAnalysisParser_reset((CAnalysisParser*)self->headerParser);
	//
	self->status = MultipartStatus_Header;
	*self->name = '\0';
	*self->value = '\0';
	self->isValueFile = AC_FALSE;
}


static AcBool CAnalysisExecutor_Multipart_start(CAnalysisExecutor* p_this, const apr_table_t* table) {
	//ac_printf("-[start]------------------\n");
	return AC_TRUE;
}


/**
boundary�̋�؂�ɗ��邽�тɌĂ΂��B
*/
static const AnalysisCommand CAnalysisExecutor_Multipart_forward(CAnalysisExecutor* p_this,
	const AnalysisCommand cmd, const AcBool isRejectPreCmd, const char c) 
{
	CAnalysisExecutor_Multipart_Super* self = (CAnalysisExecutor_Multipart_Super*)p_this;
	//�O��̃R�}���h�����s���ۂ��ꂽ�̂ŏ�ԃ��Z�b�g����
	if(isRejectPreCmd){
		self->status = MultipartStatus_Header;
	}

	//
	AnalysisCommand nextCmd;
	nextCmd.type = AC_POS;
	nextCmd.pos.pos = 0;

	//�u���ʒu��ς����ɉ�͂���
	return 	CAnalysisExecutor_Multipart_pos((CAnalysisExecutor*)self, nextCmd, c, AC_FALSE);
}


static const AnalysisCommand CAnalysisExecutor_Multipart_replace(CAnalysisExecutor* p_this,
	const AnalysisCommand cmd, AcBool result) 
{
	CAnalysisExecutor_Multipart_Super* self = (CAnalysisExecutor_Multipart_Super*)p_this;
	//
	AnalysisCommand nextCmd;
	nextCmd.type = AC_FORWARD;
	return nextCmd;
}


/**

*/
static const AnalysisCommand CAnalysisExecutor_Multipart_pos(CAnalysisExecutor* p_this,
	const AnalysisCommand cmd, const char c, const AcBool isEos)
{
	CAnalysisExecutor_Multipart_Super* self = (CAnalysisExecutor_Multipart_Super*)p_this;
	//
	AnalysisCommand nextCmd;
	const char* str;
	size_t len;

	//EOS�̏ꍇ�A�ǂݔ�΂��ďI��������i�s���S�Ȍ`�Ȃ̂Łj
	if(isEos){
		nextCmd.type = AC_FORWARD_N;
		nextCmd.forward_n.len = cmd.pos.pos;
		return nextCmd;
	}

	//�p�[�T�ɕ�����n��
	if(self->status == MultipartStatus_Header){
		//�w�b�_���̉�͂����Ă���ꍇ
		CAnalysisParser* parser = (CAnalysisParser*)self->headerParser;
		CAnalysisParser_accept(parser, c);
		if(CAnalysisParser_isEnd(parser)){
			if(CAnalysisParser_isMatched(parser)){
				//�������}�b�`���Ă�����A���O���擾����
				CAnalysisParser_HttpHeader_getValue((CAnalysisParser_HttpHeader*)parser, &str, &len);
				char** ret = ac_splitHttpHeaderValue(str, ";", AC_TRUE, 4);
				AcBool isValueFile = AC_FALSE;
				for(char** p = ret; *p != NULL; p += 2){
					//"name"�Ƀ}�b�`������R�s�[
					if(strcmp("name", *p) == 0){
						size_t nameLen = strlen(*(p + 1));
						if(nameLen >= MULTIPART_NAME_MAX_LEN) nameLen = MULTIPART_NAME_MAX_LEN;
						strncpy(self->name, *(p + 1), nameLen);
						self->name[nameLen] = '\0';
						//ac_toLowerCase(self->name); 
					} else if(strcmp("filename", *p) == 0){
						//�t�@�C���̏ꍇ
						isValueFile = AC_TRUE;
					}
				}
				//name���}�b�`���āAvalue���t�@�C���̏ꍇ�ɐݒ�
				if(*self->name != '\0' && isValueFile) self->isValueFile = AC_TRUE;
				//���
				free((void*)ret);
				//�w�b�_�p�[�T�����Z�b�g���Ď��̃w�b�_��
				CAnalysisParser_reset(parser);

			} else if(CAnalysisParser_getParsedStrLen(parser) == 2){
				//�w�b�_�̏I���i\r\n�j�������ꍇ
				self->status = MultipartStatus_Body;
			} else{
				//�s�̏I���܂ŗ��Ă��邪�}�b�`���Ȃ������ꍇ�A�w�b�_�p�[�T�����Z�b�g���Ď��̃w�b�_��
				CAnalysisParser_reset(parser);
			}
		} 
	} else if(self->status == MultipartStatus_Body){
		//�{�f�B���̃p�[�X�����Ă���ꍇ
		CAnalysisParser* parser = (CAnalysisParser*)self->boundaryParser;
		CAnalysisParser_accept(parser, c);
		if(CAnalysisParser_isEnd(parser)){
			//�}�b�`������EOS�̏ꍇ
			if(!self->isParamsAddedIfNotFile || !self->isValueFile){
				//�p�����^�e�[�u���ɒǉ�����
				CAnalysisParser_Split_getStockStr((CAnalysisParser_Split*)parser, &str, &len);
				//value��������R�s�[����
				size_t valueLen = len;
				if((int)len - (self->boundaryStrLen) >= 0) valueLen = len - (self->boundaryStrLen);
				if(valueLen >= MULTIPART_VALUE_LEN) valueLen = MULTIPART_VALUE_LEN;
				strncpy(self->value, str, valueLen);
				self->value[valueLen] = '\0';
				//�e�[�u���Ƀp�����^���ƒl��ۑ�(table��key�͑啶������������ʂ��Ȃ�)
				apr_table_add(self->paramsTable, self->name, self->value);
				//�N���X�����Z�b�g
				CAnalysisExecutor_Multipart_reset(self);
			}
			//���̃u���b�N�܂œǂݔ�΂�(�p�[�X����������ǂݔ�΂�)
			nextCmd.type = AC_FORWARD_N;
			nextCmd.forward_n.len = cmd.pos.pos + 2 + 1; //���s���ǂݔ�΂�
			return nextCmd;
		}

	}

	nextCmd.type = AC_POS;
	nextCmd.pos.pos = cmd.pos.pos + 1;
	return nextCmd;
}


static const AnalysisCommand CAnalysisExecutor_Multipart_end(CAnalysisExecutor* p_this, const AnalysisCommand cmd) {
	CAnalysisExecutor_Multipart_Super* self = (CAnalysisExecutor_Multipart_Super*)p_this;
	//
	AnalysisCommand nextCmd;
	nextCmd.type = AC_END;
	nextCmd.end.status = -1;
	nextCmd.end.replacement = NULL;
	//
	//ac_printf("-[end]------------------\n");
	return nextCmd;
}




const apr_table_t* CAnalysisExecutor_Multipart_getParams(CAnalysisExecutor_Multipart* p_this){
	CAnalysisExecutor_Multipart_Super* self = (CAnalysisExecutor_Multipart_Super*)p_this;
	return self->paramsTable;
}


static void CAnalysisExecutor_Multipart_delete(void* p_this) {
	CAnalysisExecutor_Multipart_Super* self = (CAnalysisExecutor_Multipart_Super*)p_this; 
	ac_checkClass(gClassName_AnalysisExecutor_Multipart, self->parentMember.publicMember.thisIsClass->name, "in CAnalysisExecutor_PartialMatch_delete()", AC_FALSE);
	//
	AcClass_delete(self->boundaryParser);
	AcClass_delete(self->headerParser);
	free(self);
}

CAnalysisExecutor_Multipart* CAnalysisExecutor_Multipart_new(apr_table_t* table, const char* targetParamStr, const char* boundaryStr) {
	CAnalysisExecutor_Multipart_Super* self = (CAnalysisExecutor_Multipart_Super*)malloc(sizeof(CAnalysisExecutor_Multipart_Super));
	if (self == NULL) return NULL;
	//�e�����o�[�̏�����
	CAnalysisExecutor_init((CAnalysisExecutor*)self, gClassName_AnalysisExecutor_Multipart, 
		CAnalysisExecutor_Multipart_delete,
		CAnalysisExecutor_Multipart_start,
		CAnalysisExecutor_Multipart_forward,
		CAnalysisExecutor_Multipart_replace,
		CAnalysisExecutor_Multipart_pos,
		CAnalysisExecutor_Multipart_end
		);
	//
	self->boundaryParser = CAnalysisParser_Split_new(boundaryStr, 200);
	self->headerParser = CAnalysisParser_HttpHeader_new("Content-Disposition:", 200);
	self->paramsTable = table;
	//self->targetParamName = ac_copyToLowerCase(targetParamStr);
	self->boundaryStrLen = strlen(boundaryStr);
	self->isParamsAddedIfNotFile = AC_TRUE;
	//
	CAnalysisExecutor_Multipart_reset(self);
	//self->status = MultipartStatus_Header;
	//self->name[0] = '\0';
	//self->value[0] = '\0';
	//
	CAnalysisExecutor_setDebugPrintFunc((CAnalysisExecutor*)self, CAnalysisExecutor_Multipart__debugPrint);
	return (CAnalysisExecutor_Multipart*)self;
}
















