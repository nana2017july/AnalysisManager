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

#ifndef __ANALYSIS_PARSER_IMPL_H__
#define __ANALYSIS_PARSER_IMPL_H__


#include "analysis_parser.h"





//------------------------------------------------------
//----    CAnalysisParser_split Class       ---------------------
//------------------------------------------------------
/**
@~japanese
@brief 指定の区切り文字列を検索するクラス。区切り文字列を見つけた位置でisEnd=trueとなる。
@n     区切り文字を見つけるまで文字列を内部にストックする。設定したストックのサイズを超えた場合でもパースは続ける。

@~english
@brief A class that searches delimitar string.It becomes isEnd = true at the position where it finds the delimiter string.
@n Stock strings internally until it finds a delimiter. It continues parsing even if over stock size.
*/
typedef struct {
	CAnalysisParser parentMember;
} CAnalysisParser_Split;


/**
@~japanese
@brief コンストラクタ。
@param targetStr [in]区切り文字列。

@~english
@brief Constructor.
@param targetStr [in]target delimiter string.
*/
CAnalysisParser_Split* CAnalysisParser_Split_new(const char* delimiterStr, size_t stockStrSize);


/**
@~japanese
@brief ストックした文字列を取得する。
@param str [out]ストックした文字列のポインタをセットして返す。
@param len [out]ストックした文字列の長さをセットして返す。

@~english
@brief Get the stocked string.
@ param str [out] Set and return a pointer to the stocked string.
@ param len [out] Set and return the length of the stocked string.
*/
void CAnalysisParser_Split_getStockStr(CAnalysisParser_Split* p_this, const char** str, size_t* len);



//------------------------------------------------------
//----    CAnalysisParser_CaseInsensitive Class       ---------------------
//------------------------------------------------------
/**
@~japanese
@brief 大文字小文字を気にせずに検索し、指定の文字列が見つかったかどうかをチェックするクラス。
@n     現在位置からマッチを開始するので、部分一致検索をする場合、マッチしなかった都度、リセットをする必要がある。

@~english
@brief A class that searches case-insensitively and checks if the specified string is found.
@n   Starting with current position, So if you want to do partial matching, you neet to reset at unmatched.
*/
typedef struct {
	CAnalysisParser parentMember;
} CAnalysisParser_CaseInsensitive;


/**
@~japanese
@brief コンストラクタ。
@param targetStr [in]検索対象の文字列。

@~english
@brief Constructor.
@param targetStr [in]target string for search.
*/
CAnalysisParser_CaseInsensitive* CAnalysisParser_CaseInsensitive_new(const char* targetStr);







//------------------------------------------------------
//----    CAnalysisParser_HttpHeader Class       ---------------------
//------------------------------------------------------
/**
@~japanese
@brief Httpヘッダの形式で指定の属性名が見つかるか解析する。見つかった場合はストックに値をコピーする。
@n   指定のストックサイズを超えてコピーはしない。
@n   すぐにマッチングが開始されるので、行の先頭位置でリセットする必要がある。

@~english
@brief Analyzes whether the specified attribute name is found in the form of Http header. If found, copy the value to stock string.
@n Do not copy beyond the specified stock size.
@n Starting matching soon, it should reset at top of the line. 
*/
typedef struct {
	CAnalysisParser parentMember;
} CAnalysisParser_HttpHeader;


/**
@~japanese
@brief コンストラクタ。
@param headerName [in]HTTPヘッダ属性名。(例："Content-Disposition:")
@param stockStrSize [in]ストックサイズ。ヘッダ属性値のコピー先バッファの最大サイズ。

@~english
@brief Constructor.
@param headearName [in]HTTP header attribute name .(ex."Content-Disposition:")
@param stockStrSize [in]Stock size. Max size of copying HTTP header value.
*/
CAnalysisParser_HttpHeader* CAnalysisParser_HttpHeader_new(const char* headearName, size_t stockStrSize);

/**
@~japanese
@brief ストックした文字列を取得する。
@param str [out]ストックした文字列のポインタをセットして返す。
@param len [out]ストックした文字列の長さをセットして返す。

@~english
@brief Get the stocked string.
@ param str [out] Set and return a pointer to the stocked string.
@ param len [out] Set and return the length of the stocked string.
*/
void CAnalysisParser_HttpHeader_getValue(CAnalysisParser_HttpHeader* p_this, const char** str, size_t* len);







//------------------------------------------------------
//----    CAnalysisParser_LineUp Class       ---------------------
//------------------------------------------------------
/**
@~japanese
@brief CAnalysisParser を順番に実行するクラス。複数のパーサを内部に保持し、isEndがtrueになったら次のパーサを実行する。
@n     最後のパーサがisEndがtrueになった時点で、このパーサもisEndがtrueになる。

@~english
@brief Class that executes CAnalysisParser clsses in order. Hold multiple parsers internally, and execute the next parser when isEnd becomes true.
@n     When the last parser is isEnd = true, this parser is also isEnd = true.
*/
typedef struct {
	CAnalysisParser parentMember;
} CAnalysisParser_LineUp;


/**
@~japanese
@brief コンストラクタ。
@param targetStr [in]検索対象の文字列。

@~english
@brief Constructor.
@param targetStr [in]target string for search.
*/
CAnalysisParser_LineUp* CAnalysisParser_LineUp_new(const char* targetStr);






#endif  //end __ANALYSIS_PARSER_IMPL_H__