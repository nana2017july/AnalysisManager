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
#include "analysis_parser.h"
#include "analysis_parser_.h"
#include "ac_bucket_utils.h"

//
static const char* statusSearching = "[AnalysisParserStatus_Searching]";
static const char* statusMatching = "[AnalysisParserStatus_Matching]";
static const char* statusMatched = "[AnalysisParserStatus_Matched]";
static const char* statusNone = "[?]";

///ステータスを文字列にする。デバッグ用。
static const char* getStatusStr(AnalysisParserStatus status){
	if(status == AnalysisParserStatus_Searching){
		return statusSearching;
	} else if(status == AnalysisParserStatus_Matching){
		return statusMatching;
	} else if(status == AnalysisParserStatus_Matched){
		return statusMatched;
	}
	return statusNone;
}


/**public:
デバッグ用の関数
*/
void CAnalysisParser__debugPrint(const CAnalysisParser* p_this, const char* file, int line){
	const CAnalysisParser_Super* self = (const CAnalysisParser_Super*)p_this;
	//char format[50];
	const char* filename = (!strrchr(file, '\\') ? file : strrchr(file, '\\') + 1);

	ac_printf("-------debugPrint(%s, %d):\n%s[%p], \n", filename, line, AcClass_getName((void*)self), self);
	ac_printf("status=%s, isEnd=%d, parsedStrLen=%u\n", getStatusStr(self->status), self->isEnd, self->parsedStrLen);
	if(self->debugPrintFunc != NULL){
		self->debugPrintFunc((CAnalysisParser*)self);
	}
	ac_printf("-----------------------------------------------------\n");
}


void CAnalysisParser_accept(CAnalysisParser* p_this, const char c){
	CAnalysisParser_Super* self = (CAnalysisParser_Super*)p_this;
	//
	++self->parsedStrLen;
	self->acceptFunc((CAnalysisParser*)self, c);
}


void CAnalysisParser_reset(CAnalysisParser* p_this){
	CAnalysisParser_Super* self = (CAnalysisParser_Super*)p_this;
	//
	self->resetFunc((CAnalysisParser*)self);
	self->parsedStrLen = 0;
}


AcBool CAnalysisParser_isEnd(CAnalysisParser* p_this){
	CAnalysisParser_Super* self = (CAnalysisParser_Super*)p_this;
	//
	return self->isEnd;
}


size_t CAnalysisParser_getParsedStrLen(CAnalysisParser* p_this){
	CAnalysisParser_Super* self = (CAnalysisParser_Super*)p_this;
	//
	return self->parsedStrLen;
}


AcBool CAnalysisParser_isMatched(CAnalysisParser* p_this){
	CAnalysisParser_Super* self = (CAnalysisParser_Super*)p_this;
	//
	return self->status == AnalysisParserStatus_Matched;
}

void CAnalysisParser_setDebugFunc(CAnalysisParser* p_this,
	void (*debugPrintFunc)(const CAnalysisParser* p_this)
){
	CAnalysisParser_Super* self = (CAnalysisParser_Super*)p_this;
	//
	self->debugPrintFunc = debugPrintFunc;
}


static void CAnalysisParser_delete(void* p_this){
	CAnalysisParser_Super* self = (CAnalysisParser_Super*)p_this;
	self->subclassDeleteFunc(self);
	//selfは派生クラスでfreeする。生成したクラスがfreeするルール。
	//free(self);
}


void CAnalysisParser_init(CAnalysisParser* p_this, const char* className, AcDeleteFunc_T deleteFunc,
	void (*acceptFunc)(CAnalysisParser* p_this, const char c), 
	void (*resetFunc)(CAnalysisParser* p_this)
){
	CAnalysisParser_Super* self = (CAnalysisParser_Super*)p_this;
	//
	self->publicMember.thisIsClass = AcCThisIsClass_new(className, CAnalysisParser_delete);
	self->isEnd = AC_FALSE;
	self->status = AnalysisParserStatus_Searching;
	self->parsedStrLen = 0;
	self->debugPrintFunc = NULL;
	self->acceptFunc = acceptFunc;
	self->resetFunc = resetFunc;
	self->subclassDeleteFunc = deleteFunc;
}



