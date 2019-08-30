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



static const char* gClassName_AnalysisExecutor_PartialMatch = "CAnalysisExecutor_PartialMatch";


//------------------------------------------------------
//----  CAnalysisExecutor_PartialMatch Class    ---------------------
//------------------------------------------------------
//------------------------------------------------------

/**
private
*/
typedef struct CAnalysisExecutor_PartialMatch_Super {
	CAnalysisExecutor_Super parentMember;
	///�����Ώە�����
	char* targetStr;
	///�����Ώە�����̒���
	size_t targetStrLen;
	///���݈ʒu
	char* targetStrPos;
	///�u��������
	char* replaceStr;
} CAnalysisExecutor_PartialMatch_Super;


static void CAnalysisExecutor_PartialMatch__debugPrint(const CAnalysisExecutor* p_this) {
	const CAnalysisExecutor_PartialMatch_Super* self = (const CAnalysisExecutor_PartialMatch_Super*)p_this;
	//
	ac_printf("targetStr=%s[targetStrLen=%d, targetStrPos=%p], replaceStr=%s\n",
		self->targetStr, (int)self->targetStrLen, self->targetStrPos, self->replaceStr);
}


static AcBool CAnalysisExecutor_PartialMatch_start(CAnalysisExecutor* p_this, const apr_table_t* table) {
	//ac_printf("-[start]------------------\n");
	return AC_TRUE;
}

/**
���̃N���X�̏ꍇ�Aforward�X�e�[�^�X�ł͈ȉ��̏�Ԃ��������Ƃɂ���B
�E�����Ώە�����̐擪��1�����ƃ}�b�`����ӏ���T���B���������ꍇ��pos�X�e�[�^�X�֑J�ځB
*/
static const AnalysisCommand CAnalysisExecutor_PartialMatch_forward(CAnalysisExecutor* p_this, 
	const AnalysisCommand cmd, const AcBool isRejectPreCmd, const char c) 
{
	CAnalysisExecutor_PartialMatch_Super* self = (CAnalysisExecutor_PartialMatch_Super*)p_this;
	//�O��̃R�}���h�����s���ۂ��ꂽ�̂ŏ�ԃ��Z�b�g����
	if(isRejectPreCmd){
		self->targetStrPos = self->targetStr;
	}
	//
	AnalysisCommand nextCmd;
	//�����Ώە�����̐擪�̕����ƃ}�b�`���邩�m�F
	if(c == *self->targetStr){
		//�擪�����ƈ�v����ꍇ�Apos�X�e�[�^�X�ɑJ�ځB���̂��Ƃ̕������}�b�`���邩�`�F�b�N���Ă����B
		nextCmd.type = AC_POS;
		nextCmd.pos.pos = 1;
		self->targetStrPos = self->targetStr + 1;
		return nextCmd;
	}

	//�}�b�`���Ȃ������ꍇ�́A�u���J�n�ʒu���P�o�C�g�����߂�B
	nextCmd.type = AC_FORWARD;
	return nextCmd;
}

static const AnalysisCommand CAnalysisExecutor_PartialMatch_replace(CAnalysisExecutor* p_this, 
	const AnalysisCommand cmd, AcBool result) 
{
	CAnalysisExecutor_PartialMatch_Super* self = (CAnalysisExecutor_PartialMatch_Super*)p_this;
	//
	AnalysisCommand nextCmd;
	nextCmd.type = AC_FORWARD;
	return nextCmd;
}

/**
���̃N���X�̏ꍇ�Apos�X�e�[�^�X�ł͈ȉ��̏�Ԃ��������Ƃɂ���B
@n �E�����Ώە������2�����ȍ~���Ō�܂Ń}�b�`���邩���`�F�b�N�B
@n   �}�b�`�̏ꍇ��replace�X�e�[�^�X�֑J�ځB�}�b�`���Ȃ��ꍇ��forward�X�e�[�^�X�֑J�ځB
*/
static const AnalysisCommand CAnalysisExecutor_PartialMatch_pos(CAnalysisExecutor* p_this, 
	const AnalysisCommand cmd, const char c, const AcBool isEos)
{
	CAnalysisExecutor_PartialMatch_Super* self = (CAnalysisExecutor_PartialMatch_Super*)p_this;
	//
	AnalysisCommand nextCmd;
	
	//EoS�������ꍇ�A���ׂă}�b�`����O��EoS�ɂȂ����ƍl������
	if(isEos){
		nextCmd.type = AC_FORWARD_N;
		nextCmd.forward_n.len = cmd.pos.pos;
		return nextCmd;
	}

	//���
	if (*self->targetStrPos == c) {
		//�������}�b�`�����ꍇ
		++self->targetStrPos;
		if (*self->targetStrPos == '\0') {
			//���ׂă}�b�`�����ꍇ
			nextCmd.type = AC_REPLACE;
			nextCmd.replace.len = self->targetStrLen;
			nextCmd.replace.replacement = self->replaceStr;
		}
		else {
			nextCmd.type = AC_POS;
			nextCmd.pos.pos = cmd.pos.pos + 1;
		}
	}
	else {
		//�}�b�`���Ȃ������ꍇ
		nextCmd.type = AC_FORWARD;
	}
	return nextCmd;
}

static const AnalysisCommand CAnalysisExecutor_PartialMatch_end(CAnalysisExecutor* p_this, const AnalysisCommand cmd) {
	CAnalysisExecutor_PartialMatch_Super* self = (CAnalysisExecutor_PartialMatch_Super*)p_this;
	//
	AnalysisCommand nextCmd;
	nextCmd.type = AC_END;
	nextCmd.end.status = -1;
	nextCmd.end.replacement = NULL;
	//
	//ac_printf("-[end]------------------\n");
	return nextCmd;
}



static void CAnalysisExecutor_PartialMatch_delete(void* p_this) {
	CAnalysisExecutor_PartialMatch_Super* self = (CAnalysisExecutor_PartialMatch_Super*)p_this; 
	ac_checkClass(gClassName_AnalysisExecutor_PartialMatch, self->parentMember.publicMember.thisIsClass->name, "in CAnalysisExecutor_PartialMatch_delete()", AC_FALSE);
	//
	free(self->replaceStr);
	free(self->targetStr);
	free(self);
}

CAnalysisExecutor_PartialMatch* CAnalysisExecutor_PartialMatch_new(const char* targetStr, const char* replaceStr) {
	CAnalysisExecutor_PartialMatch_Super* self = (CAnalysisExecutor_PartialMatch_Super*)malloc(sizeof(CAnalysisExecutor_PartialMatch_Super));
	if (self == NULL) return NULL;
	//�e�����o�[�̏�����
	CAnalysisExecutor_init((CAnalysisExecutor*)self, gClassName_AnalysisExecutor_PartialMatch, 
		CAnalysisExecutor_PartialMatch_delete,
		CAnalysisExecutor_PartialMatch_start, 
		CAnalysisExecutor_PartialMatch_forward,
		CAnalysisExecutor_PartialMatch_replace,
		CAnalysisExecutor_PartialMatch_pos,
		CAnalysisExecutor_PartialMatch_end
		);
	//
	self->targetStrLen = strlen(targetStr);
	char* str = (char*)malloc(self->targetStrLen + 1);
	if (str == NULL) {
		free(self);
		return NULL;
	}
	strcpy(str, targetStr);
	self->targetStr = str;
	//
	str = (char*)malloc(strlen(replaceStr) + 1);
	if (str == NULL) {
		free(self->targetStr);
		free(self);
		return NULL;
	}
	strcpy(str, replaceStr);
	self->replaceStr = str;
	self->targetStrPos = str;
	//
	CAnalysisExecutor_setDebugPrintFunc((CAnalysisExecutor*)self, CAnalysisExecutor_PartialMatch__debugPrint);
	return (CAnalysisExecutor_PartialMatch*)self;
}
















