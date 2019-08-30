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

#ifndef __ANALYSIS_PARSER_H__
#define __ANALYSIS_PARSER_H__


#include "ac_this_is_class.h"



//------------------------------------------------------
//----    CAnalysisParser Class       ---------------------
//------------------------------------------------------
/**
@~japanese
@brief CAnalysisParser は CAnalysisExecutor	の補助クラス。文字列をチェックし、ある条件を満たしたときにマッチしたことを
@n     知らせるクラス。ある条件は派生クラスで実装される。

@~english
@brief CAnalysisParser is assistant of CAnalysisExecutor class, that checks if strings satisfies some conditions.
@n     "some conditions" are implemented by derived class.
*/
typedef struct {
	AcCThisIsClass* thisIsClass;
} CAnalysisParser;



/**
@~japanese
@n 次の文字を渡し、ステータスをチェックする
@param c [in]次に読み込む1バイト。

@~english
@n Specifying next 1 byte , this function checks status.
@param c [in]next 1 byte to be read.
*/
void CAnalysisParser_accept(CAnalysisParser* p_this, const char c);


/**
@~japanese
@n リセットしてチェックをやり直せる状態にする関数。

@~english
@n Invoking this function, class will be in status to start checking.
*/
void CAnalysisParser_reset(CAnalysisParser* p_this);


/**
@~japanese
@n 終了状態か？trueの場合、構文解析が終わり、構文が正しいか、間違っているかが判断付いた状態。

@~english
@n Is this finished? If true , analysis is finished and It is determined that the syntax is correct or incorrect.
*/
AcBool CAnalysisParser_isEnd(CAnalysisParser* p_this);


/**
@~japanese
@n パースした文字数。 CAnalysisParser_reset を呼ぶと0に戻り、そこから文字数をカウントする。

@~english
@n Number of characters parsed. Invoking CAnalysisParser_reset ,number return to 0 and count the number of characters from there.
*/
size_t CAnalysisParser_getParsedStrLen(CAnalysisParser* p_this);


/**
@~japanese
@n 指定の構文にマッチした状態か？構文は派生クラスが知っている。

@~english
@n Is syntax matched? The Syntax is known by derived class.
*/
AcBool CAnalysisParser_isMatched(CAnalysisParser* p_this);

/**
print function for debug.
*/
void CAnalysisParser__debugPrint(const CAnalysisParser* p_this, const char* file, int line);
#define CANALYSIS_PARSER_DEBUG_PRINT(p_this) CAnalysisParser__debugPrint(p_this, __FILE__, __LINE__);


#endif  //end __ANALYSIS_PARSER_H__