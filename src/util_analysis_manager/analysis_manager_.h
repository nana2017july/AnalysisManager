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

#ifndef __ANALYSIS_MANGER___H__
#define __ANALYSIS_MANGER___H__


#include "apr_pools.h"
#include "apr_hash.h"

#include "analysis_manager.h"



/**
private�B
Bucket��ۊǂ���A�e�N���X(private�܂�)
*/
typedef struct CBucketController_Super {
	CBucketController publicMember;
	//private:
	///���݂̕�����̒��̕����̈ʒu�B������Ԃ�-1�B
	int currentBucketStrPos;
	///���݂�bucket�B
	apr_bucket* currentBucket;
	///���݂�bucket�̒��̕�����̒���
	size_t currentBucketStrLen;
	///���݂�bucket�̒��̕�����̌��݂̈ʒu
	const char* currentBucketStr;
	///�����ɕۊǂ�������bucket�̍Ō�ɒB�������H�iinputBrigade���󂾂�����EOS�������肵�Ă��邩�H�j
	AcBool isEndBuffer;
	///inputBrigade��EOS���܂܂�邩�H
	AcBool isContainingEos;
	//�������֘A------------------------
	apr_pool_t* pool;
	apr_bucket_alloc_t* bucketAlloc;
	///�ҏW���bucket��ۊǂ��Ă���brigade
	apr_bucket_brigade* modifiedBrigate;
	///���͂�bucket��ۊǂ��Ă���brigade(���̒��̐擪��bucket�͂��ł��J�����g��bucket)
	apr_bucket_brigade* inputBrigade;
} CBucketController_Super;



//------------------------------------------------------------------
 



//------------------------------------------------------------------
/**
private�B
CAnalysisExecutor�̐e�N���X(private�̒�`�̒ǉ�)
*/
typedef struct CAnalysisExecutor_Super {
	CAnalysisExecutor publicMember;
	//private:
	AcDeleteFunc_T subclassDeleteFunc;
	AcBool (*startFunc)(CAnalysisExecutor* p_this, const apr_table_t* table);
	const AnalysisCommand(*forwardFunc)(CAnalysisExecutor* p_this, const AnalysisCommand cmd, const AcBool isRejectCmd, const char c);
	const AnalysisCommand(*replaceFunc)(CAnalysisExecutor* p_this, const AnalysisCommand cmd, AcBool result);
	const AnalysisCommand(*posFunc)(CAnalysisExecutor* p_this, const AnalysisCommand cmd, const char c, const AcBool isEos);
	const AnalysisCommand(*endFunc)(CAnalysisExecutor* p_this, const AnalysisCommand cmd);
	//
	void (*debugPrintFunc)(const CAnalysisExecutor* p_this);
} CAnalysisExecutor_Super;





//------------------------------------------------------------------
/**
private�B
BucketManager�̐e�N���X(private�̒�`�̒ǉ�)
*/
typedef struct CAnalysisManager_Super {
	CAnalysisManager publicMember;
	//private:
	AnalysisCommand nextCmd;
	CAnalysisExecutor* analysisExecutor;
	CBucketController* bucketController;
	///0:not started, 1:runnig, 2:end
	int runningStatus;
	AcBool isAllPassThough;
	///�o�O�΍��p
	size_t posFuncLoopCountWithEos;
} CAnalysisManager_Super;






#endif  //end __BUCKET_CONTROLLER___H__

