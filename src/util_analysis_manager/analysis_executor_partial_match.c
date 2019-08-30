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



static const char* gClassName_AnalysisExecutor_PartialMatch = "CAnalysisExecutor_PartialMatch";


//------------------------------------------------------
//----  CAnalysisExecutor_PartialMatch Class    ---------------------
//------------------------------------------------------
//------------------------------------------------------

/**
private
*/
typedef struct CAnalysisExecutor_PartialMatch_Super {
	CAnalysisExecutor_Super parentMember;
	///検索対象文字列
	char* targetStr;
	///検索対象文字列の長さ
	size_t targetStrLen;
	///現在位置
	char* targetStrPos;
	///置換文字列
	char* replaceStr;
} CAnalysisExecutor_PartialMatch_Super;


static void CAnalysisExecutor_PartialMatch__debugPrint(const CAnalysisExecutor* p_this) {
	const CAnalysisExecutor_PartialMatch_Super* self = (const CAnalysisExecutor_PartialMatch_Super*)p_this;
	//
	ac_printf("targetStr=%s[targetStrLen=%d, targetStrPos=%p], replaceStr=%s\n",
		self->targetStr, (int)self->targetStrLen, self->targetStrPos, self->replaceStr);
}


static AcBool CAnalysisExecutor_PartialMatch_start(CAnalysisExecutor* p_this, const apr_table_t* table) {
	//ac_printf("-[start]------------------\n");
	return AC_TRUE;
}

/**
このクラスの場合、forwardステータスでは以下の状態を扱うことにする。
・検索対象文字列の先頭の1文字とマッチする箇所を探す。見つかった場合はposステータスへ遷移。
*/
static const AnalysisCommand CAnalysisExecutor_PartialMatch_forward(CAnalysisExecutor* p_this, 
	const AnalysisCommand cmd, const AcBool isRejectPreCmd, const char c) 
{
	CAnalysisExecutor_PartialMatch_Super* self = (CAnalysisExecutor_PartialMatch_Super*)p_this;
	//前回のコマンドを実行拒否されたので状態リセットする
	if(isRejectPreCmd){
		self->targetStrPos = self->targetStr;
	}
	//
	AnalysisCommand nextCmd;
	//検索対象文字列の先頭の文字とマッチするか確認
	if(c == *self->targetStr){
		//先頭文字と一致する場合、posステータスに遷移。そのあとの文字もマッチするかチェックしていく。
		nextCmd.type = AC_POS;
		nextCmd.pos.pos = 1;
		self->targetStrPos = self->targetStr + 1;
		return nextCmd;
	}

	//マッチしなかった場合は、置換開始位置を１バイトすすめる。
	nextCmd.type = AC_FORWARD;
	return nextCmd;
}

static const AnalysisCommand CAnalysisExecutor_PartialMatch_replace(CAnalysisExecutor* p_this, 
	const AnalysisCommand cmd, AcBool result) 
{
	CAnalysisExecutor_PartialMatch_Super* self = (CAnalysisExecutor_PartialMatch_Super*)p_this;
	//
	AnalysisCommand nextCmd;
	nextCmd.type = AC_FORWARD;
	return nextCmd;
}

/**
このクラスの場合、posステータスでは以下の状態を扱うことにする。
@n ・検索対象文字列の2文字以降が最後までマッチするかをチェック。
@n   マッチの場合はreplaceステータスへ遷移。マッチしない場合はforwardステータスへ遷移。
*/
static const AnalysisCommand CAnalysisExecutor_PartialMatch_pos(CAnalysisExecutor* p_this, 
	const AnalysisCommand cmd, const char c, const AcBool isEos)
{
	CAnalysisExecutor_PartialMatch_Super* self = (CAnalysisExecutor_PartialMatch_Super*)p_this;
	//
	AnalysisCommand nextCmd;
	
	//EoSが来た場合、すべてマッチする前にEoSになったと考えられる
	if(isEos){
		nextCmd.type = AC_FORWARD_N;
		nextCmd.forward_n.len = cmd.pos.pos;
		return nextCmd;
	}

	//解析
	if (*self->targetStrPos == c) {
		//文字がマッチした場合
		++self->targetStrPos;
		if (*self->targetStrPos == '\0') {
			//すべてマッチした場合
			nextCmd.type = AC_REPLACE;
			nextCmd.replace.len = self->targetStrLen;
			nextCmd.replace.replacement = self->replaceStr;
		}
		else {
			nextCmd.type = AC_POS;
			nextCmd.pos.pos = cmd.pos.pos + 1;
		}
	}
	else {
		//マッチしなかった場合
		nextCmd.type = AC_FORWARD;
	}
	return nextCmd;
}

static const AnalysisCommand CAnalysisExecutor_PartialMatch_end(CAnalysisExecutor* p_this, const AnalysisCommand cmd) {
	CAnalysisExecutor_PartialMatch_Super* self = (CAnalysisExecutor_PartialMatch_Super*)p_this;
	//
	AnalysisCommand nextCmd;
	nextCmd.type = AC_END;
	nextCmd.end.status = -1;
	nextCmd.end.replacement = NULL;
	//
	//ac_printf("-[end]------------------\n");
	return nextCmd;
}



static void CAnalysisExecutor_PartialMatch_delete(void* p_this) {
	CAnalysisExecutor_PartialMatch_Super* self = (CAnalysisExecutor_PartialMatch_Super*)p_this; 
	ac_checkClass(gClassName_AnalysisExecutor_PartialMatch, self->parentMember.publicMember.thisIsClass->name, "in CAnalysisExecutor_PartialMatch_delete()", AC_FALSE);
	//
	free(self->replaceStr);
	free(self->targetStr);
	free(self);
}

CAnalysisExecutor_PartialMatch* CAnalysisExecutor_PartialMatch_new(const char* targetStr, const char* replaceStr) {
	CAnalysisExecutor_PartialMatch_Super* self = (CAnalysisExecutor_PartialMatch_Super*)malloc(sizeof(CAnalysisExecutor_PartialMatch_Super));
	if (self == NULL) return NULL;
	//親メンバーの初期化
	CAnalysisExecutor_init((CAnalysisExecutor*)self, gClassName_AnalysisExecutor_PartialMatch, 
		CAnalysisExecutor_PartialMatch_delete,
		CAnalysisExecutor_PartialMatch_start, 
		CAnalysisExecutor_PartialMatch_forward,
		CAnalysisExecutor_PartialMatch_replace,
		CAnalysisExecutor_PartialMatch_pos,
		CAnalysisExecutor_PartialMatch_end
		);
	//
	self->targetStrLen = strlen(targetStr);
	char* str = (char*)malloc(self->targetStrLen + 1);
	if (str == NULL) {
		free(self);
		return NULL;
	}
	strcpy(str, targetStr);
	self->targetStr = str;
	//
	str = (char*)malloc(strlen(replaceStr) + 1);
	if (str == NULL) {
		free(self->targetStr);
		free(self);
		return NULL;
	}
	strcpy(str, replaceStr);
	self->replaceStr = str;
	self->targetStrPos = str;
	//
	CAnalysisExecutor_setDebugPrintFunc((CAnalysisExecutor*)self, CAnalysisExecutor_PartialMatch__debugPrint);
	return (CAnalysisExecutor_PartialMatch*)self;
}
















