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
private。
Bucketを保管する、親クラス(private含む)
*/
typedef struct CBucketController_Super {
	CBucketController publicMember;
	//private:
	///現在の文字列の中の文字の位置。初期状態は-1。
	int currentBucketStrPos;
	///現在のbucket。
	apr_bucket* currentBucket;
	///現在のbucketの中の文字列の長さ
	size_t currentBucketStrLen;
	///現在のbucketの中の文字列の現在の位置
	const char* currentBucketStr;
	///内部に保管した入力bucketの最後に達したか？（inputBrigadeが空だったりEOSだったりしているか？）
	AcBool isEndBuffer;
	///inputBrigadeにEOSが含まれるか？
	AcBool isContainingEos;
	//メモリ関連------------------------
	apr_pool_t* pool;
	apr_bucket_alloc_t* bucketAlloc;
	///編集後のbucketを保管しておくbrigade
	apr_bucket_brigade* modifiedBrigate;
	///入力のbucketを保管しておくbrigade(この中の先頭のbucketはいつでもカレントのbucket)
	apr_bucket_brigade* inputBrigade;
} CBucketController_Super;



//------------------------------------------------------------------
 



//------------------------------------------------------------------
/**
private。
CAnalysisExecutorの親クラス(privateの定義の追加)
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
private。
BucketManagerの親クラス(privateの定義の追加)
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
	///バグ対策用
	size_t posFuncLoopCountWithEos;
} CAnalysisManager_Super;






#endif  //end __BUCKET_CONTROLLER___H__

