/*
* Copyright 2017 the original author or authors.
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


static const char* gClassName_AnalysisExecutor_HtmlTgaReplace = "CAnalysisExecutor_HtmlTagReplace";


//------------------------------------------------------
//----  CAnalysisExecutor_HtmlTagReplace Class    ---------------------
//------------------------------------------------------
//------------------------------------------------------

/**
@brief CAnalysisExecutor_HtmlTagReplace �N���X�̌��݂̏�ԁB
*/
typedef enum HtmlTagReplaceStatus {
	///�^�O�̊J�n�i���j��T���Ă�����
	HtmlTagReplaceStatus_SearchStart,
	///�^�O�����}�b�`���邩�����ؒ��̏��
	HtmlTagReplaceStatus_MatchingTagName,
	///�^�O���̌��i�X�y�[�X�����j���}�b�`���邩�����ؒ��̏��
	HtmlTagReplaceStatus_MatchingAfterTagName,
	///�^�O�̏I���i���j��T���āA�u���͂��Ȃ���ԁi�^�O�����}�b�`���Ȃ������j�B
	HtmlTagReplaceStatus_SearchTagEndWithNoReplace,
	///�^�O�̏I���i���j��T���āA�u�����悤�Ƃ��Ă����ԁB
	HtmlTagReplaceStatus_SearchTagEndWithReplace
} HtmlTagReplaceStatus;


/**
private�܂Ŋ܂߂��N���X��`�B
*/
typedef struct CAnalysisExecutor_HtmlTagReplace_Super {
	CAnalysisExecutor_Super parentMember;
	///�^�O���i�������ɂ��ĕۊǂ����j
	char* tagName;
	///�^�u���̕�����
	size_t tagNameStrLen;
	///�^�O���`�F�b�N�p�B���݂̈ʒu�B
	char* tagNameStrPos;
	///�^�O�̏I���i���j�ʒu�܂ł̒����B"��"���܂ށB
	size_t tagStrLen;
	///�u��������
	char* replaceStr;
	///�^�O���̂�u������̂��H�iAC_FALSE�̂Ƃ��A�^�O���̂͒u�������A���������^�O�̌��ɕ������u���j
	AcBool isTagReplace;
	///�ǂݔ�΂������Ēu��������ꍇ�B
	HtmlTagReplaceStatus replaceStatus;
} CAnalysisExecutor_HtmlTagReplace_Super;



static void CAnalysisExecutor_HtmlTagReplace__debugPrint(const CAnalysisExecutor* p_this){
	const CAnalysisExecutor_HtmlTagReplace_Super* self = (const CAnalysisExecutor_HtmlTagReplace_Super*)p_this;
	//
	ac_printf("tagName=%s[tagNameStrLen=%d, tagNameStrPos=%p], tagStrLen=%d, replaceStr=%s, isTagReplace=%d, replaceStatus=%d\n",
		self->tagName, (int)self->tagNameStrLen, self->tagNameStrPos, (int)self->tagStrLen, self->replaceStr, 
		(int)self->isTagReplace, (int)self->replaceStatus);
}

/**
�N���X�����̏�Ԃ����Z�b�g���A�V����forward�ł����Ԃɂ���B
*/
static void reset(CAnalysisExecutor_HtmlTagReplace_Super* p_this){
	CAnalysisExecutor_HtmlTagReplace_Super* self = (CAnalysisExecutor_HtmlTagReplace_Super*)p_this;
	//��Ԃ����Z�b�g
	self->tagStrLen = 0;
	self->replaceStatus = HtmlTagReplaceStatus_SearchStart;
}

static AcBool CAnalysisExecutor_HtmlTagReplace_start(CAnalysisExecutor* p_this, const apr_table_t* table){
	//ac_printf("-[start]------------------\n");
	return AC_TRUE;
}

/**
���̃N���X�ł́A���̃��\�b�h���Ă΂��͈̂ȉ��̂����ꂩ�̏�Ԃ̂Ƃ��B
@n �E�^�O�̊J�n�i���j���������Ă���
@n �E�^�O�̏I���i���j�܂Ői�߂�i���̏ꍇ�A����Ɉȉ��̃p�^�[��������j
@n    ���^�O�����w��ƈႤ���߃^�O�I���ʒu�܂œǂݔ�΂��i�u���͍s��Ȃ��j
@n    ���^�O�����w��ƃ}�b�`���A�^�O�̌��ɕ������}������
*/
static const AnalysisCommand CAnalysisExecutor_HtmlTagReplace_forward(CAnalysisExecutor* p_this, 
	const AnalysisCommand cmd, const AcBool isRejectPreCmd, const char c)
{
	CAnalysisExecutor_HtmlTagReplace_Super* self = (CAnalysisExecutor_HtmlTagReplace_Super*)p_this;
	
	//�O��̃R�}���h�����s���ۂ��ꂽ�̂ŏ�ԃ��Z�b�g����
	if(isRejectPreCmd){
		//���Z�b�g
		reset(self);
	}
	//
	AnalysisCommand nextCmd;

	//���������^�O�̌��̈ʒu�܂Ői�߂�ꍇ�B
	if(self->replaceStatus == HtmlTagReplaceStatus_SearchTagEndWithReplace
	   || self->replaceStatus == HtmlTagReplaceStatus_SearchTagEndWithNoReplace)
	{
		//�u���ʒu�i�^�O�̏I���̎��̈ʒu�j�܂ŗ����̂Œu������
		if(self->replaceStatus == HtmlTagReplaceStatus_SearchTagEndWithReplace){
			//�u������ꍇ
			nextCmd.type = AC_REPLACE;
			nextCmd.replace.replacement = self->replaceStr;
			nextCmd.replace.len = 0;
			//�X�e�[�^�X��ύX�i�^�O�̊J�n��T���X�e�[�^�X�Ɂj
			self->replaceStatus = HtmlTagReplaceStatus_SearchStart;
			return nextCmd;
		}
		
		//��Ԃ����Z�b�g���ĉ��̏����ɔC����
		reset(self);
	}

	//�^�O�̊J�n�ʒu��T���ꍇ
	//�^�O�̊J�n�ł͂Ȃ������ꍇ
	if(c != '<'){
		nextCmd.type = AC_FORWARD;
		return nextCmd;
	}

	//�^�O�̊J�n�����������ꍇ�AAC_POS�Ń^�O�̏I���ʒu����������
	self->replaceStatus = HtmlTagReplaceStatus_MatchingTagName;
	nextCmd.type = AC_POS;
	nextCmd.pos.pos = 1;
	//�����ʒu�����Z�b�g
	self->tagNameStrPos = self->tagName;
	self->tagStrLen = 1;
	return nextCmd;
}

static const AnalysisCommand CAnalysisExecutor_HtmlTagReplace_replace(CAnalysisExecutor* p_this, 
	const AnalysisCommand cmd, AcBool result)
{
	CAnalysisExecutor_HtmlTagReplace_Super* self = (CAnalysisExecutor_HtmlTagReplace_Super*)p_this;
	//
	AnalysisCommand nextCmd;
	nextCmd.type = AC_FORWARD;

	//���Z�b�g
	reset(self);

	return nextCmd;
}

/**

*/
static const AnalysisCommand CAnalysisExecutor_HtmlTagReplace_pos(CAnalysisExecutor* p_this, 
	const AnalysisCommand cmd, const char c, const AcBool isEos)
{
	CAnalysisExecutor_HtmlTagReplace_Super* self = (CAnalysisExecutor_HtmlTagReplace_Super*)p_this;
	//
	AnalysisCommand nextCmd;

	++self->tagStrLen;

	//EoS�������ꍇ�ł��̊֐��ɗ����ꍇ�A�}�b�`���^�O�̏I�����������ĂȂ��̂œǂݔ�΂��ďI��������B
	if(isEos){
		nextCmd.type = AC_FORWARD_N;
		nextCmd.forward_n.len = cmd.pos.pos;
		return nextCmd;
	}

	//�^�O���̃}�b�`���O���������Ă���ꍇ
	if(self->replaceStatus == HtmlTagReplaceStatus_MatchingTagName){
		if(tolower(c) == *self->tagNameStrPos){
			//1�����}�b�`�����ꍇ
			++self->tagNameStrPos;
			//�^�O�����S�}�b�`�����ꍇ
			if(*self->tagNameStrPos == '\0'){
				self->replaceStatus = HtmlTagReplaceStatus_MatchingAfterTagName;
			}
		} else{
			//�}�b�`���Ȃ������ꍇ�A�^�O�̏I����T��
			self->replaceStatus = HtmlTagReplaceStatus_SearchTagEndWithNoReplace;
		}
		//���̈ʒu�̕�����ǂݎ��
		nextCmd.type = AC_POS;
		nextCmd.pos.pos = cmd.pos.pos + 1;
		return nextCmd;
	}

	//�^�O���̌��̕������X�y�[�X�⁄�����������Ă���ꍇ
	if(self->replaceStatus == HtmlTagReplaceStatus_MatchingAfterTagName){
		//�^�O�̏I���̏ꍇ
		if(c == '>'){
			//�X�e�[�^�X��ς��āA���̏����ɔC����
			self->replaceStatus = HtmlTagReplaceStatus_SearchTagEndWithReplace;
		} else{
			//�X�y�[�X�̏ꍇ
			if(isspace(c)){
				//�w��^�O�Ƀ}�b�`���^�O�̏I���ʒu��T���X�e�[�^�X�ɕς��āA���̏����ɔC����
				self->replaceStatus = HtmlTagReplaceStatus_SearchTagEndWithReplace;
			} else{
				//�w��^�O�Ƀ}�b�`���Ȃ��������^�O�̏I���ʒu��T���X�e�[�^�X�ɕς��āA���̏����ɔC����
				self->replaceStatus = HtmlTagReplaceStatus_SearchTagEndWithNoReplace;
			}
		}
	}
	
	//�^�O���}�b�`�����A�^�O�̏I���ʒu��T���Ă���ꍇ
	if(self->replaceStatus == HtmlTagReplaceStatus_SearchTagEndWithNoReplace){
		if(c == '>'){
			nextCmd.type = AC_FORWARD_N;
			nextCmd.forward_n.len = self->tagStrLen;
			self->tagStrLen = 0;
		} else{
			nextCmd.type = AC_POS;
			nextCmd.pos.pos = cmd.pos.pos + 1;
		}
		return nextCmd;
	}

	//�^�O���}�b�`���āA�^�O�̏I���ʒu��T���Ă���ꍇ
	if(self->replaceStatus == HtmlTagReplaceStatus_SearchTagEndWithReplace){
		if(c != '>'){//�^�O�̏I���ł͂Ȃ��ꍇ
			nextCmd.type = AC_POS;
			nextCmd.pos.pos = cmd.pos.pos + 1;
		}else{//�^�O�̏I���̏ꍇ
			if(self->isTagReplace){
				//�^�O���̂�u������ꍇ
				nextCmd.type = AC_REPLACE;
				nextCmd.replace.replacement = self->replaceStr;
				nextCmd.replace.len = self->tagStrLen;
			} else{
				//�^�O�̌��ɕ������}������ꍇ
				nextCmd.type = AC_FORWARD_N;
				nextCmd.forward_n.len = self->tagStrLen;
			}
		}
		return nextCmd;
	}

	//�����ɂ͗���͂��Ȃ��̂ŃG���[����������
	nextCmd.type = AC_PASS_THROUGH_ALL;
	fprintf(stderr, "Error in CAnalysisExecutor_HtmlTagReplace_pos");
	return nextCmd;
}

static const AnalysisCommand CAnalysisExecutor_HtmlTagReplace_end(CAnalysisExecutor* p_this, const AnalysisCommand cmd){
	CAnalysisExecutor_HtmlTagReplace_Super* self = (CAnalysisExecutor_HtmlTagReplace_Super*)p_this;
	//
	AnalysisCommand nextCmd;
	nextCmd.type = AC_END;
	nextCmd.end.status = -1;
	nextCmd.end.replacement = NULL;
	//
	//ac_printf("-[end]------------------\n");
	return nextCmd;
}



static void CAnalysisExecutor_HtmlTagReplace_delete(void* p_this){
	CAnalysisExecutor_HtmlTagReplace_Super* self = (CAnalysisExecutor_HtmlTagReplace_Super*)p_this;
	ac_checkClass(gClassName_AnalysisExecutor_HtmlTgaReplace, self->parentMember.publicMember.thisIsClass->name, "in CAnalysisExecutor_HtmlTagReplace_delete()", AC_FALSE);
	//
	free(self->tagName);
	free(self->replaceStr);
	free(self);
}

CAnalysisExecutor_HtmlTagReplace* CAnalysisExecutor_HtmlTagReplace_new(const char* targetTagName, const char* replaceStr, AcBool isTagReplace){
	CAnalysisExecutor_HtmlTagReplace_Super* self = (CAnalysisExecutor_HtmlTagReplace_Super*)malloc(sizeof(CAnalysisExecutor_HtmlTagReplace_Super));
	if(self == NULL) return NULL;
	//�e�����o�[�̏�����
	CAnalysisExecutor_init((CAnalysisExecutor*)self, gClassName_AnalysisExecutor_HtmlTgaReplace,
		CAnalysisExecutor_HtmlTagReplace_delete,
		CAnalysisExecutor_HtmlTagReplace_start,
		CAnalysisExecutor_HtmlTagReplace_forward,
		CAnalysisExecutor_HtmlTagReplace_replace,
		CAnalysisExecutor_HtmlTagReplace_pos,
		CAnalysisExecutor_HtmlTagReplace_end
	);
	//
	self->tagNameStrLen = strlen(targetTagName);
	char* str = (char*)malloc(self->tagNameStrLen + 1);
	if(str == NULL){
		free(self);
		return NULL;
	}
	self->tagName = str;
	self->tagNameStrPos = str;
	//�������ɂ��ĕۊ�
	for(const char* p = targetTagName; *p != '\0'; ++p, ++str)*str = tolower(*p);
	*str = '\0';
	//
	str = (char*)malloc(strlen(replaceStr) + 1);
	if(str == NULL){
		free(self->tagName);
		free(self);
		return NULL;
	}
	strcpy(str, replaceStr);
	self->replaceStr = str;
	//
	self->isTagReplace = isTagReplace;
	self->replaceStatus = HtmlTagReplaceStatus_SearchStart;
	self->tagStrLen = 0;
	//
	CAnalysisExecutor_setDebugPrintFunc((CAnalysisExecutor*)self, CAnalysisExecutor_HtmlTagReplace__debugPrint);
	return (CAnalysisExecutor_HtmlTagReplace*)self;
}





