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
#include "analysis_executor_impl.h"
#include "analysis_manager_.h"


static const char* gClassName_AnalysisExecutor_HtmlTgaReplace = "CAnalysisExecutor_HtmlTagReplace";


//------------------------------------------------------
//----  CAnalysisExecutor_HtmlTagReplace Class    ---------------------
//------------------------------------------------------
//------------------------------------------------------

/**
@brief CAnalysisExecutor_HtmlTagReplace クラスの現在の状態。
*/
typedef enum HtmlTagReplaceStatus {
	///タグの開始（＜）を探している状態
	HtmlTagReplaceStatus_SearchStart,
	///タグ名がマッチするかを検証中の状態
	HtmlTagReplaceStatus_MatchingTagName,
	///タグ名の後ろ（スペースか＞）がマッチするかを検証中の状態
	HtmlTagReplaceStatus_MatchingAfterTagName,
	///タグの終了（＞）を探して、置換はしない状態（タグ名がマッチしなかった）。
	HtmlTagReplaceStatus_SearchTagEndWithNoReplace,
	///タグの終了（＞）を探して、置換しようとしている状態。
	HtmlTagReplaceStatus_SearchTagEndWithReplace
} HtmlTagReplaceStatus;


/**
privateまで含めたクラス定義。
*/
typedef struct CAnalysisExecutor_HtmlTagReplace_Super {
	CAnalysisExecutor_Super parentMember;
	///タグ名（小文字にして保管される）
	char* tagName;
	///タブ名の文字列長
	size_t tagNameStrLen;
	///タグ名チェック用。現在の位置。
	char* tagNameStrPos;
	///タグの終了（＞）位置までの長さ。"＞"を含む。
	size_t tagStrLen;
	///置換文字列
	char* replaceStr;
	///タグ自体を置換するのか？（AC_FALSEのとき、タグ自体は置換せず、見つかったタグの後ろに文字列を置く）
	AcBool isTagReplace;
	///読み飛ばしをして置換をする場合。
	HtmlTagReplaceStatus replaceStatus;
} CAnalysisExecutor_HtmlTagReplace_Super;



static void CAnalysisExecutor_HtmlTagReplace__debugPrint(const CAnalysisExecutor* p_this){
	const CAnalysisExecutor_HtmlTagReplace_Super* self = (const CAnalysisExecutor_HtmlTagReplace_Super*)p_this;
	//
	ac_printf("tagName=%s[tagNameStrLen=%d, tagNameStrPos=%p], tagStrLen=%d, replaceStr=%s, isTagReplace=%d, replaceStatus=%d\n",
		self->tagName, (int)self->tagNameStrLen, self->tagNameStrPos, (int)self->tagStrLen, self->replaceStr, 
		(int)self->isTagReplace, (int)self->replaceStatus);
}

/**
クラス内部の状態をリセットし、新たにforwardできる状態にする。
*/
static void reset(CAnalysisExecutor_HtmlTagReplace_Super* p_this){
	CAnalysisExecutor_HtmlTagReplace_Super* self = (CAnalysisExecutor_HtmlTagReplace_Super*)p_this;
	//状態をリセット
	self->tagStrLen = 0;
	self->replaceStatus = HtmlTagReplaceStatus_SearchStart;
}

static AcBool CAnalysisExecutor_HtmlTagReplace_start(CAnalysisExecutor* p_this, const apr_table_t* table){
	//ac_printf("-[start]------------------\n");
	return AC_TRUE;
}

/**
このクラスでは、このメソッドが呼ばれるのは以下のいずれかの状態のとき。
@n ・タグの開始（＜）を検索している
@n ・タグの終了（＞）まで進める（この場合、さらに以下のパターンがある）
@n    ┗タグ名が指定と違うためタグ終了位置まで読み飛ばす（置換は行わない）
@n    ┗タグ名が指定とマッチし、タグの後ろに文字列を挿入する
*/
static const AnalysisCommand CAnalysisExecutor_HtmlTagReplace_forward(CAnalysisExecutor* p_this, 
	const AnalysisCommand cmd, const AcBool isRejectPreCmd, const char c)
{
	CAnalysisExecutor_HtmlTagReplace_Super* self = (CAnalysisExecutor_HtmlTagReplace_Super*)p_this;
	
	//前回のコマンドを実行拒否されたので状態リセットする
	if(isRejectPreCmd){
		//リセット
		reset(self);
	}
	//
	AnalysisCommand nextCmd;

	//見つかったタグの後ろの位置まで進める場合。
	if(self->replaceStatus == HtmlTagReplaceStatus_SearchTagEndWithReplace
	   || self->replaceStatus == HtmlTagReplaceStatus_SearchTagEndWithNoReplace)
	{
		//置換位置（タグの終わりの次の位置）まで来たので置換する
		if(self->replaceStatus == HtmlTagReplaceStatus_SearchTagEndWithReplace){
			//置換する場合
			nextCmd.type = AC_REPLACE;
			nextCmd.replace.replacement = self->replaceStr;
			nextCmd.replace.len = 0;
			//ステータスを変更（タグの開始を探すステータスに）
			self->replaceStatus = HtmlTagReplaceStatus_SearchStart;
			return nextCmd;
		}
		
		//状態をリセットして下の処理に任せる
		reset(self);
	}

	//タグの開始位置を探す場合
	//タグの開始ではなかった場合
	if(c != '<'){
		nextCmd.type = AC_FORWARD;
		return nextCmd;
	}

	//タグの開始文字だった場合、AC_POSでタグの終了位置を検索する
	self->replaceStatus = HtmlTagReplaceStatus_MatchingTagName;
	nextCmd.type = AC_POS;
	nextCmd.pos.pos = 1;
	//検査位置をリセット
	self->tagNameStrPos = self->tagName;
	self->tagStrLen = 1;
	return nextCmd;
}

static const AnalysisCommand CAnalysisExecutor_HtmlTagReplace_replace(CAnalysisExecutor* p_this, 
	const AnalysisCommand cmd, AcBool result)
{
	CAnalysisExecutor_HtmlTagReplace_Super* self = (CAnalysisExecutor_HtmlTagReplace_Super*)p_this;
	//
	AnalysisCommand nextCmd;
	nextCmd.type = AC_FORWARD;

	//リセット
	reset(self);

	return nextCmd;
}

/**

*/
static const AnalysisCommand CAnalysisExecutor_HtmlTagReplace_pos(CAnalysisExecutor* p_this, 
	const AnalysisCommand cmd, const char c, const AcBool isEos)
{
	CAnalysisExecutor_HtmlTagReplace_Super* self = (CAnalysisExecutor_HtmlTagReplace_Super*)p_this;
	//
	AnalysisCommand nextCmd;

	++self->tagStrLen;

	//EoSだった場合でこの関数に来た場合、マッチもタグの終了も見つかってないので読み飛ばして終了させる。
	if(isEos){
		nextCmd.type = AC_FORWARD_N;
		nextCmd.forward_n.len = cmd.pos.pos;
		return nextCmd;
	}

	//タグ名のマッチングを検査している場合
	if(self->replaceStatus == HtmlTagReplaceStatus_MatchingTagName){
		if(tolower(c) == *self->tagNameStrPos){
			//1文字マッチした場合
			++self->tagNameStrPos;
			//タグが完全マッチした場合
			if(*self->tagNameStrPos == '\0'){
				self->replaceStatus = HtmlTagReplaceStatus_MatchingAfterTagName;
			}
		} else{
			//マッチしなかった場合、タグの終了を探す
			self->replaceStatus = HtmlTagReplaceStatus_SearchTagEndWithNoReplace;
		}
		//次の位置の文字を読み取る
		nextCmd.type = AC_POS;
		nextCmd.pos.pos = cmd.pos.pos + 1;
		return nextCmd;
	}

	//タグ名の後ろの文字がスペースや＞かを検査している場合
	if(self->replaceStatus == HtmlTagReplaceStatus_MatchingAfterTagName){
		//タグの終了の場合
		if(c == '>'){
			//ステータスを変えて、後ろの処理に任せる
			self->replaceStatus = HtmlTagReplaceStatus_SearchTagEndWithReplace;
		} else{
			//スペースの場合
			if(isspace(c)){
				//指定タグにマッチ＆タグの終了位置を探すステータスに変えて、後ろの処理に任せる
				self->replaceStatus = HtmlTagReplaceStatus_SearchTagEndWithReplace;
			} else{
				//指定タグにマッチしなかった＆タグの終了位置を探すステータスに変えて、後ろの処理に任せる
				self->replaceStatus = HtmlTagReplaceStatus_SearchTagEndWithNoReplace;
			}
		}
	}
	
	//タグがマッチせず、タグの終了位置を探している場合
	if(self->replaceStatus == HtmlTagReplaceStatus_SearchTagEndWithNoReplace){
		if(c == '>'){
			nextCmd.type = AC_FORWARD_N;
			nextCmd.forward_n.len = self->tagStrLen;
			self->tagStrLen = 0;
		} else{
			nextCmd.type = AC_POS;
			nextCmd.pos.pos = cmd.pos.pos + 1;
		}
		return nextCmd;
	}

	//タグがマッチして、タグの終了位置を探している場合
	if(self->replaceStatus == HtmlTagReplaceStatus_SearchTagEndWithReplace){
		if(c != '>'){//タグの終了ではない場合
			nextCmd.type = AC_POS;
			nextCmd.pos.pos = cmd.pos.pos + 1;
		}else{//タグの終了の場合
			if(self->isTagReplace){
				//タグ自体を置換する場合
				nextCmd.type = AC_REPLACE;
				nextCmd.replace.replacement = self->replaceStr;
				nextCmd.replace.len = self->tagStrLen;
			} else{
				//タグの後ろに文字列を挿入する場合
				nextCmd.type = AC_FORWARD_N;
				nextCmd.forward_n.len = self->tagStrLen;
			}
		}
		return nextCmd;
	}

	//ここには来るはずないのでエラー処理をする
	nextCmd.type = AC_PASS_THROUGH_ALL;
	fprintf(stderr, "Error in CAnalysisExecutor_HtmlTagReplace_pos");
	return nextCmd;
}

static const AnalysisCommand CAnalysisExecutor_HtmlTagReplace_end(CAnalysisExecutor* p_this, const AnalysisCommand cmd){
	CAnalysisExecutor_HtmlTagReplace_Super* self = (CAnalysisExecutor_HtmlTagReplace_Super*)p_this;
	//
	AnalysisCommand nextCmd;
	nextCmd.type = AC_END;
	nextCmd.end.status = -1;
	nextCmd.end.replacement = NULL;
	//
	//ac_printf("-[end]------------------\n");
	return nextCmd;
}



static void CAnalysisExecutor_HtmlTagReplace_delete(void* p_this){
	CAnalysisExecutor_HtmlTagReplace_Super* self = (CAnalysisExecutor_HtmlTagReplace_Super*)p_this;
	ac_checkClass(gClassName_AnalysisExecutor_HtmlTgaReplace, self->parentMember.publicMember.thisIsClass->name, "in CAnalysisExecutor_HtmlTagReplace_delete()", AC_FALSE);
	//
	free(self->tagName);
	free(self->replaceStr);
	free(self);
}

CAnalysisExecutor_HtmlTagReplace* CAnalysisExecutor_HtmlTagReplace_new(const char* targetTagName, const char* replaceStr, AcBool isTagReplace){
	CAnalysisExecutor_HtmlTagReplace_Super* self = (CAnalysisExecutor_HtmlTagReplace_Super*)malloc(sizeof(CAnalysisExecutor_HtmlTagReplace_Super));
	if(self == NULL) return NULL;
	//親メンバーの初期化
	CAnalysisExecutor_init((CAnalysisExecutor*)self, gClassName_AnalysisExecutor_HtmlTgaReplace,
		CAnalysisExecutor_HtmlTagReplace_delete,
		CAnalysisExecutor_HtmlTagReplace_start,
		CAnalysisExecutor_HtmlTagReplace_forward,
		CAnalysisExecutor_HtmlTagReplace_replace,
		CAnalysisExecutor_HtmlTagReplace_pos,
		CAnalysisExecutor_HtmlTagReplace_end
	);
	//
	self->tagNameStrLen = strlen(targetTagName);
	char* str = (char*)malloc(self->tagNameStrLen + 1);
	if(str == NULL){
		free(self);
		return NULL;
	}
	self->tagName = str;
	self->tagNameStrPos = str;
	//小文字にして保管
	for(const char* p = targetTagName; *p != '\0'; ++p, ++str)*str = tolower(*p);
	*str = '\0';
	//
	str = (char*)malloc(strlen(replaceStr) + 1);
	if(str == NULL){
		free(self->tagName);
		free(self);
		return NULL;
	}
	strcpy(str, replaceStr);
	self->replaceStr = str;
	//
	self->isTagReplace = isTagReplace;
	self->replaceStatus = HtmlTagReplaceStatus_SearchStart;
	self->tagStrLen = 0;
	//
	CAnalysisExecutor_setDebugPrintFunc((CAnalysisExecutor*)self, CAnalysisExecutor_HtmlTagReplace__debugPrint);
	return (CAnalysisExecutor_HtmlTagReplace*)self;
}





