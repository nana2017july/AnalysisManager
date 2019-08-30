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


static const char* gClassName_AnalysisExecutor_DoNothing = "CAnalysisExecutor_DoNothing";


//------------------------------------------------------
//----  CAnalysisExecutor_DoNothing Class    ---------------------
//------------------------------------------------------
//------------------------------------------------------

/**
private
*/
typedef struct CAnalysisExecutor_DoNothing_Super {
	CAnalysisExecutor_Super parentMember;
	//
	char* targetStr;
	size_t targetStrLen;
	char* replaceStr;
	char* targetStrPos;
} CAnalysisExecutor_DoNothing_Super;


static void CAnalysisExecutor_DoNothing__debugPrint(const CAnalysisExecutor* p_this){
	const CAnalysisExecutor_DoNothing_Super* self = (const CAnalysisExecutor_DoNothing_Super*)p_this;
}


static AcBool CAnalysisExecutor_DoNothing_start(CAnalysisExecutor* p_this, const apr_table_t* table){
	//ac_printf("-[start]------------------\n");
	return AC_TRUE;
}

static const AnalysisCommand CAnalysisExecutor_DoNothing_forward(CAnalysisExecutor* p_this, 
	const AnalysisCommand cmd, const AcBool isRejectPreCmd, const char c)
{
	CAnalysisExecutor_DoNothing_Super* self = (CAnalysisExecutor_DoNothing_Super*)p_this;
	//
	//前回のコマンドを実行拒否されたので状態リセットする
	if(isRejectPreCmd){
		//このクラスでは特にリセットの必要なし。
	}
	AnalysisCommand nextCmd;
	nextCmd.type = AC_FORWARD;
	return nextCmd;
}

static const AnalysisCommand CAnalysisExecutor_DoNothing_replace(CAnalysisExecutor* p_this, 
	const AnalysisCommand cmd, AcBool result)
{
	CAnalysisExecutor_DoNothing_Super* self = (CAnalysisExecutor_DoNothing_Super*)p_this;
	//
	AnalysisCommand nextCmd;
	nextCmd.type = AC_FORWARD;
	return nextCmd;
}

static const AnalysisCommand CAnalysisExecutor_DoNothing_pos(CAnalysisExecutor* p_this, 
	const AnalysisCommand cmd, const char c, const AcBool isEos)
{
	CAnalysisExecutor_DoNothing_Super* self = (CAnalysisExecutor_DoNothing_Super*)p_this;
	//
	AnalysisCommand nextCmd;

	//EOSの場合、読み飛ばして終了させる
	if(isEos){
		nextCmd.type = AC_FORWARD_N;
		nextCmd.forward_n.len = cmd.pos.pos;
		return nextCmd;
	}

	nextCmd.type = AC_FORWARD;
	return nextCmd;
}

static const AnalysisCommand CAnalysisExecutor_DoNothing_end(CAnalysisExecutor* p_this, const AnalysisCommand cmd){
	CAnalysisExecutor_DoNothing_Super* self = (CAnalysisExecutor_DoNothing_Super*)p_this;
	//
	AnalysisCommand nextCmd;
	nextCmd.type = AC_END;
	nextCmd.end.status = -1;
	nextCmd.end.replacement = NULL;
	//
	//printf("-[end]------------------\n");
	return nextCmd;
}



static void CAnalysisExecutor_DoNothing_delete(void* p_this){
	CAnalysisExecutor_DoNothing_Super* self = (CAnalysisExecutor_DoNothing_Super*)p_this;
	ac_checkClass(gClassName_AnalysisExecutor_DoNothing, self->parentMember.publicMember.thisIsClass->name, "in CAnalysisExecutor_DoNothing_delete()", AC_FALSE);
	//
	free(self);
}

/**
@brief コンストラクタ。
*/
CAnalysisExecutor_DoNothing* CAnalysisExecutor_DoNothing_new(){
	CAnalysisExecutor_DoNothing_Super* self = (CAnalysisExecutor_DoNothing_Super*)malloc(sizeof(CAnalysisExecutor_DoNothing_Super));
	if(self == NULL) return NULL;
	//親メンバーの初期化
	CAnalysisExecutor_init((CAnalysisExecutor*)self, gClassName_AnalysisExecutor_DoNothing,
		CAnalysisExecutor_DoNothing_delete,
		CAnalysisExecutor_DoNothing_start,
		CAnalysisExecutor_DoNothing_forward,
		CAnalysisExecutor_DoNothing_replace,
		CAnalysisExecutor_DoNothing_pos,
		CAnalysisExecutor_DoNothing_end
	);
	//
	CAnalysisExecutor_setDebugPrintFunc((CAnalysisExecutor*)self, CAnalysisExecutor_DoNothing__debugPrint);
	return (CAnalysisExecutor_DoNothing*)self;
}











