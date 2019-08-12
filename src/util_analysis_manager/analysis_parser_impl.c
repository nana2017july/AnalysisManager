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
//�֐��Ƃ��Ďg�����߂Ƀ}�N�����폜
#undef isspace
#undef tolower

#include "analysis_parser_.h"
#include "analysis_parser_impl.h"
#include "ac_bucket_utils.h"



static const char* gClassName_CAnalysisParser_CaseNotSensitive = "CAnalysisParser_CaseInsensitive";
static const char* gClassName_CAnalysisParser_Split = "CAnalysisParser_Split";
static const char* gClassName_CAnalysisParser_HttpHeader = "CAnalysisParser_HttpHeader";




//------------------------------------------------------
//----    CAnalysisParser_Split Class       ---------------------
//------------------------------------------------------

typedef struct {
	CAnalysisParser_Super parentMember;
	//
	char* delimiterStr;
	char* delimiterStrPos;
	///�������X�g�b�N���Ă����o�b�t�@
	char* stockStr;
	size_t stockStrBufferSize;
	///���Ƀo�b�t�@�ɕ������������ވʒu
	char* stockStrPos;
} CAnalysisParser_Split_Super;


/**
stockStr�̎��ۂ̕����񒷂��擾����B
*/
static size_t CAnalysisParser_Split_getStockStrLen(CAnalysisParser_Split_Super* p_this){
	CAnalysisParser_Split_Super* self = (CAnalysisParser_Split_Super*)p_this;
	return (size_t)(self->stockStrPos - self->stockStr);
}

/**
stockStr�̎��ۂ̕����񒷂��擾����B
*/
static AcBool CAnalysisParser_Split_isStockStrBufferSizeOver(CAnalysisParser_Split_Super* p_this){
	CAnalysisParser_Split_Super* self = (CAnalysisParser_Split_Super*)p_this;
	return self->stockStrPos >= self->stockStr + self->stockStrBufferSize;
}

void CAnalysisParser_Split_getStockStr(CAnalysisParser_Split* p_this, const char** str, size_t *len){
	CAnalysisParser_Split_Super* self = (CAnalysisParser_Split_Super*)p_this;
	//
	*str = self->stockStr;
	*len = CAnalysisParser_Split_getStockStrLen(self);
}


/**
�f�o�b�O�p�̊֐�
*/
static void CAnalysisParser_Split__debugPrint(const CAnalysisParser* p_this){
	const CAnalysisParser_Split_Super* self = (const CAnalysisParser_Split_Super*)p_this;

	ac_printf("delimiterStr=%s, delimiterStrPos=%s\n", self->delimiterStr, self->delimiterStrPos);
}


static void CAnalysisParser_Split_resetFunc(CAnalysisParser* p_this){
	CAnalysisParser_Split_Super* self = (CAnalysisParser_Split_Super*)p_this;
	//
	self->parentMember.status = AnalysisParserStatus_Searching;
	self->parentMember.isEnd = AC_FALSE;
	self->delimiterStrPos = self->delimiterStr;
	//
	self->stockStrPos = self->stockStr;
}


static void CAnalysisParser_Split_acceptFunc(CAnalysisParser* p_this, const char c){
	CAnalysisParser_Split_Super* self = (CAnalysisParser_Split_Super*)p_this;
	//
	if(self->parentMember.isEnd) return;

	//�X�g�b�N�̃o�b�t�@�T�C�Y�𒴂��Ȃ��ꍇ�̂݃X�g�b�N��������Z�b�g����
	if(CAnalysisParser_Split_getStockStrLen(self) + 1 <= self->stockStrBufferSize){
		//�X�g�b�N��������Z�b�g
		*self->stockStrPos = c;
		++self->stockStrPos;
	}

	//�`�F�b�N����
	if(self->parentMember.status == AnalysisParserStatus_Searching){
		//�J�n�����ƈ�����ꍇ�A���̕����ցB
		if(c != *self->delimiterStrPos) return;
		//�J�n�����ƈ�v�����ꍇ�A�X�e�[�^�X��ς���
		self->parentMember.status = AnalysisParserStatus_Matching;
	} else if(self->parentMember.status == AnalysisParserStatus_Matching){
		//������ƃ}�b�`���Ȃ������ꍇ�A�X�e�[�^�X�����Z�b�g���Ď��ցB
		if(c != *self->delimiterStrPos){
			CAnalysisParser_Split_resetFunc((CAnalysisParser*)self);
			return;
		}
	}

	//�}�b�`�Ώۂ̎��̕����֐i�߂Ă���
	++self->delimiterStrPos;
	//������S�ă}�b�`�����ꍇ�A�X�e�[�^�X���I����ԂɕύX
	if(*self->delimiterStrPos == '\0'){
		self->parentMember.isEnd = AC_TRUE;
		self->parentMember.status = AnalysisParserStatus_Matched;
		*self->stockStrPos = '\0';
	}
}


static void CAnalysisParser_Split_delete(void* p_this){
	CAnalysisParser_Split_Super* self = (CAnalysisParser_Split_Super*)p_this;
	ac_checkClass(gClassName_CAnalysisParser_Split, AcClass_getName(self), "in CAnalysisParser_Split_delete()", AC_FALSE);
	//
	free(self->delimiterStr);
	free(self->stockStr);
	free(self);
}


CAnalysisParser_Split* CAnalysisParser_Split_new(const char* delimiterStr, size_t stockStrSize){
	CAnalysisParser_Split_Super* self = (CAnalysisParser_Split_Super*)malloc(sizeof(CAnalysisParser_Split_Super));
	if(self == NULL) return NULL;
	//
	CAnalysisParser_init((CAnalysisParser*)self, gClassName_CAnalysisParser_Split,
		CAnalysisParser_Split_delete,
		CAnalysisParser_Split_acceptFunc,
		CAnalysisParser_Split_resetFunc
	);
	//
	size_t len = strlen(delimiterStr);
	self->delimiterStr = (char*)malloc(len + 1);
	if(self->delimiterStr == NULL) return NULL;
	strcpy(self->delimiterStr, delimiterStr);
	//
	self->delimiterStrPos = self->delimiterStr;
	//
	self->stockStr = (char*)malloc(stockStrSize + 1);
	self->stockStrBufferSize = stockStrSize;
	self->stockStrPos = self->stockStr;
	//
	CAnalysisParser_setDebugFunc((CAnalysisParser*)self, CAnalysisParser_Split__debugPrint);
	//
	return (CAnalysisParser_Split*)self;
}












//------------------------------------------------------
//----    CAnalysisParser_CaseInsensitive Class       ---------------------
//------------------------------------------------------

typedef struct {
	CAnalysisParser_Super parentMember;
	//
	char* targetStr;
	char* targetStrPos;
} CAnalysisParser_CaseInsensitive_Super;



/**public:
�f�o�b�O�p�̊֐�
*/
static void CAnalysisParser_CaseInsensitive__debugPrint(const CAnalysisParser* p_this){
	const CAnalysisParser_CaseInsensitive_Super* self = (const CAnalysisParser_CaseInsensitive_Super*)p_this;

	ac_printf("targetStr=%s, targetStrPos=%s\n", self->targetStr, self->targetStrPos);
}


static void CAnalysisParser_CaseInsensitive_acceptFunc(CAnalysisParser* p_this, const char c){
	CAnalysisParser_CaseInsensitive_Super* self = (CAnalysisParser_CaseInsensitive_Super*)p_this;
	//
	if(self->parentMember.isEnd) return;
	//�}�b�`�m�F��
	if(self->parentMember.status == AnalysisParserStatus_Matching){
		//�}�b�`���Ȃ������ꍇ�A�I����
		if(tolower(c) != *self->targetStrPos){
			self->parentMember.isEnd = AC_TRUE;
			return;
		}
	}

	//�}�b�`�����ꍇ�A���̈ʒu��
	++self->targetStrPos;
	//���S�}�b�`�����ꍇ�A�X�e�[�^�X���I����Ԃɂ���B
	if(*self->targetStrPos == '\0'){
		self->parentMember.status = AnalysisParserStatus_Matched;
		self->parentMember.isEnd = AC_TRUE;
	}
}


static void CAnalysisParser_CaseInsensitive_resetFunc(CAnalysisParser* p_this){
	CAnalysisParser_CaseInsensitive_Super* self = (CAnalysisParser_CaseInsensitive_Super*)p_this;
	//
	self->parentMember.status = AnalysisParserStatus_Matching;
	self->parentMember.isEnd = AC_FALSE;
	//
	self->targetStrPos = self->targetStr;
}



static void CAnalysisParser_CaseInsensitive_delete(void* p_this){
	CAnalysisParser_CaseInsensitive_Super* self = (CAnalysisParser_CaseInsensitive_Super*)p_this;
	ac_checkClass(gClassName_CAnalysisParser_CaseNotSensitive, AcClass_getName(self), "in CAnalysisParser_CaseInsensitive_delete()", AC_FALSE);
	//
	free(self->targetStr);
	free(self);
}


CAnalysisParser_CaseInsensitive* CAnalysisParser_CaseInsensitive_new(const char* targetStr){
	CAnalysisParser_CaseInsensitive_Super* self = (CAnalysisParser_CaseInsensitive_Super*)malloc(sizeof(CAnalysisParser_CaseInsensitive_Super));
	if(self == NULL) return NULL;
	//
	CAnalysisParser_init((CAnalysisParser*)self, gClassName_CAnalysisParser_CaseNotSensitive,
		CAnalysisParser_CaseInsensitive_delete, 
		CAnalysisParser_CaseInsensitive_acceptFunc,
		CAnalysisParser_CaseInsensitive_resetFunc
	);
	self->targetStr = ac_copyToLowerCase(targetStr);
	self->targetStrPos = self->targetStr;
	CAnalysisParser_CaseInsensitive_resetFunc((CAnalysisParser*)self);
	CAnalysisParser_setDebugFunc((CAnalysisParser*)self, CAnalysisParser_CaseInsensitive__debugPrint);
	//
	return (CAnalysisParser_CaseInsensitive*)self;
}









//------------------------------------------------------
//----    CAnalysisParser_HttpHeader Class       ---------------------
//------------------------------------------------------

typedef enum{
	HttpHeaderStatus_Attribute,
	HttpHeaderStatus_Value,
	///���������}�b�`���Ȃ������̂Ńw�b�_�̍Ō��T��
	HttpHeaderStatus_HeaderEndSearching,
	///�w�b�_�̍Ō�̈ꕔ���}�b�`�����̂ōŌ��T��
	HttpHeaderStatus_HeaderEndMatching,

} HttpHeaderStatus;

typedef struct {
	CAnalysisParser_Super parentMember;
	//
	CAnalysisParser_CaseInsensitive* attrNameParser;
	CAnalysisParser_Split* valueParser;
	HttpHeaderStatus status;
} CAnalysisParser_HttpHeader_Super;



/**public:
�f�o�b�O�p�̊֐�
*/
static void CAnalysisParser_HttpHeader__debugPrint(const CAnalysisParser* p_this){
	const CAnalysisParser_HttpHeader_Super* self = (const CAnalysisParser_HttpHeader_Super*)p_this;

	ac_printf("status=%d\n", (int)self->status);
	if(self->attrNameParser != NULL){
		CANALYSIS_PARSER_DEBUG_PRINT((CAnalysisParser*)self->attrNameParser);
	}
	if(self->valueParser != NULL){
		CANALYSIS_PARSER_DEBUG_PRINT((CAnalysisParser*)self->valueParser);
	}
}


static void CAnalysisParser_HttpHeader_acceptFunc(CAnalysisParser* p_this, const char c){
	CAnalysisParser_HttpHeader_Super* self = (CAnalysisParser_HttpHeader_Super*)p_this;
	//
	if(self->parentMember.isEnd) return;
	//�}�b�`�m�F��
	if(self->status == HttpHeaderStatus_Attribute){
		//����������������
		CAnalysisParser* parser = (CAnalysisParser*)self->attrNameParser;
		CAnalysisParser_accept(parser, c);
		if(CAnalysisParser_isEnd(parser)){
			if(CAnalysisParser_isMatched(parser)){
				//���������w��̖��O�������ꍇ�A�l���p�[�X����X�e�[�^�X�ɕύX����
				self->status = HttpHeaderStatus_Value;
			} else{
				//���������w��̂��̂ł͂Ȃ������ꍇ�A�s�̍Ō��T��
				self->status = HttpHeaderStatus_HeaderEndSearching;
			}
		}
	} else if(self->status == HttpHeaderStatus_Value){
		//���������}�b�`�����̂ŁA�l���R�s�[����
		CAnalysisParser* parser = (CAnalysisParser*)self->valueParser;
		CAnalysisParser_accept(parser, c);
		if(CAnalysisParser_isEnd(parser)){
			//�p�[�X���I���ɂ���
			self->parentMember.status = AnalysisParserStatus_Matched;
			self->parentMember.isEnd = AC_TRUE;
		}
	}
	
	//
	if(self->status == HttpHeaderStatus_HeaderEndSearching){
		//���������}�b�`���Ȃ������̂Œl�͖������āA�s�̏I���(\r)��T���B
		if(c == '\r'){
			//\n��T���i���̕��@����\rX\n�̂悤�ȕ�����̏ꍇ���q�b�g���Ă��܂������ӂ̂���ꍇ�݂̂Ȃ̂ŗǂ��Ƃ���j
			self->status = HttpHeaderStatus_HeaderEndMatching;
		}
	} else if(self->status == HttpHeaderStatus_HeaderEndMatching){
		//�s�̏I���(\n)��T���B
		if(c == '\n'){
			//�p�[�X���I���ɂ���
			self->parentMember.isEnd = AC_TRUE;
		}
	}
}


static void CAnalysisParser_HttpHeader_resetFunc(CAnalysisParser* p_this){
	CAnalysisParser_HttpHeader_Super* self = (CAnalysisParser_HttpHeader_Super*)p_this;
	//
	self->parentMember.status = AnalysisParserStatus_Matching;
	self->parentMember.isEnd = AC_FALSE;
	//�����̕ϐ������Z�b�g
	self->status = HttpHeaderStatus_Attribute;
	CAnalysisParser_reset((CAnalysisParser*)self->attrNameParser);
	CAnalysisParser_reset((CAnalysisParser*)self->valueParser);
}

void CAnalysisParser_HttpHeader_getValue(CAnalysisParser_HttpHeader* p_this, const char** str, size_t* len){
	CAnalysisParser_HttpHeader_Super* self = (CAnalysisParser_HttpHeader_Super*)p_this;
	//��������擾����
	CAnalysisParser_Split_getStockStr(self->valueParser, str, len);
}

static void CAnalysisParser_HttpHeader_delete(void* p_this){
	CAnalysisParser_HttpHeader_Super* self = (CAnalysisParser_HttpHeader_Super*)p_this;
	ac_checkClass(gClassName_CAnalysisParser_HttpHeader, AcClass_getName(self), "in CAnalysisParser_HttpHeader_delete()", AC_FALSE);
	//
	AcClass_delete(self->attrNameParser);
	AcClass_delete(self->valueParser);
	free(self);
}


CAnalysisParser_HttpHeader* CAnalysisParser_HttpHeader_new(const char* headearName, size_t stockStrSize){
	CAnalysisParser_HttpHeader_Super* self = (CAnalysisParser_HttpHeader_Super*)malloc(sizeof(CAnalysisParser_HttpHeader_Super));
	if(self == NULL) return NULL;
	//
	CAnalysisParser_init((CAnalysisParser*)self, gClassName_CAnalysisParser_HttpHeader,
		CAnalysisParser_HttpHeader_delete,
		CAnalysisParser_HttpHeader_acceptFunc,
		CAnalysisParser_HttpHeader_resetFunc
	);
	//
	self->attrNameParser = CAnalysisParser_CaseInsensitive_new(headearName);
	self->valueParser = CAnalysisParser_Split_new("\r\n", 300);
	CAnalysisParser_setDebugFunc((CAnalysisParser*)self, CAnalysisParser_HttpHeader__debugPrint);
	//����������
	CAnalysisParser_HttpHeader_resetFunc((CAnalysisParser*)self);
	return (CAnalysisParser_HttpHeader*)self;
}




