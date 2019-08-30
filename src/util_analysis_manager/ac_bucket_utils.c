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
//�֐��Ƃ��Ďg�����߂Ƀ}�N�����폜
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
//�֐��錾
void sctest_printfFunc(const char* msg, va_list args);

//�O���[�o���ȕϐ�
apr_pool_t* sctest_global_pool = NULL;
static void (*sctest_printFunc)(const char* msg, va_list args) = sctest_printfFunc;


/**
apr_initialize()���Ăяo������A�e�X�gutils�̏������ɕK�v�ȍ�Ƃ�����B
*/
void sctest_initialize() {
	if (apr_initialize() != APR_SUCCESS) {
		abort();
	}
	//apr_terminate��apr_initialize()��ǂ񂾂Ƃ���1�x�Ă΂Ȃ��Ƃ����Ȃ��炵���B
	atexit(apr_terminate);

	apr_pool_create(&sctest_global_pool, NULL);
	apr_pool_tag(sctest_global_pool, "apr-util global test pool");
}


void sctest_destoroy() {
	apr_pool_destroy(sctest_global_pool);
	apr_terminate();
}

/**
�u���Q�[�h���쐬����B
������̐���10000�𒴂�����ُ�I������B
�Ō�͕K��SCTEST_BT_EOS��ݒ肷�邱�ƁB
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
	//�T�C�Y�̌v�Z 
	for (p = src; *p != SCTEST_BT_EOS; ++p) {
		++size;
		//�̈�T�C�Y���傫��������ُ�I�� 
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
		//�u���Q�[�h�A�o�P�b�g������Ă��� 
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

	//SCTEST_BT_EOS�̏��� 
	//ac_printf("EOS!!!!!\n");
	tmpb = apr_bucket_eos_create(ba);
	APR_BRIGADE_INSERT_TAIL(*curbb, tmpb);
	++curbb;
	*curbb = NULL;
	return ret;
}

/**
�f�[�^���܂ރ^�C�v�̕�������o�͂���B
META�f�[�^�ɑ΂��Ă͌Ăяo���Ȃ����ƁB
*/
static void _ac_printBucketString(const char* typeStr, apr_bucket* b){
	//�擪��bucket�̕������ǂݍ���
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
			//�擪��bucket�̕�������o��
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
@return ��v���Ȃ��Ƃ�AC_FALSE
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
		//�u���Q�[�h�A�o�P�b�g������Ă��� 
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
		else {//������
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
	//bucket�̐���������ꍇ
	if (*p != NULL || b != APR_BRIGADE_SENTINEL(bb)) {
		ac_printf("buckets num is different. checked buckets num=%d\n", actualBucketsNum);
		return 0;
	}

	return 1;
}



char* ac_copyToLowerCase(const char* str){
	char* ret = (char*)malloc(strlen(str) + 1);
	if(ret == NULL) return NULL;
	//�������ɂ��ĕۊ�
	char* p = ret;
	for(; *str != '\0'; ++p, ++str) *p = tolower(*str);
	*p = '\0';
	return ret;
}


void ac_toLowerCase(char* str){
	//�������ɕϊ�
	char* p = str;
	for(; *str != '\0'; ++p, ++str) *p = tolower(*str);
	*p = '\0';
}



void ac_unescapeChar(char* str, const char escape){
	//�G�X�P�[�v�������폜����
	char* p = str;
	for(; *str != '\0'; ++p, ++str){
		if(*str == escape) ++str;
		*p = *str;
	}
	*p = '\0';
}


/**
�N���X���폜����֐��B
Pool��cleanup�ɓo�^���邽�߂̂��́B
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

	//pool�ɓo�^
	apr_pool_cleanup_register(pool, classPtr, poolCleanupClassFunc, apr_pool_cleanup_null);
	//userdata�ɓo�^
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
RFC�ɂ���HTTP�w�b�_�́A
�E�_�u���N�H�[�g�ł�����ꂽ����1��word�Ƃ��Ĉ����B
�E�_�u���N�H�[�g����\�ŃG�X�P�[�v�ł��A\+1�����@��1�����Ƃ��Ĉ����B
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
			//�΂ɂȂ�"��T��
			for(; *p != '\0'; ++p){
				if(*p == '\\'){
					if(++p == '\0') break;
				} else if(*p == '"'){
					break;
				}
			}
		} else if(strchr(seps, *p) != NULL){
			//��؂蕶���������ꍇ
			*p = '\0';
			//blank�ȊO�̏ꍇ�����o�^����
			if(*(p - 1) != '\0'){
				++cnt;
				if(cnt >= maxSize) break;
				*(++startPos) = p + 1;
				*(++startPos) = emptyStr;
			}
		}
	}

	//�z��̍Ōオ�󕶎��Ȃ疳������
	if(**(startPos - 1) == '\0') startPos -= 2;

	//�z��̍Ō��NULL��ݒ�
	*(++startPos) = NULL;

	//"="����؂邩�ǂ����H
	if(!isSplitEq) return ret;

	//"="����؂�
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

	//�z��̍Ō��NULL��ݒ�
	*(++startPos) = NULL;

	//eq����؂�
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
		//�O����trim
		for(char* a = *startPos; ; ++a){
			if(!isspace(*a)){
				*startPos = a;
				break;
			}
			if(*a == '\0') break;
		}
		//�����trim
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
		//�O����quote
		if(**startPos == quote){
			//quote�폜
			*startPos = *startPos + 1;

			//�G�X�P�[�v�̍폜
			char* p = *startPos;
			char* str = *startPos;
			for(; *str != '\0'; ++p, ++str){
				if(*str == escape) ++str;
				*p = *str;
			}
			//�����quote�폜
			if(*(p - 1) == '"') --p;
			*p = '\0';
		}
		

		/*/�����quote
		size_t len = strlen(*startPos);
		if(len == 0) continue;
		char* p = *startPos + len - 1;
		//�Ō���̕�����quote���ǂ���
		if(*p == quote){
			*p = '\0';
		}*/


	}
}

