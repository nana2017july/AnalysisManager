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
19.07.22 BucketController_initCurrentVar() の不具合修正。SENTINELのときにSegmentation faultする問題を修正。
</pre>
*/


static const char* gClassName_BucketController = "BucketCotroller";
static const char* gClassName_AnalysisManager = "AnalysisManger";

//定義
const int CBUCKET_CONTROLLER_ERR_POS = -10000;

//------------------------------------------------------
//----    BucketCotroller Class       ---------------------
//------------------------------------------------------
//------------------------------------------------------

/**public:
デバッグ用の関数
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
引数のbucketをカレントの値として、current*変数を初期化する。
※カレントの文字の位置は-1にするので、もしcurBucketの先頭文字の位置にしたい場合はこの関数呼び出し後に+1すること。
bucketがEOSの場合は内部のステータスをEOSの状態に書き変える。
@param curBucket [in]このbucketで初期化する
*/
static void BucketController_initCurrentVar(CBucketController_Super* p_this, apr_bucket* curBucket) {
	CBucketController_Super* self = (CBucketController_Super*)p_this;
	//先頭のbucketの文字列を読み込み
	const char* bucketStr = NULL;
	apr_size_t bucketStrLen = 0;
	if (!APR_BUCKET_IS_METADATA(curBucket) && curBucket != APR_BRIGADE_SENTINEL(self->inputBrigade)) {
		apr_status_t status = apr_bucket_read(curBucket, &bucketStr, &bucketStrLen, APR_BLOCK_READ);
	}
	else if (APR_BUCKET_IS_EOS(curBucket)) {
		self->isEndBuffer = AC_TRUE;
	}
	//設定
	self->currentBucket = curBucket;
	self->currentBucketStr = bucketStr;
	self->currentBucketStrLen = bucketStrLen;
	self->currentBucketStrPos = -1;
}


/**
指定のbucketをbrigadeから切り離して、指定のbrigadeの最後尾に移動するだけのメソッド。
@param bb [out]bucketを最後尾に追加したbrigade。
@param bucket [in]どこかのbrigadeに所属しているbucket。
*/
static void moveBucketToBrigade(apr_bucket_brigade* bb, apr_bucket* bucket) {
	//元のbrigadeからbucketを切り離す（紐づけを無くす）。
	APR_BUCKET_REMOVE(bucket);
	//内部のbrigadeの最後尾に追加する
	APR_BRIGADE_INSERT_TAIL(bb, bucket);
}

/**
brigadeの中のbucketを切り離して、指定のbrigadeの最後尾に全て追加する。
@param in [out]srcのbucketが追加されたbrigadeが返る
@param src [in]追加対象のbucketを含むbrigade。
@n         [out]空になったbrigade。
*/
static void moveBrigadeToBrigade(apr_bucket_brigade* in, apr_bucket_brigade* src){
	while(!APR_BRIGADE_EMPTY(src)){
		apr_bucket* b = APR_BRIGADE_FIRST(src);
		//移動する
		moveBucketToBrigade(in, b);
	}
}

/**
指定のbucketをbrigadeから切り離して、指定のbrigadeの最後尾に追加する。
ただし、EOS以外のMETAのbucketは読み捨てる。
@param bb [out]META以外のbucketをすべて追加したbrigade。
@param in [in]移動元。関数呼び出し後は、空のbrigadeになる。
*/
static void moveAllBucketsToBrigadeWithoutMeta(apr_bucket_brigade* bb, apr_bucket_brigade* in) {
	apr_bucket* b = NULL;
	while (!APR_BRIGADE_EMPTY(in)) {
		b = APR_BRIGADE_FIRST(in);
		//移動する
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
検索結果を保存する構造体。
*/
typedef struct _BucketSearchResult {
	///見つからなかった場合はNULL
	apr_bucket* bucket;
	///bucketの中の位置。
	size_t bucketStrPos;
} _BucketSearchResult;

/**
brigadeの先頭bucketの指定の位置を起点として、指定の位置のbucketを探す。
指定の位置のbucketの中のどの位置が指定の位置かも返す。
@param startStrPos [in]開始位置（0～）。brigadeの最初のbucketの中の文字列の位置を指定。指定位置が最初のbucketを超える場合は見つからないエラーを返す。
@param posFromStart [in]開始位置からの位置（0～）。例えば開始位置がstr[1]の場合で、0を指定するとstr[1]の位置が返る。
@return 見つかった位置のbucketのポインタとその中の文字列の位置を返す。見つからなかった場合、bucketStrPos=-1が設定される。
*/
static const _BucketSearchResult searchBucketPos(apr_bucket_brigade* brigade, size_t startStrPos, size_t posFromStart) {
	_BucketSearchResult result = {NULL, 0};
	apr_bucket* bucket = APR_BRIGADE_FIRST(brigade);
	const char* bucketStr = NULL;
	apr_size_t bucketStrLen = 0; 
	apr_status_t status = apr_bucket_read(bucket, &bucketStr, &bucketStrLen, APR_BLOCK_READ);
	//最初のbucketの文字列長を指定の長さが超えている場合はエラー
	if (bucketStrLen < startStrPos) return result;
	//最初のbucketで指定の長さを満たす場合
	size_t len = bucketStrLen - startStrPos;
	if (posFromStart < len) {
		result.bucket = bucket;
		result.bucketStrPos = startStrPos + posFromStart;
		return result;
	}
	//目的の位置を探す
	for (bucket = APR_BUCKET_NEXT(bucket); bucket != APR_BRIGADE_SENTINEL(brigade); bucket = APR_BUCKET_NEXT(bucket)) {
		apr_status_t status = apr_bucket_read(bucket, &bucketStr, &bucketStrLen, APR_BLOCK_READ);
		len += (size_t)bucketStrLen;
		//このbucketに目的の位置がある場合
		if (posFromStart < len) {
			result.bucket = bucket;
			result.bucketStrPos = bucketStrLen - (len - posFromStart);
			return result;
		}
	}
	//brigadeの最後まで達した。
	return result;
}


/** private:
指定のinputBucketをbrigadeから切り離して、modifiedBrigadeの最後尾に追加する。
※文字列のカレント位置は-1にセットされる。
@param inputBucket [in]brigadeに挿入するbucket。self->inputBrigadeのbucketを入力にする。
*/
static void BucketController_moveInputBucketToModifiedBrigade(CBucketController_Super* p_this, apr_bucket* inputBucket) {
	CBucketController_Super* self = (CBucketController_Super*)p_this;
	//
	moveBucketToBrigade(self->modifiedBrigate, inputBucket);
	//初期化する
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
	//brigadeの終わりに来ている場合。
	if (CBucketController_isEndBuffer((const CBucketController*)self)) return CBUCKET_CONTROLLER_ERR_POS;
	//開始していない場合
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
		//このbucketに目的の位置がある場合
		if (pos < len + bucketStrLen) {
			return bucketStr[pos - len];
		}
		len += (size_t)bucketStrLen;
	}
	//目的の位置が見つかる前にbucketがなくなった
	return CBUCKET_CONTROLLER_ERR_POS;
}


AcBool CBucketController_forward(CBucketController* p_this) {
	CBucketController_Super* self = (CBucketController_Super*)p_this;
	//brigadeの終わりに来ている場合。
	if (CBucketController_isEndBuffer((CBucketController*)self)) return AC_FALSE;
	//
	++self->currentBucketStrPos;
	//カレントのbucketの中で移動ができた場合
	if (self->currentBucketStrPos < self->currentBucketStrLen) {
		return AC_TRUE;
	}

	//次のbucketに移動
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
	//一度もforwardが呼ばれてない場合
	if (self->currentBucketStrPos == -1) return AC_FALSE;
	//inputBrigadeが空などバッファの終わりの場合
	if (CBucketController_isEndBuffer((CBucketController*)self)) return AC_FALSE;
	apr_bucket* b = self->currentBucket;
	//置換開始位置のsplit(カレントの先頭の場合は分割不要のため何もしない)
	if (self->currentBucketStrPos != 0) {
		apr_bucket_split(b, self->currentBucketStrPos);
		//置換対象じゃない文字列を最後尾に追加
		BucketController_moveInputBucketToModifiedBrigade(self, APR_BRIGADE_FIRST(self->inputBrigade));
	}

	//長さ0の置換の場合
	if (len == 0) {
		//置換文字列を最後尾に追加。
		apr_bucket* tmpb = apr_bucket_transient_create(replaceStr, strlen(replaceStr), self->bucketAlloc);
		APR_BRIGADE_INSERT_TAIL(self->modifiedBrigate, tmpb);
		//bucket位置や文字列の位置等の初期化
		BucketController_initCurrentVar(self, APR_BRIGADE_FIRST(self->inputBrigade));
		return AC_TRUE;
	}
	//置換終了位置を検索する
	const _BucketSearchResult result = searchBucketPos(self->inputBrigade, 0, len - 1);
	//位置が見つからなかった場合（バッファのエンドに達した場合など）
	if (result.bucket == NULL) {
		//bucket位置や文字列の位置等の初期化
		BucketController_initCurrentVar(self, APR_BRIGADE_FIRST(self->inputBrigade));
		return AC_FALSE;
	}

	//置換文字列を最後尾に追加。
	apr_bucket* tmpb = apr_bucket_transient_create(replaceStr, strlen(replaceStr), self->bucketAlloc);
	APR_BRIGADE_INSERT_TAIL(self->modifiedBrigate, tmpb);

	//置換対象bucketを削除する
	while ((b = APR_BRIGADE_FIRST(self->inputBrigade)) != result.bucket) {
		//先頭のbucketを削除する
		apr_bucket_delete(b);
	}
	//inputBrigadeの先頭のbucketを分割する
	apr_bucket_split(b, result.bucketStrPos + 1);
	//分割したbucketを削除する（置換対象）
	apr_bucket_delete(b);
	//bucket位置や文字列の位置等の初期化
	BucketController_initCurrentVar(self, APR_BRIGADE_FIRST(self->inputBrigade));
	//先頭のbucketが0文字の場合、削除する
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
	
	//追加位置のbucketを取得
	apr_bucket* b = APR_BRIGADE_LAST(self->modifiedBrigate);
	//最後のbucketがEOSの場合、その1つ前を指定する
	if (APR_BUCKET_IS_EOS(b)) {
		b = APR_BUCKET_PREV(b);
	}

	//文字列を作成。
	apr_bucket* tmpb = apr_bucket_transient_create(replaceStr, strlen(replaceStr), self->bucketAlloc);
	//bucketの後に追加
	APR_BUCKET_INSERT_AFTER(b, tmpb);
}




void CBucketController_addBrigade(CBucketController* p_this, apr_bucket_brigade* bb) {
	CBucketController_Super* self = (CBucketController_Super*)p_this;
	//
	apr_bucket* currentBucket = self->currentBucket;

	//入力brigadeにすべて移動する
	moveAllBucketsToBrigadeWithoutMeta(self->inputBrigade, bb);
	if (APR_BUCKET_IS_EOS(APR_BRIGADE_LAST(self->inputBrigade))) {
		self->isContainingEos = AC_TRUE;
	}

	//最後尾の次のbucketの場合
	if (currentBucket == APR_BRIGADE_SENTINEL(self->inputBrigade)) {
		//カレントbucketを先頭にする
		BucketController_initCurrentVar(self, APR_BRIGADE_FIRST(self->inputBrigade));
	}

	//get()で次の文字に移動できるように
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
	//新たなbrigade領域を作成
	self->modifiedBrigate = apr_brigade_create(self->pool, self->bucketAlloc);
	return bb;
}


void CBucketController_exportModifiedBrigadeToBrigade(CBucketController* p_this, apr_bucket_brigade* outbb){
	CBucketController_Super* self = (CBucketController_Super*)p_this;
	//移動する
	apr_bucket_brigade* in = CBucketController_exportModifiedBrigade((CBucketController*)self);
	moveBrigadeToBrigade(outbb, in);

	//破壊する
	apr_brigade_destroy(in);
}


/**private:
デストラクタ
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
	//bucket_allocは1つのスレッドに1つしか作ってはいけないらしい
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
	//入力のbucket_brigadeを設定
	self->inputBrigade = apr_brigade_create(pool, ba);
	moveAllBucketsToBrigadeWithoutMeta(self->inputBrigade, bb);
	//
	self->isContainingEos = AC_FALSE;
	if (APR_BUCKET_IS_EOS(APR_BRIGADE_LAST(self->inputBrigade))) {
		self->isContainingEos = AC_TRUE;
	}
	BucketController_initCurrentVar(self, APR_BRIGADE_FIRST(self->inputBrigade));
	self->isEndBuffer = AC_FALSE;

	//修正かけたbrigade関連の情報を設定
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
	//あとは派生クラスに任せる
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
	//selfは派生クラスでfreeする。生成したクラスがfreeするルール。
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
@brief pos()の結果を返す。
@param c [out]結果を設定する。エラーの場合0が設定される。
@param pos [in]位置（0～）。
@return 検索結果。
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
forward(置換開始位置を1バイト進める)の処理をする。
@return EOSやBUFFERENDの場合は改変済みbrigadeを返す。問題なく1バイト進められた場合はNULLを返す。
*/
static apr_bucket_brigade* CAnalysisManager_forward(CAnalysisManager_Super* p_this, const AnalysisCommand nextCmd) {
	CAnalysisManager_Super* self = (CAnalysisManager_Super*)p_this;
	//置換開始位置を1バイト進める
	if (!CBucketController_forward(self->bucketController)) {
		return CBucketController_exportModifiedBrigade(self->bucketController);
	}
	char retC;
	//進めた後の現在の位置0の文字を読み取る。上でエラーになっていないのでエラーにならず読み取れるはず。
	AanalysisPosStatus posStatus = CAnalysisManager_searchPos(self, &retC, 0);
	self->nextCmd = CAnalysisExecutor_forwardFunc(self->analysisExecutor, nextCmd, AC_FALSE, retC);
	
	return NULL;
}


/**
forward(置換開始位置をNバイト進める)の処理をする。
@return EOSやBUFFERENDの場合は改変済みbrigadeを返す。問題なくNバイト進められた場合はNULLを返す。
*/
static apr_bucket_brigade* CAnalysisManager_forward_n(CAnalysisManager_Super* p_this, const AnalysisCommand nextCmd){
	CAnalysisManager_Super* self = (CAnalysisManager_Super*)p_this;
	//
	apr_bucket_brigade* brigade = NULL;
	size_t len = nextCmd.forward_n.len;
	if(len < 1) len = 1;
	for(size_t i = len; i >= 1; --i){
		//1バイト進める。EOS
		if(!CBucketController_forward(self->bucketController)){
			self->nextCmd.forward_n.len = i;
			return CBucketController_exportModifiedBrigade(self->bucketController);
		}
	}

	AnalysisCommand cmd = {AC_FORWARD};
	char retC;
	//進めた後の現在の位置0の文字を読み取る。上でエラーになっていないのでエラーにならず読み取れるはず。
	AanalysisPosStatus posStatus = CAnalysisManager_searchPos(self, &retC, 0);
	self->nextCmd = CAnalysisExecutor_forwardFunc(self->analysisExecutor, cmd, AC_FALSE, retC);

	return NULL;
}

/**
pos（指定位置の文字取得）の処理をする。
@return BUFFEREND, EOSなどの場合はAC_FALSE。文字を取得できればAC_TRUEを返す。
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

	//POSのExecutor解析の実行
	self->nextCmd = CAnalysisExecutor_posFunc(self->analysisExecutor, nextCmd, retC, isEos);
	
	//バグ対策(無限ループに入っている可能性がある場合にエラー出力する)
	if(isEos){
		++self->posFuncLoopCountWithEos;
		if(self->posFuncLoopCountWithEos >= 10 && self->posFuncLoopCountWithEos <= 100){
			ac_printf("Improve your derived CAnalysisExecutor class. Maybe posFunc() is infinite looping %u counts in CAnalysisExecutor_posFunc.\n", 
				self->posFuncLoopCountWithEos);
			//デバッグプリントもする
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
@param bb [out]エクスポート出力先のbrigade
@return 入力のbbがNULLでなければ追加されたbbが返る。NULLのときは新たなbrigadeにエクスポートして返す。
*/
static apr_bucket_brigade* CAnalysisManager_end(CAnalysisManager_Super* p_this, const AnalysisCommand nextCmd, apr_bucket_brigade* bb) {
	CAnalysisManager_Super* self = (CAnalysisManager_Super*)p_this;

	//Executorを終了処理を呼び出す
	self->nextCmd = CAnalysisExecutor_endFunc(self->analysisExecutor, self->nextCmd);
	CBucketController_moveAllInputBucketToModifiedBrigade(self->bucketController);

	//END以外はミスとみなして何もしない
	if (self->nextCmd.type != AC_END) {
		self->nextCmd.type = AC_END;
		self->nextCmd.end.replacement = NULL;
		self->nextCmd.end.status = -1;
	}else if (self->nextCmd.end.replacement != NULL) {
		//置換文字列を最後尾に追加。
		CBucketController_addStringToModifiedBrigade(self->bucketController, self->nextCmd.end.replacement);
	}

	//modifiedBrigadeにEndを渡す
	if(bb != NULL){
		CBucketController_exportModifiedBrigadeToBrigade(self->bucketController, bb);
	} else{
		bb = CBucketController_exportModifiedBrigade(self->bucketController);
	}

	self->runningStatus = RunningStatus_End;
	return bb;
}

/**
@brief 入力されたbrigadeを何もせず、通過させる処理。
*/
static apr_bucket_brigade* CAnalysisManager_PassThrough(CAnalysisManager_Super* p_this, apr_bucket_brigade* inputBrigade) {
	CAnalysisManager_Super* self = (CAnalysisManager_Super*)p_this; 
	//入力bucketをすべて変更後保管庫に移動する
	if (!CBucketController_isInputEmpty(self->bucketController)) {
		CBucketController_moveAllInputBucketToModifiedBrigade(self->bucketController);
	}
	//変更保管庫から抽出する
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
				//連続でREPLACEが実行されようとした場合、強引にエラーを出させてexitさせる
				ac_checkClass("err", "", "The AC_REPLACE was called multiple times in series at CAnalysisManager_run.", AC_TRUE);
			}
			CAnalysisManager_replace(self, self->nextCmd);
			break;
		case AC_PASS_THROUGH_ALL:
			self->isAllPassThough = AC_TRUE;
			brigade = CAnalysisManager_PassThrough(self, inputBrigade);
		}
		preType = cmdType;
		
		//終了の場合
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
	//Executorなどを開始する
	CAnalysisManager_start(self, table);
	return (CAnalysisManager*)self;
}



