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

#ifdef _WIN32
// dllmain.cpp : DLL アプリケーションのエントリ ポイントを定義します。
#include "pch.h"
#include "win_huck.h"
#else
//windows以外の場合
#include <linux/limits.h>
#include <sys/types.h>
#include <unistd.h>
#endif
//
#include "apr.h"
#include "apr_strings.h" 
#include "apr_tables.h"
#include "httpd.h"
#include "http_config.h"
#include "http_protocol.h"
#include "http_log.h"
#include "ap_config.h"
//
#include "analysis_manager.h"
#include "analysis_executor_impl.h"
#include "ac_bucket_utils.h"


//宣言
static void mod_replace_content_register_hooks(apr_pool_t* p);
static void* create_dir_config(apr_pool_t* p, char* dir);
static const char* setConfReplaceContent(cmd_parms* cmd, void* c, const char* type, const char* target, const char* replacement);
static void* create_server_config(apr_pool_t* p, server_rec* s);


/**
httpd.confを読みとるための定義
*/
static command_rec replace_content_cmds[] = {
	AP_INIT_TAKE3("ReplaceContent", setConfReplaceContent, NULL, RSRC_CONF | ACCESS_CONF,
		"replace typeStr[partial_match | tag] for replace_content"),
{NULL},
};

/* Dispatch list for API hooks この変数名をhttpd.conf のloadmoduleに書く*/
#ifdef __APACHE24__
AP_DECLARE_MODULE(replace_content) = {
#else
module AP_MODULE_DECLARE_DATA replace_content_module = {
#endif
	STANDARD20_MODULE_STUFF,
	create_dir_config,                  /* create per-dirStr    config structures */
	NULL,                  /* merge  per-dirStr    config structures */
	create_server_config,                  /* create per-server config structures */
	NULL,                  /* merge  per-server config structures */
	replace_content_cmds,                  /* table of config file commands       */
	mod_replace_content_register_hooks  /* register hooks                      */
};


///命令のタイプ
typedef enum {
	ReplaceType_PartialMatch,
	ReplaceType_Tag
} ReplaceType;

///config 
typedef union ModuleConfig{
	ReplaceType replaceType;
	//
	struct{
		ReplaceType replaceType;
		char* target;
		char* replacement;
	} partialMatch;
	//
	struct{
		ReplaceType replaceType;
		char* tag;
		char* replacement;
		AcBool isTagReplace;
	} tag;
} ModuleConfig;


//
static void printFunc(const char* msg, va_list args){
	char str[301];
	vsnprintf(str, 300, msg, args);
	ap_log_error(APLOG_MARK, APLOG_NOTICE, 0, NULL, str);
}

static void test_printf(const char* msg, ...){
	va_list ap;
	va_start(ap, msg);
	printFunc(msg, ap);
}


/* mod_replace_content content handler */
static int mod_replace_content_handler(request_rec* r){
	return DECLINED;
}



//sample outout filter-------------------------------------------------
const char* OUTPUT_USER_DATA_KEY = "AnalysisManager";
//
static apr_status_t my_output_filter(ap_filter_t* f, apr_bucket_brigade* in_bb){
	pid_t pid = getpid();
	request_rec* r = f->r;
	server_rec* s = r->server;
	ModuleConfig* conf = (ModuleConfig*)ap_get_module_config(s->module_config, &replace_content_module);
	//
	CAnalysisManager* ana = NULL;
	apr_bucket_brigade* brigade = NULL;
	ana = ac_getClassFromPoolUserData(OUTPUT_USER_DATA_KEY, r->pool);
	if(ana == NULL){
		//生成
		CAnalysisExecutor* executor;
		if(conf->replaceType == ReplaceType_PartialMatch){
			executor = (CAnalysisExecutor*)CAnalysisExecutor_PartialMatch_new(
				conf->partialMatch.target, conf->partialMatch.replacement);
		} else{
			executor = (CAnalysisExecutor*)CAnalysisExecutor_HtmlTagReplace_new(
				conf->partialMatch.target, conf->partialMatch.replacement, conf->tag.isTagReplace);
		}
		
		//CANALYSIS_EXECUTOR_DEBUG_PRINT(executor);
		ana = CAnalysisManager_new(executor, r->pool, r->connection->bucket_alloc, NULL, in_bb);
		//空のbrigadeを破棄する
		apr_brigade_destroy(in_bb);
		//poolに登録
		ac_registClassToUserDataAndPoolCleanup(r->pool, OUTPUT_USER_DATA_KEY, ana);

		//CANALYSIS_MANAGER_DEBUG_PRINT(ana);

		//実行
		brigade = CAnalysisManager_run(ana, NULL);
	} else{
		test_printf("[my1:%d] start 3-1", pid);
		//ana = *data;
		brigade = CAnalysisManager_run(ana, in_bb);
		//空のbrigadeを破棄する
		apr_brigade_destroy(in_bb);
	}
	//test_printf("[my1:%d] start 4-1", pid);
	//CANALYSIS_MANAGER_DEBUG_PRINT(ana);

	//
	return ap_pass_brigade(f->next, brigade);
}






//sample input filter-------------------------------------------------
typedef struct{
	CAnalysisExecutor_Multipart* executor;
	AcBool isOutput;
} InputFilterRequestData;
const char* INPUT_MANAGER_USER_DATA_KEY = "Input_AnalysisManager";
const char* INPUT_FILTER_REQUEST_DATA_USER_DATA_KEY = "Input_AnalysisExecutor";
//
static apr_status_t my_input_filter(ap_filter_t* f, apr_bucket_brigade* in_bb,
	ap_input_mode_t mode, apr_read_type_e block, apr_off_t readbytes
){
	pid_t pid = getpid();
	request_rec* r = f->r;
	CAnalysisManager* ana = NULL;
	apr_bucket_brigade* brigade = NULL;

	//brigadeを吸い上げる
	apr_status_t status = ap_get_brigade(f->next, in_bb, mode, block, readbytes);
	if(status != APR_SUCCESS) return status;
	//ac_printBrigade(in_bb);

	//ヘッダを取得する
	apr_table_t* headers = r->headers_in;
	//test_printf("[in:%d] %s\n", pid, r->content_type);
	const char* contentTypeValue = (const char*)apr_table_get(headers, "content-type");
	if(contentTypeValue == NULL) return status;
	//マルチパートではない場合
	if(strncmp("multipart", contentTypeValue, 9) != 0) return status;
	
	//データを取得する
	InputFilterRequestData* myReqData = NULL;
	apr_pool_userdata_get((void**)&myReqData, INPUT_FILTER_REQUEST_DATA_USER_DATA_KEY, r->pool);
	if(myReqData == NULL){
		myReqData = (InputFilterRequestData*)apr_palloc(r->pool, sizeof(InputFilterRequestData));
		myReqData->isOutput = AC_FALSE;
		myReqData->executor = NULL;
		//userdataに登録
		apr_pool_userdata_set(myReqData, INPUT_FILTER_REQUEST_DATA_USER_DATA_KEY, apr_pool_cleanup_null, r->pool);
	}

	//保存したPoolからManegerを取得する
	ana = (CAnalysisManager*)ac_getClassFromPoolUserData(INPUT_MANAGER_USER_DATA_KEY, r->pool);
	if(ana == NULL){//なければ作成する
		//マルチパートの境界文字列を取得する
		const char* boundaryStr = NULL;
		if(contentTypeValue != NULL){
			//分解
			char** ary = ac_splitHttpHeaderValue(contentTypeValue, ";", AC_TRUE, 5);
			int i = ac_getIndexFromKeyValueArray(ary, "boundary");
			//boundaryが見つからない場合は返す
			if(i == -1) return status;
			//boundary文字列を作成する
			boundaryStr = apr_pstrcat(r->pool, "\r\n", "--", ary[i], NULL);
			free(ary);
		}

		//解析クラスの生成
		apr_table_t* table = apr_table_make(r->pool, 5);
		myReqData->executor = CAnalysisExecutor_Multipart_new(table, "", boundaryStr);

		//CANALYSIS_EXECUTOR_DEBUG_PRINT(executor);

		//マネージャー生成
		ana = CAnalysisManager_new((CAnalysisExecutor*)myReqData->executor, r->pool, r->connection->bucket_alloc, NULL, in_bb);
		//poolに登録
		ac_registClassToUserDataAndPoolCleanup(r->pool, INPUT_MANAGER_USER_DATA_KEY, ana);

		//実行
		brigade = CAnalysisManager_run(ana, NULL);
	} else{
		//test_printf("[in:%d] start 3-1, isEnd=%d", pid, CAnalysisManager_isEnd(ana));
		//
		brigade = CAnalysisManager_run(ana, in_bb);
	}
	
	//出力
	if(CAnalysisManager_isEnd(ana) && !myReqData->isOutput){
		//出力したフラグを立てておく
		myReqData->isOutput = AC_TRUE;

		//パラメタテーブルを取得
		const apr_table_t* paramsTable = CAnalysisExecutor_Multipart_getParams(myReqData->executor);
		const apr_array_header_t* tarr = apr_table_elts(paramsTable);
		const apr_table_entry_t* telts = (const apr_table_entry_t*)tarr->elts;
		
		for(int i = 0; i < tarr->nelts; i++){
			test_printf("**** paramsTable[%s] = %s\n", telts[i].key, telts[i].val);
		}
	}
	//
	apr_brigade_cleanup(in_bb);
	APR_BRIGADE_CONCAT(in_bb, brigade);
	apr_brigade_destroy(brigade);
	//ac_printBrigade(in_bb);
	
	return status;
}





//conf設定-------------------------------------------------
static AcBool isBlank(const char* str){
	if(str == NULL) return AC_TRUE;
	if(*str == '\0') return AC_TRUE;
	return AC_FALSE;
}

/**
ディレクティブ（httpd.conf 内の @<Directory> か @<Location> の中）のconf構造体の生成
*/
static void* create_dir_config(apr_pool_t* p, char* dirStr){
	ModuleConfig* conf = (ModuleConfig*)apr_pcalloc(p, sizeof(ModuleConfig));
	//
	conf->replaceType = ReplaceType_PartialMatch;
	conf->partialMatch.target = "";
	conf->partialMatch.replacement = "";
	return (void*)conf;
}

/**
ディレクティブなし（httpd.conf 内の <Directory>の外か<Location> の外）のconf構造体の生成
*/
static void* create_server_config(apr_pool_t* p, server_rec* s){
	return create_dir_config(p, NULL);
}

/**
ReplaceContentディレクティブの設定
*/
static const char* setConfReplaceContent(cmd_parms* cmd, void* c, const char* typeStr, const char* targetStr, const char* replacementStr){
	ModuleConfig* conf = (ModuleConfig*)ap_get_module_config(cmd->server->module_config, &replace_content_module);
	//
	if(strcmp("partial_match", typeStr) == 0){
		conf->replaceType = ReplaceType_PartialMatch;
	}else if(strncmp("tag", typeStr, 3) == 0){
		//test_printf("###%s", &typeStr[4]);
		AcBool isReplaceTag = AC_FALSE;
		if(strlen(&typeStr[4]) >= 4){
			if(strncmp(&typeStr[4], "true", 4) == 0) isReplaceTag = AC_TRUE;
		}
		//test_printf("###%s", (isReplaceTag ? "true" : "false"));
		conf->replaceType = ReplaceType_Tag;
		conf->tag.isTagReplace = isReplaceTag;
	} else{
		return "replaceType is invalid in ReplaceContent.";
	}

	//
	if(!isBlank(targetStr)){
		conf->partialMatch.target = apr_pstrdup(cmd->pool, targetStr);
	} else{
		return "targetStr is invalid in ReplaceContent.";
	}

	//
	if(!isBlank(replacementStr)){
		conf->partialMatch.replacement = apr_pstrdup(cmd->pool, replacementStr);
	} else{
		return "replacementStr is invalid in ReplaceContent.";
	}

	return NULL;
}


static void mod_replace_content_register_hooks(apr_pool_t* p){
	ap_hook_handler(mod_replace_content_handler, NULL, NULL, APR_HOOK_MIDDLE);

	ap_register_output_filter("REPLACE_CONTENT_OUTPUTFILTER", my_output_filter, NULL,AP_FTYPE_RESOURCE);
	ap_register_input_filter("REPLACE_CONTENT_INPUTFILTER", my_input_filter, NULL, AP_FTYPE_RESOURCE);
	
	//出力関数の設定
	ac_setPrintFunc(printFunc);
}




