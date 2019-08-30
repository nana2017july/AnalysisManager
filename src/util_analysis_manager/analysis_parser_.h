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

#ifndef __ANALYSIS_PARSER__H__
#define __ANALYSIS_PARSER__H__


#include "analysis_parser.h"



//------------------------------------------------------
//----    CAnalysisParser Class       ---------------------
//------------------------------------------------------

typedef enum{
	AnalysisParserStatus_Searching,
	AnalysisParserStatus_Matching,
	AnalysisParserStatus_Matched
} AnalysisParserStatus;

/**
private
*/
typedef struct {
	CAnalysisParser publicMember;
	
	//
	AcBool isEnd;
	AnalysisParserStatus status;
	///チェックをした文字列の長さ。パースした文字を読み飛ばすときに使用する。
	size_t parsedStrLen;
	void (*debugPrintFunc)(const CAnalysisParser* p_this);
	void (*acceptFunc)(CAnalysisParser* p_this, const char c);
	void (*resetFunc)(CAnalysisParser* p_this);
	AcDeleteFunc_T subclassDeleteFunc;
} CAnalysisParser_Super;


/**
@~japanese
@n 親クラスを初期化する
@param acceptFunc [in]次の文字を渡し、ステータスをチェックする関数。
@param resetFunc [in]リセットしてチェックをやり直せる状態にする関数。

@~english
@n Initialize parent class.
@param acceptFunc [in] Specifying next 1 byte , this function checks status.
@param resetFunc  [in] Invoking this function, class will be in status to start checking.
*/
void CAnalysisParser_init(CAnalysisParser* p_this, const char* className, AcDeleteFunc_T deleteFunc,
	void (*acceptFunc)(CAnalysisParser* p_this, const char c),
	void (*resetFunc)(CAnalysisParser* p_this)
);


/**
@~japanese
@n デバッグ用のprint関数を設定する。
@param debugPrintFunc [in]デバッグprint関数。

@~english
@n Set print function for debug.
@param debugPrintFunc [in] print function for debug.
*/
void CAnalysisParser_setDebugFunc(CAnalysisParser* p_this,
	void (*debugPrintFunc)(const CAnalysisParser* p_this)
);


#endif  //end __ANALYSIS_PARSER__H__