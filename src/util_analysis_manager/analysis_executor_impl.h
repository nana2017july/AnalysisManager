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

#ifndef __ANALYSIS_EXECUTOR_H__
#define __ANALYSIS_EXECUTOR_H__


#include "apr_buckets.h"
#include "ac_this_is_class.h"

#include "analysis_manager.h"


//------------------------------------------------------
//----    CAnalysisExecutor_PartialMatch Class       ---------------------
//------------------------------------------------------
/**
@brief CAnalysisExecutor の派生クラス。部分一致した文字を置換するクラス。
@see CAnalysisManager_new()
*/
typedef struct CAnalysisExecutor_PartialMatch {
	CAnalysisExecutor parentMember;
	//
} CAnalysisExecutor_PartialMatch;


CAnalysisExecutor_PartialMatch* CAnalysisExecutor_PartialMatch_new(const char* targetStr, const char* replaceStr);



//------------------------------------------------------
//----    CAnalysisExecutor_DoNothing Class       ---------------------
//------------------------------------------------------
/**
@~japanese
@brief CAnalysisExecutor の派生クラス。テスト用。何もしないクラス。これを使うことでExcutor以外の部分の処理にかかる時間が測れる。

@~english
@brief Derived class of CAnalysisExecutor for test. Ex. this is used to measure performance time without Executor class process. 

@~
@see CAnalysisManager_new()
*/
typedef struct CAnalysisExecutor_DoNothing {
	CAnalysisExecutor parentMember;
	//
} CAnalysisExecutor_DoNothing;


CAnalysisExecutor_DoNothing* CAnalysisExecutor_DoNothing_new();



//------------------------------------------------------
//----    CAnalysisExecutor_HtmlTagReplace Class       ---------------------
//------------------------------------------------------
/**
@brief CAnalysisExecutor の派生クラス。指定のHTMLタグを置換する。
@n このクラスの動作内容。
@n タグの開始（＜）を見つけたら、AC_POSでタグの終了（＞）を見つける。
@n その後、タグ自体を置換する場合はAC_REPLACEを実行。
@n タグの後ろを置換する場合はAC_FORWARDでタグの終了位置まで移動してから、AC_REPLACEを実行。
@see CAnalysisExecutor_HtmlTagReplace_new()
*/
typedef struct CAnalysisExecutor_HtmlTagReplace {
	CAnalysisExecutor parentMember;
	//
} CAnalysisExecutor_HtmlTagReplace;

/**
コンストラクタ
@param targetTagName [in]置換対象のタグ名
@param replaceStr [in]置換文字列
@param isTagReplace [in]タグ自体を置換するのか？FALSEのとき、タグ自体は置換せず、見つかったタグの後ろに置換文字を挿入する。
*/
CAnalysisExecutor_HtmlTagReplace* CAnalysisExecutor_HtmlTagReplace_new(const char* targetTagName, const char* replaceStr, AcBool isTagReplace);






//------------------------------------------------------
//----    CAnalysisExecutor_Multipart Class       ---------------------
//------------------------------------------------------
/**
@brief CAnalysisExecutor の派生クラス。マルチパートのリクエストを解析してパラメタを取得する。
@n     現状ではファイル（filenameがヘッダに存在するもの）は取得しない。メモリを多く消費してしまうためである。
@n     multipart/mixedのような二重構造には対応していない。
@n     取得したパラメタの名前と値の文字列の長さには最大桁数があり、最大桁数 MULTIPART_NAME_MAX_LEN , MULTIPART_VALUE_LEN と同じ長さの場合、
@n     もっと長い可能性もある。
@see CAnalysisExecutor_Multipart_new()
*/
typedef struct {
	CAnalysisExecutor parentMember;
	//
} CAnalysisExecutor_Multipart;

/**
コンストラクタ
@param table       [in]解析してパラメタ名と値を設定。apr_table_t に保存されるので値の取得の仕方はaprのリファレンスを参照のこと。
@n                     破壊は利用者側で行う。ただし、通常tableはpoolで作成し管理されるので、破壊はpoolに任せればよい。
@param targetParamName [in]現状未使用。""を設定。
@param boundaryStr [in]boundary文字列。"\r\n--" + boundary　を設定すること。
*/
CAnalysisExecutor_Multipart* CAnalysisExecutor_Multipart_new(apr_table_t* table, const char* targetParamStr, const char* boundaryStr);

/**
解析後にマルチパートのパラメタを取得する。パラメタ名をキーにして、パラメタ値を登録されている。
@return 解析した結果で得られたパラメタ
*/
const apr_table_t* CAnalysisExecutor_Multipart_getParams(CAnalysisExecutor_Multipart* p_this);




#endif  //end __ANALYSIS_EXECUTOR_H__