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
#include "analysis_manager_.h"

/**
@file 
<pre>
19.07.22 BucketController_initCurrentVar() �̕s��C���BSENTINEL�̂Ƃ���Segmentation fault��������C���B
</pre>
*/


static const char* gClassName_BucketController = "BucketCotroller";
static const char* gClassName_AnalysisManager = "AnalysisManger";

//��`
const int CBUCKET_CONTROLLER_ERR_POS = -10000;

//------------------------------------------------------
//----    BucketCotroller Class       ---------------------
//------------------------------------------------------
//------------------------------------------------------

/**public:
�f�o�b�O�p�̊֐�
*/
void CBucketController__debugPrint(const CBucketController* p_this, const char* file, int line) {
	const CBucketController_Super* self = (const CBucketController_Super*)p_this;
	char format[50];
	const char* filename = (!strrchr(file, '\\') ? file : strrchr(file, '\\') + 1);

	ac_printf("\n-------debugPrint(%s, %d):\nBucketController[%p], \n", filename, line, self);
	ac_printf("current[currentBucketStrLen=%d,currentBucketStrPos=%d,", (int)self->currentBucketStrLen, self->currentBucketStrPos);
	if (self->currentBucketStrLen > 0) {
		sprintf(format, "currentBucketStr=%%.%ds,", (int)self->currentBucketStrLen);
		ac_printf(format, self->currentBucketStr);
	}
	ac_printf("currentBucket=%s],", sctest_getBucketType(self->currentBucket));
	ac_printf("\ninputBrigade=");
	ac_printBrigade(self->inputBrigade);
	ac_printf("\nmodifiedBrigate=");
	ac_printBrigade(self->modifiedBrigate);
	ac_printf(", isEndBuffer=%d, ", self->isEndBuffer);
	ac_printf("\n-----------------------------------------------------\n");
}




/**private:
������bucket���J�����g�̒l�Ƃ��āAcurrent*�ϐ�������������B
���J�����g�̕����̈ʒu��-1�ɂ���̂ŁA����curBucket�̐擪�����̈ʒu�ɂ������ꍇ�͂��̊֐��Ăяo�����+1���邱�ƁB
bucket��EOS�̏ꍇ�͓����̃X�e�[�^�X��EOS�̏�Ԃɏ����ς���B
@param curBucket [in]����bucket�ŏ���������
*/
static void BucketController_initCurrentVar(CBucketController_Super* p_this, apr_bucket* curBucket) {
	CBucketController_Super* self = (CBucketController_Super*)p_this;
	//�擪��bucket�̕������ǂݍ���
	const char* bucketStr = NULL;
	apr_size_t bucketStrLen = 0;
	if (!APR_BUCKET_IS_METADATA(curBucket) && curBucket != APR_BRIGADE_SENTINEL(self->inputBrigade)) {
		apr_status_t status = apr_bucket_read(curBucket, &bucketStr, &bucketStrLen, APR_BLOCK_READ);
	}
	else if (APR_BUCKET_IS_EOS(curBucket)) {
		self->isEndBuffer = AC_TRUE;
	}
	//�ݒ�
	self->currentBucket = curBucket;
	self->currentBucketStr = bucketStr;
	self->currentBucketStrLen = bucketStrLen;
	self->currentBucketStrPos = -1;
}


/**
�w���bucket��brigade����؂藣���āA�w���brigade�̍Ō���Ɉړ����邾���̃��\�b�h�B
@param bb [out]bucket���Ō���ɒǉ�����brigade�B
@param bucket [in]�ǂ�����brigade�ɏ������Ă���bucket�B
*/
static void moveBucketToBrigade(apr_bucket_brigade* bb, apr_bucket* bucket) {
	//����brigade����bucket��؂藣���i�R�Â��𖳂����j�B
	APR_BUCKET_REMOVE(bucket);
	//������brigade�̍Ō���ɒǉ�����
	APR_BRIGADE_INSERT_TAIL(bb, bucket);
}

/**
brigade�̒���bucket��؂藣���āA�w���brigade�̍Ō���ɑS�Ēǉ�����B
@param in [out]src��bucket���ǉ����ꂽbrigade���Ԃ�
@param src [in]�ǉ��Ώۂ�bucket���܂�brigade�B
@n         [out]��ɂȂ���brigade�B
*/
static void moveBrigadeToBrigade(apr_bucket_brigade* in, apr_bucket_brigade* src){
	while(!APR_BRIGADE_EMPTY(src)){
		apr_bucket* b = APR_BRIGADE_FIRST(src);
		//�ړ�����
		moveBucketToBrigade(in, b);
	}
}

/**
�w���bucket��brigade����؂藣���āA�w���brigade�̍Ō���ɒǉ�����B
�������AEOS�ȊO��META��bucket�͓ǂݎ̂Ă�B
@param bb [out]META�ȊO��bucket�����ׂĒǉ�����brigade�B
@param in [in]�ړ����B�֐��Ăяo����́A���brigade�ɂȂ�B
*/
static void moveAllBucketsToBrigadeWithoutMeta(apr_bucket_brigade* bb, apr_bucket_brigade* in) {
	apr_bucket* b = NULL;
	while (!APR_BRIGADE_EMPTY(in)) {
		b = APR_BRIGADE_FIRST(in);
		//�ړ�����
		if (APR_BUCKET_IS_EOS(b)) {
			moveBucketToBrigade(bb, b);
			return;
		}
		else if (b == APR_BRIGADE_SENTINEL(in)) {
			return;
		}
		else if (APR_BUCKET_IS_METADATA(b) || APR_BUCKET_IS_FLUSH(b)) {
			apr_bucket_delete(b);
		}
		else {
			moveBucketToBrigade(bb, b);
		}
	}
}
/**
�������ʂ�ۑ�����\���́B
*/
typedef struct _BucketSearchResult {
	///������Ȃ������ꍇ��NULL
	apr_bucket* bucket;
	///bucket�̒��̈ʒu�B
	size_t bucketStrPos;
} _BucketSearchResult;

/**
brigade�̐擪bucket�̎w��̈ʒu���N�_�Ƃ��āA�w��̈ʒu��bucket��T���B
�w��̈ʒu��bucket�̒��̂ǂ̈ʒu���w��̈ʒu�����Ԃ��B
@param startStrPos [in]�J�n�ʒu�i0�`�j�Bbrigade�̍ŏ���bucket�̒��̕�����̈ʒu���w��B�w��ʒu���ŏ���bucket�𒴂���ꍇ�͌�����Ȃ��G���[��Ԃ��B
@param posFromStart [in]�J�n�ʒu����̈ʒu�i0�`�j�B�Ⴆ�ΊJ�n�ʒu��str[1]�̏ꍇ�ŁA0���w�肷���str[1]�̈ʒu���Ԃ�B
@return ���������ʒu��bucket�̃|�C���^�Ƃ��̒��̕�����̈ʒu��Ԃ��B������Ȃ������ꍇ�AbucketStrPos=-1���ݒ肳���B
*/
static const _BucketSearchResult searchBucketPos(apr_bucket_brigade* brigade, size_t startStrPos, size_t posFromStart) {
	_BucketSearchResult result = {NULL, 0};
	apr_bucket* bucket = APR_BRIGADE_FIRST(brigade);
	const char* bucketStr = NULL;
	apr_size_t bucketStrLen = 0; 
	apr_status_t status = apr_bucket_read(bucket, &bucketStr, &bucketStrLen, APR_BLOCK_READ);
	//�ŏ���bucket�̕����񒷂��w��̒����������Ă���ꍇ�̓G���[
	if (bucketStrLen < startStrPos) return result;
	//�ŏ���bucket�Ŏw��̒����𖞂����ꍇ
	size_t len = bucketStrLen - startStrPos;
	if (posFromStart < len) {
		result.bucket = bucket;
		result.bucketStrPos = startStrPos + posFromStart;
		return result;
	}
	//�ړI�̈ʒu��T��
	for (bucket = APR_BUCKET_NEXT(bucket); bucket != APR_BRIGADE_SENTINEL(brigade); bucket = APR_BUCKET_NEXT(bucket)) {
		apr_status_t status = apr_bucket_read(bucket, &bucketStr, &bucketStrLen, APR_BLOCK_READ);
		len += (size_t)bucketStrLen;
		//����bucket�ɖړI�̈ʒu������ꍇ
		if (posFromStart < len) {
			result.bucket = bucket;
			result.bucketStrPos = bucketStrLen - (len - posFromStart);
			return result;
		}
	}
	//brigade�̍Ō�܂ŒB�����B
	return result;
}


/** private:
�w���inputBucket��brigade����؂藣���āAmodifiedBrigade�̍Ō���ɒǉ�����B
��������̃J�����g�ʒu��-1�ɃZ�b�g�����B
@param inputBucket [in]brigade�ɑ}������bucket�Bself->inputBrigade��bucket����͂ɂ���B
*/
static void BucketController_moveInputBucketToModifiedBrigade(CBucketController_Super* p_this, apr_bucket* inputBucket) {
	CBucketController_Super* self = (CBucketController_Super*)p_this;
	//
	moveBucketToBrigade(self->modifiedBrigate, inputBucket);
	//����������
	apr_bucket* curBucket = APR_BRIGADE_FIRST(self->inputBrigade);
	BucketController_initCurrentVar(self, curBucket);
}




AcBool CBucketController_isContainingEos(const CBucketController* p_this) {
	const CBucketController_Super* self = (const CBucketController_Super*)p_this;
	return self->isContainingEos;
}



AcBool CBucketController_isEOS(const CBucketController* p_this) {
	const CBucketController_Super* self = (const CBucketController_Super*)p_this;
	//
	return APR_BUCKET_IS_EOS(self->currentBucket);
}



AcBool CBucketController_isEndBuffer(const CBucketController* p_this) {
	const CBucketController_Super* self = (const CBucketController_Super*)p_this;
	//
	return (self->isEndBuffer);
}


AcBool CBucketController_isInputEmpty(const CBucketController* p_this) {
	const CBucketController_Super* self = (const CBucketController_Super*)p_this;
	//
	return APR_BRIGADE_EMPTY(self->inputBrigade);
}


AcBool CBucketController_isModifiedEmpty(const CBucketController* p_this) {
	const CBucketController_Super* self = (const CBucketController_Super*)p_this;
	//
	return APR_BRIGADE_EMPTY(self->modifiedBrigate);
}


const int CBucketController_pos(const CBucketController* p_this, const size_t pos) {
	const CBucketController_Super* self = (const CBucketController_Super*)p_this;
	//
	//brigade�̏I���ɗ��Ă���ꍇ�B
	if (CBucketController_isEndBuffer((const CBucketController*)self)) return CBUCKET_CONTROLLER_ERR_POS;
	//�J�n���Ă��Ȃ��ꍇ
	if (self->currentBucketStrPos == -1) return CBUCKET_CONTROLLER_ERR_POS;
	apr_bucket* bucket = self->currentBucket;
	size_t len = self->currentBucketStrLen - self->currentBucketStrPos;
	if (pos < len) {
		return self->currentBucketStr[self->currentBucketStrPos + pos];
	}
	//
	const char* bucketStr = NULL;
	apr_size_t bucketStrLen = 0;
	for (bucket = APR_BUCKET_NEXT(bucket); bucket != APR_BRIGADE_SENTINEL(self->inputBrigade); bucket = APR_BUCKET_NEXT(bucket)) {
		apr_status_t status = apr_bucket_read(bucket, &bucketStr, &bucketStrLen, APR_BLOCK_READ);
		//����bucket�ɖړI�̈ʒu������ꍇ
		if (pos < len + bucketStrLen) {
			return bucketStr[pos - len];
		}
		len += (size_t)bucketStrLen;
	}
	//�ړI�̈ʒu��������O��bucket���Ȃ��Ȃ���
	return CBUCKET_CONTROLLER_ERR_POS;
}


AcBool CBucketController_forward(CBucketController* p_this) {
	CBucketController_Super* self = (CBucketController_Super*)p_this;
	//brigade�̏I���ɗ��Ă���ꍇ�B
	if (CBucketController_isEndBuffer((CBucketController*)self)) return AC_FALSE;
	//
	++self->currentBucketStrPos;
	//�J�����g��bucket�̒��ňړ����ł����ꍇ
	if (self->currentBucketStrPos < self->currentBucketStrLen) {
		return AC_TRUE;
	}

	//����bucket�Ɉړ�
	apr_bucket* bucket = self->currentBucket;
	self->currentBucket = APR_BUCKET_NEXT(bucket);
	BucketController_moveInputBucketToModifiedBrigade(self, bucket);
	++self->currentBucketStrPos;
	//
	if (APR_BRIGADE_EMPTY(self->inputBrigade) || APR_BUCKET_IS_EOS(self->currentBucket)) {
		self->isEndBuffer = AC_TRUE;
		return AC_FALSE;
	}
	return AC_TRUE;
}


AcBool CBucketController_execReplace(CBucketController* p_this, size_t len, const char* replaceStr) {
	CBucketController_Super* self = (CBucketController_Super*)p_this;
	//��x��forward���Ă΂�ĂȂ��ꍇ
	if (self->currentBucketStrPos == -1) return AC_FALSE;
	//inputBrigade����Ȃǃo�b�t�@�̏I���̏ꍇ
	if (CBucketController_isEndBuffer((CBucketController*)self)) return AC_FALSE;
	apr_bucket* b = self->currentBucket;
	//�u���J�n�ʒu��split(�J�����g�̐擪�̏ꍇ�͕����s�v�̂��߉������Ȃ�)
	if (self->currentBucketStrPos != 0) {
		apr_bucket_split(b, self->currentBucketStrPos);
		//�u���Ώۂ���Ȃ���������Ō���ɒǉ�
		BucketController_moveInputBucketToModifiedBrigade(self, APR_BRIGADE_FIRST(self->inputBrigade));
	}

	//����0�̒u���̏ꍇ
	if (len == 0) {
		//�u����������Ō���ɒǉ��B
		apr_bucket* tmpb = apr_bucket_transient_create(replaceStr, strlen(replaceStr), self->bucketAlloc);
		APR_BRIGADE_INSERT_TAIL(self->modifiedBrigate, tmpb);
		//bucket�ʒu�╶����̈ʒu���̏�����
		BucketController_initCurrentVar(self, APR_BRIGADE_FIRST(self->inputBrigade));
		return AC_TRUE;
	}
	//�u���I���ʒu����������
	const _BucketSearchResult result = searchBucketPos(self->inputBrigade, 0, len - 1);
	//�ʒu��������Ȃ������ꍇ�i�o�b�t�@�̃G���h�ɒB�����ꍇ�Ȃǁj
	if (result.bucket == NULL) {
		//bucket�ʒu�╶����̈ʒu���̏�����
		BucketController_initCurrentVar(self, APR_BRIGADE_FIRST(self->inputBrigade));
		return AC_FALSE;
	}

	//�u����������Ō���ɒǉ��B
	apr_bucket* tmpb = apr_bucket_transient_create(replaceStr, strlen(replaceStr), self->bucketAlloc);
	APR_BRIGADE_INSERT_TAIL(self->modifiedBrigate, tmpb);

	//�u���Ώ�bucket���폜����
	while ((b = APR_BRIGADE_FIRST(self->inputBrigade)) != result.bucket) {
		//�擪��bucket���폜����
		apr_bucket_delete(b);
	}
	//inputBrigade�̐擪��bucket�𕪊�����
	apr_bucket_split(b, result.bucketStrPos + 1);
	//��������bucket���폜����i�u���Ώہj
	apr_bucket_delete(b);
	//bucket�ʒu�╶����̈ʒu���̏�����
	BucketController_initCurrentVar(self, APR_BRIGADE_FIRST(self->inputBrigade));
	//�擪��bucket��0�����̏ꍇ�A�폜����
	if (self->currentBucketStrLen == 0 && self->currentBucketStr != NULL) {
		apr_bucket_delete(self->currentBucket);
		BucketController_initCurrentVar(self, APR_BRIGADE_FIRST(self->inputBrigade));
	}

	return AC_TRUE;
}


void CBucketController_moveAllInputBucketToModifiedBrigade(CBucketController* p_this) {
	CBucketController_Super* self = (CBucketController_Super*)p_this;
	//
	while (!APR_BRIGADE_EMPTY(self->inputBrigade)) {
		BucketController_moveInputBucketToModifiedBrigade(self, APR_BRIGADE_FIRST(self->inputBrigade));
	}
}


void CBucketController_addStringToModifiedBrigade(CBucketController* p_this, const char* replaceStr) {
	CBucketController_Super* self = (CBucketController_Super*)p_this;
	
	//�ǉ��ʒu��bucket���擾
	apr_bucket* b = APR_BRIGADE_LAST(self->modifiedBrigate);
	//�Ō��bucket��EOS�̏ꍇ�A����1�O���w�肷��
	if (APR_BUCKET_IS_EOS(b)) {
		b = APR_BUCKET_PREV(b);
	}

	//��������쐬�B
	apr_bucket* tmpb = apr_bucket_transient_create(replaceStr, strlen(replaceStr), self->bucketAlloc);
	//bucket�̌�ɒǉ�
	APR_BUCKET_INSERT_AFTER(b, tmpb);
}




void CBucketController_addBrigade(CBucketController* p_this, apr_bucket_brigade* bb) {
	CBucketController_Super* self = (CBucketController_Super*)p_this;
	//
	apr_bucket* currentBucket = self->currentBucket;

	//����brigade�ɂ��ׂĈړ�����
	moveAllBucketsToBrigadeWithoutMeta(self->inputBrigade, bb);
	if (APR_BUCKET_IS_EOS(APR_BRIGADE_LAST(self->inputBrigade))) {
		self->isContainingEos = AC_TRUE;
	}

	//�Ō���̎���bucket�̏ꍇ
	if (currentBucket == APR_BRIGADE_SENTINEL(self->inputBrigade)) {
		//�J�����gbucket��擪�ɂ���
		BucketController_initCurrentVar(self, APR_BRIGADE_FIRST(self->inputBrigade));
	}

	//get()�Ŏ��̕����Ɉړ��ł���悤��
	self->isEndBuffer = AC_FALSE;
}



apr_bucket_brigade* CBucketController_exportModifiedBrigade(CBucketController* p_this) {
	CBucketController_Super* self = (CBucketController_Super*)p_this;
	//
	int a = 0;
	if(a == 1){
		CBUCKET_CONTROLLER_DEBUG_PRINT((CBucketController*)self);
	}

	apr_bucket_brigade* bb = self->modifiedBrigate;
	//�V����brigade�̈���쐬
	self->modifiedBrigate = apr_brigade_create(self->pool, self->bucketAlloc);
	return bb;
}


void CBucketController_exportModifiedBrigadeToBrigade(CBucketController* p_this, apr_bucket_brigade* outbb){
	CBucketController_Super* self = (CBucketController_Super*)p_this;
	//�ړ�����
	apr_bucket_brigade* in = CBucketController_exportModifiedBrigade((CBucketController*)self);
	moveBrigadeToBrigade(outbb, in);

	//�j�󂷂�
	apr_brigade_destroy(in);
}


/**private:
�f�X�g���N�^
*/
static void BucketController_delete(void* p_this) {
	CBucketController_Super* self = (CBucketController_Super*)p_this;
	ac_checkClass(gClassName_BucketController, self->publicMember.thisIsClass->name, "in BucketController_delete()", AC_FALSE);
	//
	if (self->modifiedBrigate != NULL) {
		apr_brigade_destroy(self->modifiedBrigate);
		self->modifiedBrigate = NULL;
	}
	if (self->inputBrigade != NULL) {
		apr_brigade_destroy(self->inputBrigade);
		self->inputBrigade = NULL;
	}
	//bucket_alloc��1�̃X���b�h��1��������Ă͂����Ȃ��炵��
	//if (self->bucketAlloc != NULL) {
	//	apr_bucket_alloc_destroy(self->bucketAlloc);
	//	self->bucketAlloc = NULL;
	//}
	//
	free(self);
}



CBucketController* CBucketController_new(apr_pool_t* pool, apr_bucket_alloc_t* ba, apr_bucket_brigade* bb) {
	CBucketController_Super* self = (CBucketController_Super*)malloc(sizeof(CBucketController_Super));
	if (self == NULL) return NULL;
	//
	self->publicMember.thisIsClass = AcCThisIsClass_new(gClassName_BucketController, BucketController_delete);
	//
	//���͂�bucket_brigade��ݒ�
	self->inputBrigade = apr_brigade_create(pool, ba);
	moveAllBucketsToBrigadeWithoutMeta(self->inputBrigade, bb);
	//
	self->isContainingEos = AC_FALSE;
	if (APR_BUCKET_IS_EOS(APR_BRIGADE_LAST(self->inputBrigade))) {
		self->isContainingEos = AC_TRUE;
	}
	BucketController_initCurrentVar(self, APR_BRIGADE_FIRST(self->inputBrigade));
	self->isEndBuffer = AC_FALSE;

	//�C��������brigade�֘A�̏���ݒ�
	self->pool = pool;
	self->bucketAlloc = ba;
	self->modifiedBrigate = apr_brigade_create(pool, ba);

	return (CBucketController*)self;
}




//------------------------------------------------------
//----    CAnalysisExecutor Class     ---------------------
//------------------------------------------------------
//------------------------------------------------------


void CAnalysisExecutor__debugPrint(const CAnalysisExecutor* p_this, const char* file, int line) {
	const CAnalysisExecutor_Super* self = (const CAnalysisExecutor_Super*)p_this;
	//char format[50];
	const char* filename = (!strrchr(file, '\\') ? file : strrchr(file, '\\') + 1);
	
	ac_printf("\n-----------------------------------------------------\n");
	ac_printf("-------debugPrint(%s, %d):\n%s[%p], \n", filename, line, self->publicMember.thisIsClass->name, self);
	//���Ƃ͔h���N���X�ɔC����
	if (self->debugPrintFunc != NULL) {
		self->debugPrintFunc((const CAnalysisExecutor*)self);
	}
	ac_printf("-----------------------------------------------------\n");
}


AcBool CAnalysisExecutor_startFunc(CAnalysisExecutor* p_this, const apr_table_t* table) {
	CAnalysisExecutor_Super* self = (CAnalysisExecutor_Super*)p_this;
	return self->startFunc((CAnalysisExecutor*)self, table);
}


const AnalysisCommand CAnalysisExecutor_forwardFunc(CAnalysisExecutor* p_this, const AnalysisCommand cmd, 
	const AcBool isRejectPreCmd, const char c)
{
	CAnalysisExecutor_Super* self = (CAnalysisExecutor_Super*)p_this;
	return self->forwardFunc((CAnalysisExecutor*)self, cmd, isRejectPreCmd, c);
}


const AnalysisCommand CAnalysisExecutor_replaceFunc(CAnalysisExecutor* p_this, const AnalysisCommand cmd, 
	AcBool result) 
{
	CAnalysisExecutor_Super* self = (CAnalysisExecutor_Super*)p_this;
	return self->replaceFunc((CAnalysisExecutor*)self, cmd, result);
}


const AnalysisCommand CAnalysisExecutor_posFunc(CAnalysisExecutor* p_this, const AnalysisCommand cmd, 
	const char c, const AcBool isEos)
{
	CAnalysisExecutor_Super* self = (CAnalysisExecutor_Super*)p_this;
	return self->posFunc((CAnalysisExecutor*)self, cmd, c, isEos);
}


const AnalysisCommand CAnalysisExecutor_endFunc(CAnalysisExecutor* p_this, const AnalysisCommand cmd) {
	CAnalysisExecutor_Super* self = (CAnalysisExecutor_Super*)p_this;
	return self->endFunc((CAnalysisExecutor*)self, cmd);
}


void CAnalysisExecutor_setDebugPrintFunc(CAnalysisExecutor* p_this, void (*debugPrintFunc)(const CAnalysisExecutor* p_this)) {
	CAnalysisExecutor_Super* self = (CAnalysisExecutor_Super*)p_this;
	//
	self->debugPrintFunc = debugPrintFunc;
}


static void CAnalysisExecutor_delete(void* p_this) {
	CAnalysisExecutor_Super* self = (CAnalysisExecutor_Super*)p_this;
	self->subclassDeleteFunc(self);
	//self�͔h���N���X��free����B���������N���X��free���郋�[���B
	//free(self);
}


void CAnalysisExecutor_init(CAnalysisExecutor* p_this, const char* className, AcDeleteFunc_T deleteFunc,
	AcBool(*startFunc)(CAnalysisExecutor* p_this, const apr_table_t* table),
	const AnalysisCommand(*forwardFunc)(CAnalysisExecutor* p_this, const AnalysisCommand cmd, const AcBool isRejectPreCmd, const char c),
	const AnalysisCommand(*replaceFunc)(CAnalysisExecutor* p_this, const AnalysisCommand cmd, AcBool result),
	const AnalysisCommand(*posFunc)(CAnalysisExecutor* p_this, const AnalysisCommand cmd,const char c, const AcBool isEos),
	const AnalysisCommand(*endFunc)(CAnalysisExecutor* p_this, const AnalysisCommand cmd)
){
	//
	CAnalysisExecutor_Super* self = (CAnalysisExecutor_Super*)p_this;
	self->publicMember.thisIsClass = AcCThisIsClass_new(className, CAnalysisExecutor_delete);
	self->startFunc = startFunc;
	self->forwardFunc = forwardFunc;
	self->posFunc = posFunc;
	self->replaceFunc = replaceFunc;
	self->endFunc = endFunc;
	self->subclassDeleteFunc = deleteFunc;
	//
	self->debugPrintFunc = NULL;
}




//------------------------------------------------------
//----    CAnalysisManager Class     ---------------------
//------------------------------------------------------
//------------------------------------------------------
enum _RunnningStatus{
	RunningStatus_NotStarted=0,
	RunningStatus_Running=1,
	RunningStatus_End=2
};

void AnalysisCommand__debugPrint(const AnalysisCommand cmd) {
	ac_printf("AnalysisCommand{");
	switch (cmd.type) {
	case AC_FORWARD:
		ac_printf("type=forward, ");
		break;
	case AC_POS:
		ac_printf("type=pos, pos=%d", (int)cmd.pos.pos);
		break;
	case AC_REPLACE:
		ac_printf("type=replace, len=%d, replacement=%s", (int)cmd.replace.len, cmd.replace.replacement);
		break;
	case AC_END:
		ac_printf("type=end, status=%d, replacement=%s", (int)cmd.end.status, (cmd.end.replacement != NULL? cmd.end.replacement : "NULL"));
		break;
	}
	ac_printf("}");
}


void CAnalysisManager__debugPrint(const CAnalysisManager* p_this, const char* file, int line) {
	const CAnalysisManager_Super* self = (const CAnalysisManager_Super*)p_this;
	//char format[50];
	const char* filename = (!strrchr(file, '\\') ? file : strrchr(file, '\\') + 1);

	ac_printf("\n-----------------------------------------------------\n");
	ac_printf("-------debugPrint(%s, %d):\nAnalysisManager[%p], \n", filename, line, self);
	ac_printf("runningStatus=%d, posFuncLoopCountWithEos=%u \n", self->runningStatus, self->posFuncLoopCountWithEos);
	AnalysisCommand__debugPrint(self->nextCmd);
	CBUCKET_CONTROLLER_DEBUG_PRINT(self->bucketController);
	ac_printf("+");
	CANALYSIS_EXECUTOR_DEBUG_PRINT(self->analysisExecutor);
	ac_printf("-----------------------------------------------------\n\n");
}


AcBool CAnalysisManager_isEnd(const CAnalysisManager* p_this) {
	const CAnalysisManager_Super* self = (const CAnalysisManager_Super*)p_this;
	return (self->runningStatus == RunningStatus_End);
}


/**
@brief pos()�̌��ʂ�Ԃ��B
@param c [out]���ʂ�ݒ肷��B�G���[�̏ꍇ0���ݒ肳���B
@param pos [in]�ʒu�i0�`�j�B
@return �������ʁB
*/
static AanalysisPosStatus CAnalysisManager_searchPos(CAnalysisManager_Super* p_this, char *c, const size_t pos) {
	CAnalysisManager_Super* self = (CAnalysisManager_Super*)p_this;
	//
	const int ret = CBucketController_pos(self->bucketController, pos);
	if (ret == CBUCKET_CONTROLLER_ERR_POS) {
		*c = 0;
		if (CBucketController_isContainingEos(self->bucketController)) {
			return AC_POS_STATUS_EOS;
		}
		else {
			return AC_POS_STATUS_BUFFEREND;
		}
	}
	*c = ret;
	return AC_POS_STATUS_OK;
}

static void CAnalysisManager_start(CAnalysisManager_Super* p_this, apr_table_t* table) {
	CAnalysisManager_Super* self = (CAnalysisManager_Super*)p_this;
	//
	self->runningStatus = RunningStatus_Running;
	CAnalysisExecutor_startFunc(self->analysisExecutor, table);
}

/**
forward(�u���J�n�ʒu��1�o�C�g�i�߂�)�̏���������B
@return EOS��BUFFEREND�̏ꍇ�͉��ύς�brigade��Ԃ��B���Ȃ�1�o�C�g�i�߂�ꂽ�ꍇ��NULL��Ԃ��B
*/
static apr_bucket_brigade* CAnalysisManager_forward(CAnalysisManager_Super* p_this, const AnalysisCommand nextCmd) {
	CAnalysisManager_Super* self = (CAnalysisManager_Super*)p_this;
	//�u���J�n�ʒu��1�o�C�g�i�߂�
	if (!CBucketController_forward(self->bucketController)) {
		return CBucketController_exportModifiedBrigade(self->bucketController);
	}
	char retC;
	//�i�߂���̌��݂̈ʒu0�̕�����ǂݎ��B��ŃG���[�ɂȂ��Ă��Ȃ��̂ŃG���[�ɂȂ炸�ǂݎ���͂��B
	AanalysisPosStatus posStatus = CAnalysisManager_searchPos(self, &retC, 0);
	self->nextCmd = CAnalysisExecutor_forwardFunc(self->analysisExecutor, nextCmd, AC_FALSE, retC);
	
	return NULL;
}


/**
forward(�u���J�n�ʒu��N�o�C�g�i�߂�)�̏���������B
@return EOS��BUFFEREND�̏ꍇ�͉��ύς�brigade��Ԃ��B���Ȃ�N�o�C�g�i�߂�ꂽ�ꍇ��NULL��Ԃ��B
*/
static apr_bucket_brigade* CAnalysisManager_forward_n(CAnalysisManager_Super* p_this, const AnalysisCommand nextCmd){
	CAnalysisManager_Super* self = (CAnalysisManager_Super*)p_this;
	//
	apr_bucket_brigade* brigade = NULL;
	size_t len = nextCmd.forward_n.len;
	if(len < 1) len = 1;
	for(size_t i = len; i >= 1; --i){
		//1�o�C�g�i�߂�BEOS
		if(!CBucketController_forward(self->bucketController)){
			self->nextCmd.forward_n.len = i;
			return CBucketController_exportModifiedBrigade(self->bucketController);
		}
	}

	AnalysisCommand cmd = {AC_FORWARD};
	char retC;
	//�i�߂���̌��݂̈ʒu0�̕�����ǂݎ��B��ŃG���[�ɂȂ��Ă��Ȃ��̂ŃG���[�ɂȂ炸�ǂݎ���͂��B
	AanalysisPosStatus posStatus = CAnalysisManager_searchPos(self, &retC, 0);
	self->nextCmd = CAnalysisExecutor_forwardFunc(self->analysisExecutor, cmd, AC_FALSE, retC);

	return NULL;
}

/**
pos�i�w��ʒu�̕����擾�j�̏���������B
@return BUFFEREND, EOS�Ȃǂ̏ꍇ��AC_FALSE�B�������擾�ł����AC_TRUE��Ԃ��B
*/
static AcBool CAnalysisManager_pos(CAnalysisManager_Super* p_this, const AnalysisCommand nextCmd) {
	CAnalysisManager_Super* self = (CAnalysisManager_Super*)p_this;
	//
	char retC;
	AcBool isEos = AC_FALSE;
	AanalysisPosStatus posStatus = CAnalysisManager_searchPos(self, &retC, self->nextCmd.pos.pos);
	if(posStatus == AC_POS_STATUS_BUFFEREND){
		return AC_FALSE;
	}
	if(posStatus == AC_POS_STATUS_EOS){
		isEos = AC_TRUE;
	}

	//POS��Executor��͂̎��s
	self->nextCmd = CAnalysisExecutor_posFunc(self->analysisExecutor, nextCmd, retC, isEos);
	
	//�o�O�΍�(�������[�v�ɓ����Ă���\��������ꍇ�ɃG���[�o�͂���)
	if(isEos){
		++self->posFuncLoopCountWithEos;
		if(self->posFuncLoopCountWithEos >= 10 && self->posFuncLoopCountWithEos <= 100){
			ac_printf("Improve your derived CAnalysisExecutor class. Maybe posFunc() is infinite looping %u counts in CAnalysisExecutor_posFunc.\n", 
				self->posFuncLoopCountWithEos);
			//�f�o�b�O�v�����g������
			if(self->posFuncLoopCountWithEos == 100){
				CANALYSIS_MANAGER_DEBUG_PRINT((CAnalysisManager*)self);
			}
		}
	} else{
		self->posFuncLoopCountWithEos = 0;
	}

	return AC_TRUE;
}


static void CAnalysisManager_replace(CAnalysisManager_Super* p_this, const AnalysisCommand nextCmd) {
	CAnalysisManager_Super* self = (CAnalysisManager_Super*)p_this;
	//
	AcBool bl = CBucketController_execReplace(self->bucketController, nextCmd.replace.len, nextCmd.replace.replacement);
	self->nextCmd = CAnalysisExecutor_replaceFunc(self->analysisExecutor, nextCmd, bl);
}


/**
@param bb [out]�G�N�X�|�[�g�o�͐��brigade
@return ���͂�bb��NULL�łȂ���Βǉ����ꂽbb���Ԃ�BNULL�̂Ƃ��͐V����brigade�ɃG�N�X�|�[�g���ĕԂ��B
*/
static apr_bucket_brigade* CAnalysisManager_end(CAnalysisManager_Super* p_this, const AnalysisCommand nextCmd, apr_bucket_brigade* bb) {
	CAnalysisManager_Super* self = (CAnalysisManager_Super*)p_this;

	//Executor���I���������Ăяo��
	self->nextCmd = CAnalysisExecutor_endFunc(self->analysisExecutor, self->nextCmd);
	CBucketController_moveAllInputBucketToModifiedBrigade(self->bucketController);

	//END�ȊO�̓~�X�Ƃ݂Ȃ��ĉ������Ȃ�
	if (self->nextCmd.type != AC_END) {
		self->nextCmd.type = AC_END;
		self->nextCmd.end.replacement = NULL;
		self->nextCmd.end.status = -1;
	}else if (self->nextCmd.end.replacement != NULL) {
		//�u����������Ō���ɒǉ��B
		CBucketController_addStringToModifiedBrigade(self->bucketController, self->nextCmd.end.replacement);
	}

	//modifiedBrigade��End��n��
	if(bb != NULL){
		CBucketController_exportModifiedBrigadeToBrigade(self->bucketController, bb);
	} else{
		bb = CBucketController_exportModifiedBrigade(self->bucketController);
	}

	self->runningStatus = RunningStatus_End;
	return bb;
}

/**
@brief ���͂��ꂽbrigade�����������A�ʉ߂����鏈���B
*/
static apr_bucket_brigade* CAnalysisManager_PassThrough(CAnalysisManager_Super* p_this, apr_bucket_brigade* inputBrigade) {
	CAnalysisManager_Super* self = (CAnalysisManager_Super*)p_this; 
	//����bucket�����ׂĕύX��ۊǌɂɈړ�����
	if (!CBucketController_isInputEmpty(self->bucketController)) {
		CBucketController_moveAllInputBucketToModifiedBrigade(self->bucketController);
	}
	//�ύX�ۊǌɂ��璊�o����
	apr_bucket_brigade* brigade = CBucketController_exportModifiedBrigade(self->bucketController);
	if (inputBrigade != NULL) {
		moveAllBucketsToBrigadeWithoutMeta(brigade, inputBrigade);
	}

	return brigade;
}



apr_bucket_brigade* CAnalysisManager_run(CAnalysisManager* p_this, apr_bucket_brigade* inputBrigade) {
	CAnalysisManager_Super* self = (CAnalysisManager_Super*)p_this;
	//
	if (self->isAllPassThough) {
		return CAnalysisManager_PassThrough(self, inputBrigade);
	}
	if (inputBrigade != NULL) {
		CBucketController_addBrigade(self->bucketController, inputBrigade);
		//CANALYSIS_MANAGER_DEBUG_PRINT(p_this);
	}
	//
	AcBool bl = AC_FALSE;
	apr_bucket_brigade* brigade = NULL;
	AnalysisCommandType preType = AC_FORWARD;
	while (!CBucketController_isEndBuffer(self->bucketController)) {
		brigade = NULL;
		AnalysisCommandType cmdType = self->nextCmd.type;
		switch (cmdType) {
		case AC_FORWARD:
			brigade = CAnalysisManager_forward(self, self->nextCmd);
			break;
		case AC_FORWARD_N:
			brigade = CAnalysisManager_forward_n(self, self->nextCmd);
			break;
		case AC_POS:
			bl = CAnalysisManager_pos(self, self->nextCmd);
			if(!bl){
				brigade = CBucketController_exportModifiedBrigade(self->bucketController);
			}
			break;
		case AC_REPLACE:
			if(preType == AC_REPLACE){
				//�A����REPLACE�����s����悤�Ƃ����ꍇ�A�����ɃG���[���o������exit������
				ac_checkClass("err", "", "The AC_REPLACE was called multiple times in series at CAnalysisManager_run.", AC_TRUE);
			}
			CAnalysisManager_replace(self, self->nextCmd);
			break;
		case AC_PASS_THROUGH_ALL:
			self->isAllPassThough = AC_TRUE;
			brigade = CAnalysisManager_PassThrough(self, inputBrigade);
		}
		preType = cmdType;
		
		//�I���̏ꍇ
		if (CBucketController_isEOS(self->bucketController)) {
			brigade = CAnalysisManager_end(self, self->nextCmd, brigade);
		}
		if (brigade != NULL) return brigade;
	}
	return CBucketController_exportModifiedBrigade(self->bucketController);
}


static void CAnalysisManager_delete(void* p_this) {
	CAnalysisManager_Super* self = (CAnalysisManager_Super*)p_this;
	ac_checkClass(gClassName_AnalysisManager, self->publicMember.thisIsClass->name, "in BucketController_delete()", AC_FALSE);
	//
	AcClass_delete(self->bucketController);
	AcClass_delete(self->analysisExecutor);
	free(self);
}


CAnalysisManager* CAnalysisManager_new(CAnalysisExecutor* executor, apr_pool_t* pool, apr_bucket_alloc_t* ba, apr_table_t* table, apr_bucket_brigade* bb) {
	//
	if (executor == NULL) return NULL;
	CAnalysisManager_Super* self = (CAnalysisManager_Super*)malloc(sizeof(CAnalysisManager_Super));
	if (self == NULL) return NULL;
	self->publicMember.thisIsClass = AcCThisIsClass_new(gClassName_AnalysisManager, CAnalysisManager_delete);
	self->nextCmd.type = AC_FORWARD;
	self->analysisExecutor = executor;
	self->bucketController = CBucketController_new(pool, ba, bb);
	self->runningStatus = RunningStatus_NotStarted;
	self->isAllPassThough = AC_FALSE;
	self->posFuncLoopCountWithEos = 0;
	//Executor�Ȃǂ��J�n����
	CAnalysisManager_start(self, table);
	return (CAnalysisManager*)self;
}



