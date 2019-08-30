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

#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
//関数として使うためにマクロを削除
#undef isspace
#undef tolower
//
#include "ac_bucket_utils.h"
#include "ac_this_is_class.h"
//
#include "apr.h"
#include "apr_general.h"
#include "apr_pools.h"
#include "apr_buckets.h"
//関数宣言
void sctest_printfFunc(const char* msg, va_list args);

//グローバルな変数
apr_pool_t* sctest_global_pool = NULL;
static void (*sctest_printFunc)(const char* msg, va_list args) = sctest_printfFunc;


/**
apr_initialize()を呼び出したり、テストutilsの初期化に必要な作業をする。
*/
void sctest_initialize() {
	if (apr_initialize() != APR_SUCCESS) {
		abort();
	}
	//apr_terminateはapr_initialize()を読んだときに1度呼ばないといけないらしい。
	atexit(apr_terminate);

	apr_pool_create(&sctest_global_pool, NULL);
	apr_pool_tag(sctest_global_pool, "apr-util global test pool");
}


void sctest_destoroy() {
	apr_pool_destroy(sctest_global_pool);
	apr_terminate();
}

/**
ブリゲードを作成する。
文字列の数が10000を超えたら異常終了する。
最後は必ずSCTEST_BT_EOSを設定すること。
@code
const char* src[] = {
	"abc","def",NULL,
	"gh",SCTEST_BT_FLUSH,"ijk",NULL,
	SCTEST_BT_EOS
}
@endcode
*/
apr_bucket_brigade** sctest_createBucketBrigade(
	apr_pool_t* pool, apr_bucket_alloc_t* ba, const char* src[])
{
	if (src == NULL) return NULL;
	int size = 1;
	const char** p;
	//サイズの計算 
	for (p = src; *p != SCTEST_BT_EOS; ++p) {
		++size;
		//領域サイズが大きすぎたら異常終了 
		if (size > 10000) {
			ac_printf("error in sctest_createBucketBrigade(). src=%p\n", src);
			abort();
		}
	}
	apr_bucket_brigade** ret = (apr_bucket_brigade * *)apr_palloc(
		pool, sizeof(apr_bucket_brigade*) * size);
	if (ret == NULL) {
		fprintf(stderr, "apr_palloc() returns NULL. perhaps out of memory!!!");
		exit(1);
	}

	//
	apr_bucket_brigade** curbb = ret;
	apr_bucket* tmpb = NULL;
	*curbb = apr_brigade_create(pool, ba);
	for (p = src; *p != SCTEST_BT_EOS; ++p) {
		//ブリゲード、バケットを作っていく 
		if (*p == SCTEST_BT_FLUSH) {
			//ac_printf("FLUSH!!\n");
			tmpb = apr_bucket_flush_create(ba);
			APR_BRIGADE_INSERT_TAIL(*curbb, tmpb);
		}
		else if (*p == NULL) {
			//ac_printf("Bucket end.\n");
			++curbb;
			*curbb = apr_brigade_create(pool, ba);
		}
		else {
			//ac_printf("%s,", *p);
			tmpb = apr_bucket_transient_create(*p, strlen(*p), ba);
			APR_BRIGADE_INSERT_TAIL(*curbb, tmpb);
		}
	}

	//SCTEST_BT_EOSの処理 
	//ac_printf("EOS!!!!!\n");
	tmpb = apr_bucket_eos_create(ba);
	APR_BRIGADE_INSERT_TAIL(*curbb, tmpb);
	++curbb;
	*curbb = NULL;
	return ret;
}

/**
データを含むタイプの文字列を出力する。
METAデータに対しては呼び出さないこと。
*/
static void _ac_printBucketString(const char* typeStr, apr_bucket* b){
	//先頭のbucketの文字列を読み込み
	const char* bucketStr = NULL;
	apr_size_t bucketStrLen = 0;
	apr_status_t status = apr_bucket_read(b, &bucketStr, &bucketStrLen, APR_BLOCK_READ);
	char format[20];
	sprintf(format, "%%s %%.%ds\n", (int)bucketStrLen);
	ac_printf(format, typeStr, bucketStr);
}


void ac_printBrigade(apr_bucket_brigade* bb) {
	for (apr_bucket* b = APR_BRIGADE_FIRST(bb); 
		b != APR_BRIGADE_SENTINEL(bb);
		b = APR_BUCKET_NEXT(b))
	{
		if (APR_BUCKET_IS_EOS(b)) {
			ac_printf("%s\n", "[EOS]");
			break;
		} else if (APR_BUCKET_IS_FILE(b)) {
			ac_printf("%s\n", "[FILE]");
		} else if (APR_BUCKET_IS_FLUSH(b)) {
			ac_printf("%s\n", "[FLUSH]");
		} else if(APR_BUCKET_IS_HEAP(b)){
			_ac_printBucketString("[HEAP]", b);
		} else if(APR_BUCKET_IS_IMMORTAL(b)){
			_ac_printBucketString("[IMMORTAL]", b);
		} else if(APR_BUCKET_IS_MMAP(b)){
			ac_printf("%s\n", "[MMAP]");
		} else if(APR_BUCKET_IS_PIPE(b)){
			_ac_printBucketString("[PIPE]", b);
		} else if(APR_BUCKET_IS_POOL(b)){
			_ac_printBucketString( "[POOL]", b);
		} else if(APR_BUCKET_IS_SOCKET(b)){
			_ac_printBucketString("[SOCKET]", b);
		} else if (APR_BUCKET_IS_TRANSIENT(b)){
			//先頭のbucketの文字列を出力
			_ac_printBucketString("[TRANSIENT]", b);
			/*const char* bucketStr = NULL;
			apr_size_t bucketStrLen = 0;
			apr_status_t status = apr_bucket_read(b, &bucketStr, &bucketStrLen, APR_BLOCK_READ);
			char format[10];
			sprintf(format, "%%.%ds\n", (int)bucketStrLen);
			ac_printf(format, bucketStr);*/
		} else if(APR_BUCKET_IS_METADATA(b)){
			ac_printf("%s\n", "[METADATA]");
		}else {
			ac_printf("%s\n", "[OTHER]");
		}
	}
}


void sctest_destroy_brigades(apr_bucket_brigade** bb) {
	apr_bucket_brigade** p;
	for (p = bb; *p != NULL; ++p) {
		apr_brigade_destroy(*p);
	}
}


//--------
static const char* btEosStr = "[EOS]";
static const char* btFlushStr = "[FLUSH]";
static const char* btFileStr = "[FILE]";
static const char* btTransientStr = "[TRANSIENT]";
static const char* btOtherStr = "[OTHER]";


const char* sctest_getBucketType(apr_bucket* b) {
	if (APR_BUCKET_IS_EOS(b)) {
		return btEosStr;
	}
	else if (APR_BUCKET_IS_FILE(b)) {
		return btFlushStr;
	}
	else if (APR_BUCKET_IS_FLUSH(b)) {
		return btFileStr;
	}
	else if (APR_BUCKET_IS_TRANSIENT(b)) {
		return btTransientStr;
	}
	else {
		return btOtherStr;
	}
}


/**
@return 一致しないときAC_FALSE
*/
int sctest_printNotEquals(const char** expectedStrs, apr_bucket_brigade* bb) {
	const char** p = expectedStrs;
	int actualBucketsNum = 0;
	apr_bucket* b = NULL;
	for (b = APR_BRIGADE_FIRST(bb); 
		*p != NULL && b!= APR_BRIGADE_SENTINEL(bb); 
		++p, b = APR_BUCKET_NEXT(b)) 
	{
		++actualBucketsNum;
		const char* bucketType = sctest_getBucketType(b);
		//ブリゲード、バケットを作っていく 
		if (*p == SCTEST_BT_FLUSH) {
			if (bucketType != btFlushStr) {
				ac_printf("expected [FLUSH] but %s\n", bucketType);
				return 0;
			}
		}
		else if (*p == SCTEST_BT_EOS) {
			if (bucketType != btEosStr) {
				ac_printf("expected [EOS] but %s\n", bucketType);
				return 0;
			}
		}
		else if (*p == SCTEST_BT_FILE) {
			if (bucketType != btFileStr) {
				ac_printf("expected [FILE] but %s\n", bucketType);
				return 0;
			}
		}
		else {//文字列
			const char* bucketStr = NULL;
			apr_size_t bucketStrLen = 0;
			apr_status_t status = apr_bucket_read(b, &bucketStr, &bucketStrLen, APR_BLOCK_READ);
			//
			char format[50];
			sprintf(format, "expected '%%s' but '%%.%ds'\n", (int)bucketStrLen);
			if (strlen(*p) != bucketStrLen) {
				ac_printf(format, *p, bucketStr);
				return 0;
			}
			if (strncmp(*p, bucketStr, bucketStrLen) != 0) {
				ac_printf(format, *p, bucketStr);
				return 0;
			}
		}
	}
	//bucketの数が違った場合
	if (*p != NULL || b != APR_BRIGADE_SENTINEL(bb)) {
		ac_printf("buckets num is different. checked buckets num=%d\n", actualBucketsNum);
		return 0;
	}

	return 1;
}



char* ac_copyToLowerCase(const char* str){
	char* ret = (char*)malloc(strlen(str) + 1);
	if(ret == NULL) return NULL;
	//小文字にして保管
	char* p = ret;
	for(; *str != '\0'; ++p, ++str) *p = tolower(*str);
	*p = '\0';
	return ret;
}


void ac_toLowerCase(char* str){
	//小文字に変換
	char* p = str;
	for(; *str != '\0'; ++p, ++str) *p = tolower(*str);
	*p = '\0';
}



void ac_unescapeChar(char* str, const char escape){
	//エスケープ文字を削除する
	char* p = str;
	for(; *str != '\0'; ++p, ++str){
		if(*str == escape) ++str;
		*p = *str;
	}
	*p = '\0';
}


/**
クラスを削除する関数。
Poolのcleanupに登録するためのもの。
*/
static apr_status_t poolCleanupClassFunc(void* c){
	void** classPtr = (void**)c;
	AcClass_delete(*classPtr);
	return APR_SUCCESS;
}

void ac_registClassToUserDataAndPoolCleanup(apr_pool_t* pool, const char* userDataKey, void* clazz){
	//
	void** classPtr = NULL;
	classPtr = (void*)apr_palloc(pool, sizeof(void*));
	*classPtr = clazz;

	//poolに登録
	apr_pool_cleanup_register(pool, classPtr, poolCleanupClassFunc, apr_pool_cleanup_null);
	//userdataに登録
	apr_pool_userdata_set(classPtr, userDataKey, apr_pool_cleanup_null, pool);
}

void* ac_getClassFromPoolUserData(const char* userDataKey, apr_pool_t* pool){
	void** classPtr = NULL;
	apr_pool_userdata_get((void**)& classPtr, userDataKey, pool);
	if(classPtr == NULL) return NULL;
	return *classPtr;
}


void sctest_printfFunc(const char* msg, va_list args){
	vprintf(msg, args);
}

void ac_printf(const char* msg, ...){
	va_list ap;
	va_start(ap, msg);
	sctest_printFunc(msg, ap);
}

int ac_setPrintFunc(void (*printFunc)(const char* msg, va_list args)){
	if(printFunc == NULL){
		printFunc = sctest_printfFunc;
	}
	sctest_printFunc = printFunc;
	return 1;
}


/*
RFCによるとHTTPヘッダは、
・ダブルクォートでくくられた中は1つのwordとして扱う。
・ダブルクォート内は\でエスケープでき、\+1文字　を1文字として扱う。
*/
char** ac_splitHttpHeaderValue(const char* str, const char* seps, AcBool isSplitEq, size_t maxSize){
	char** ret = ac_splitKeyValueArrayWithQuote(str, seps, isSplitEq, maxSize);
	ac_trimArray(ret);
	ac_removeQuoteArray(ret, '"', '\\');
	return ret;
}


char** ac_splitKeyValueArrayWithQuote(const char* str, const char* seps, AcBool isSplitEq, size_t maxSize){

	char** ret = (char**)malloc(sizeof(char*) * (maxSize * 2 + 1) + strlen(str) + 1);
	if(ret == NULL) return NULL;
	//
	char* copiedStr = ((char*)ret + sizeof(char*) * (maxSize * 2 + 1));
	strcpy(copiedStr, str);
	char* emptyStr = copiedStr + strlen(copiedStr);
	char** startPos = ret;
	*startPos = copiedStr;
	*(++startPos) = emptyStr;
	int cnt = 0;
	for(char* p = copiedStr; *p != '\0'; ++p){
		if(*p == '"'){
			++p;
			//対になる"を探す
			for(; *p != '\0'; ++p){
				if(*p == '\\'){
					if(++p == '\0') break;
				} else if(*p == '"'){
					break;
				}
			}
		} else if(strchr(seps, *p) != NULL){
			//区切り文字だった場合
			*p = '\0';
			//blank以外の場合だけ登録する
			if(*(p - 1) != '\0'){
				++cnt;
				if(cnt >= maxSize) break;
				*(++startPos) = p + 1;
				*(++startPos) = emptyStr;
			}
		}
	}

	//配列の最後が空文字なら無視する
	if(**(startPos - 1) == '\0') startPos -= 2;

	//配列の最後にNULLを設定
	*(++startPos) = NULL;

	//"="を区切るかどうか？
	if(!isSplitEq) return ret;

	//"="を区切る
	for(startPos = ret; *startPos != NULL; startPos += 2){
		for(char* a = *startPos; *a != '\0'; ++a){
			if(*a == '='){
				*a = '\0';
				*(startPos + 1) = a + 1;
				break;
			}
		}
	}
	return ret;
}


char** ac_split2(const char* str, const char eq, const char delimiter, size_t maxSize){
	char** ret = (char**)malloc(sizeof(char*) * (maxSize * 2 + 1) + strlen(str) + 1);
	if(ret == NULL) return NULL;
	//
	char* copiedStr = ((char*)ret + sizeof(char*) * (maxSize * 2 + 1));
	strcpy(copiedStr, str);
	char* emptyStr = copiedStr + strlen(copiedStr);
	char** startPos = ret;
	*startPos = copiedStr;
	*(++startPos) = emptyStr;
	int cnt = 0;
	for(char* p = copiedStr; *p != '\0'; ++p){
		if(*p == delimiter){
			++cnt;
			*p = '\0';
			if(cnt >= maxSize) break;
			*(++startPos) = p + 1;
			*(++startPos) = emptyStr;
		}
	}

	//配列の最後にNULLを設定
	*(++startPos) = NULL;

	//eqを区切る
	for(startPos = ret; *startPos != NULL; startPos += 2){
		for(char* a = *startPos; *a != '\0'; ++a){
			if(*a == eq){
				*a = '\0';
				*(startPos + 1) = a + 1;
				break;
			}
		}
	}
	return ret;
}


int ac_getIndexFromKeyValueArray(char** strKeyValueArray, const char* key){
	char** startPos;
	int i = 1;
	for(startPos = strKeyValueArray; *startPos != NULL; startPos += 2, i += 2){
		if(strcmp(*startPos, key) == 0){
			if(*(startPos + 1) == NULL) return -1;
			return i;
		}
	}
	return -1;
}


void ac_trimArray(char** strArray){
	char** startPos;
	for(startPos = strArray; *startPos != NULL; ++startPos){
		//前方のtrim
		for(char* a = *startPos; ; ++a){
			if(!isspace(*a)){
				*startPos = a;
				break;
			}
			if(*a == '\0') break;
		}
		//後方のtrim
		for(char* a = *startPos + strlen(*startPos); ; --a){
			if(!isspace(*a) && *a != '\0'){
				*(a+1) = '\0';
				break;
			}
			if(a == *startPos) break;
		}
	}
}




void ac_removeQuoteArray(char** strArray, char quote, char escape){
	char** startPos;
	for(startPos = strArray; *startPos != NULL; ++startPos){
		//前方のquote
		if(**startPos == quote){
			//quote削除
			*startPos = *startPos + 1;

			//エスケープの削除
			char* p = *startPos;
			char* str = *startPos;
			for(; *str != '\0'; ++p, ++str){
				if(*str == escape) ++str;
				*p = *str;
			}
			//後方のquote削除
			if(*(p - 1) == '"') --p;
			*p = '\0';
		}
		

		/*/後方のquote
		size_t len = strlen(*startPos);
		if(len == 0) continue;
		char* p = *startPos + len - 1;
		//最後尾の文字がquoteかどうか
		if(*p == quote){
			*p = '\0';
		}*/


	}
}

