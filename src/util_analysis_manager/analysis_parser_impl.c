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
//関数として使うためにマクロを削除
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
	///文字をストックしておくバッファ
	char* stockStr;
	size_t stockStrBufferSize;
	///次にバッファに文字を書き込む位置
	char* stockStrPos;
} CAnalysisParser_Split_Super;


/**
stockStrの実際の文字列長を取得する。
*/
static size_t CAnalysisParser_Split_getStockStrLen(CAnalysisParser_Split_Super* p_this){
	CAnalysisParser_Split_Super* self = (CAnalysisParser_Split_Super*)p_this;
	return (size_t)(self->stockStrPos - self->stockStr);
}

/**
stockStrの実際の文字列長を取得する。
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
デバッグ用の関数
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

	//ストックのバッファサイズを超えない場合のみストック文字列をセットする
	if(CAnalysisParser_Split_getStockStrLen(self) + 1 <= self->stockStrBufferSize){
		//ストック文字列をセット
		*self->stockStrPos = c;
		++self->stockStrPos;
	}

	//チェックする
	if(self->parentMember.status == AnalysisParserStatus_Searching){
		//開始文字と違った場合、次の文字へ。
		if(c != *self->delimiterStrPos) return;
		//開始文字と一致した場合、ステータスを変える
		self->parentMember.status = AnalysisParserStatus_Matching;
	} else if(self->parentMember.status == AnalysisParserStatus_Matching){
		//文字列とマッチしなかった場合、ステータスをリセットして次へ。
		if(c != *self->delimiterStrPos){
			CAnalysisParser_Split_resetFunc((CAnalysisParser*)self);
			return;
		}
	}

	//マッチ対象の次の文字へ進めておく
	++self->delimiterStrPos;
	//文字列全てマッチした場合、ステータスを終了状態に変更
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
デバッグ用の関数
*/
static void CAnalysisParser_CaseInsensitive__debugPrint(const CAnalysisParser* p_this){
	const CAnalysisParser_CaseInsensitive_Super* self = (const CAnalysisParser_CaseInsensitive_Super*)p_this;

	ac_printf("targetStr=%s, targetStrPos=%s\n", self->targetStr, self->targetStrPos);
}


static void CAnalysisParser_CaseInsensitive_acceptFunc(CAnalysisParser* p_this, const char c){
	CAnalysisParser_CaseInsensitive_Super* self = (CAnalysisParser_CaseInsensitive_Super*)p_this;
	//
	if(self->parentMember.isEnd) return;
	//マッチ確認中
	if(self->parentMember.status == AnalysisParserStatus_Matching){
		//マッチしなかった場合、終了へ
		if(tolower(c) != *self->targetStrPos){
			self->parentMember.isEnd = AC_TRUE;
			return;
		}
	}

	//マッチした場合、次の位置へ
	++self->targetStrPos;
	//完全マッチした場合、ステータスを終了状態にする。
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
	///属性名がマッチしなかったのでヘッダの最後を探す
	HttpHeaderStatus_HeaderEndSearching,
	///ヘッダの最後の一部がマッチしたので最後を探す
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
デバッグ用の関数
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
	//マッチ確認中
	if(self->status == HttpHeaderStatus_Attribute){
		//属性名を検索する
		CAnalysisParser* parser = (CAnalysisParser*)self->attrNameParser;
		CAnalysisParser_accept(parser, c);
		if(CAnalysisParser_isEnd(parser)){
			if(CAnalysisParser_isMatched(parser)){
				//属性名が指定の名前だった場合、値をパースするステータスに変更する
				self->status = HttpHeaderStatus_Value;
			} else{
				//属性名が指定のものではなかった場合、行の最後を探す
				self->status = HttpHeaderStatus_HeaderEndSearching;
			}
		}
	} else if(self->status == HttpHeaderStatus_Value){
		//属性名がマッチしたので、値をコピーする
		CAnalysisParser* parser = (CAnalysisParser*)self->valueParser;
		CAnalysisParser_accept(parser, c);
		if(CAnalysisParser_isEnd(parser)){
			//パースを終わりにする
			self->parentMember.status = AnalysisParserStatus_Matched;
			self->parentMember.isEnd = AC_TRUE;
		}
	}
	
	//
	if(self->status == HttpHeaderStatus_HeaderEndSearching){
		//属性名がマッチしなかったので値は無視して、行の終わり(\r)を探す。
		if(c == '\r'){
			//\nを探す（この方法だと\rX\nのような文字列の場合もヒットしてしまうが悪意のある場合のみなので良しとする）
			self->status = HttpHeaderStatus_HeaderEndMatching;
		}
	} else if(self->status == HttpHeaderStatus_HeaderEndMatching){
		//行の終わり(\n)を探す。
		if(c == '\n'){
			//パースを終わりにする
			self->parentMember.isEnd = AC_TRUE;
		}
	}
}


static void CAnalysisParser_HttpHeader_resetFunc(CAnalysisParser* p_this){
	CAnalysisParser_HttpHeader_Super* self = (CAnalysisParser_HttpHeader_Super*)p_this;
	//
	self->parentMember.status = AnalysisParserStatus_Matching;
	self->parentMember.isEnd = AC_FALSE;
	//自分の変数をリセット
	self->status = HttpHeaderStatus_Attribute;
	CAnalysisParser_reset((CAnalysisParser*)self->attrNameParser);
	CAnalysisParser_reset((CAnalysisParser*)self->valueParser);
}

void CAnalysisParser_HttpHeader_getValue(CAnalysisParser_HttpHeader* p_this, const char** str, size_t* len){
	CAnalysisParser_HttpHeader_Super* self = (CAnalysisParser_HttpHeader_Super*)p_this;
	//文字列を取得する
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
	//初期化する
	CAnalysisParser_HttpHeader_resetFunc((CAnalysisParser*)self);
	return (CAnalysisParser_HttpHeader*)self;
}




