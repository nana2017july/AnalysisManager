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

#include <stdlib.h>
#include <stdio.h>
#include <time.h> 
//
#include "ac_bucket_utils.h"

///Apache系の共通インクルード 
#include "apr.h"
#include "apr_general.h"
#include "apr_pools.h"
#include "apr_file_io.h"
#include "apr_tables.h"
#include "apr_strings.h" 
#include "apr_buckets.h"

//
#include "analysis_manager.h"
//テストのためにinclude。通常はincludeしてはいけない。
#include "analysis_manager_.h"
#include "analysis_executor_impl.h"

#include "sctest_utils.h"



//テストのために型定義
typedef union _Temp {
	CAnalysisManager_Super* s;
	CAnalysisManager* a;
}_Temp;



/**
テスト。
*/
static void test_AnalysisExecutor_Multipart_run1(){
	apr_bucket_alloc_t* ba = apr_bucket_alloc_create(sctest_global_pool);

	//brigade作成 
	const char* tmp[] = {
		"--boundXXX\r\n", "Content-Disposition:name=test; \r\n\r\nvall\r\n--boundXXX", 
		"\r\nContent-Disposition:name=test2; \r\n\r\n222\r\n--boundXXX--", SCTEST_BT_EOS
	};
	//printf(tmp[0]);
	AcBool bl = AC_FALSE;
	apr_bucket_brigade* resultBB = NULL;
	apr_table_t* table = apr_table_make(sctest_global_pool, 5);
	apr_bucket_brigade* brigade = NULL;
	CAnalysisExecutor* executor = NULL;
	//
	apr_bucket_brigade** bb = sctest_createBucketBrigade(sctest_global_pool, ba, tmp);

	_Temp ana;


	//--------------------------------------------------------------------
	//--パターン-------------------------------
	//--------------------------------------------------------------------
	//生成
	executor = (CAnalysisExecutor*)CAnalysisExecutor_Multipart_new(table, "", "\r\n--boundXXX");
	ana.a = CAnalysisManager_new(executor, sctest_global_pool, ba, table, bb[0]);
	//
	brigade = CAnalysisManager_run(ana.a, NULL);
	A_NOT_NULL(brigade, "run()");
	//CANALYSIS_MANAGER_DEBUG_PRINT(ana.a);
	const apr_table_t* params = CAnalysisExecutor_Multipart_getParams((CAnalysisExecutor_Multipart*)executor);

	//
	A_EQUALS_STR("vall", apr_table_get(table, "test"), "");
	A_EQUALS_STR("222", apr_table_get(table, "test2"), "");

	apr_brigade_destroy(brigade);
	AcClass_delete(ana.a);
	sctest_destroy_brigades(bb);
};



/**
fileを含むテスト。
*/
static void test_AnalysisExecutor_Multipart_includeFile(){
	apr_bucket_alloc_t* ba = apr_bucket_alloc_create(sctest_global_pool);

	//brigade作成 
	const char* tmp[] = {
		"--boundXXX\r\n", 
		"Content-Disposition:name=test; \r\n\r\nvall\r\n--boundXXX",
		"\r\nContent-Disposition:name=test2; \r\n\r\n\r\n--boundXXX",
		"\r\nContent-Disposition: form-data; name=\"test3\"; filename=\"a.txt\"\r\n--boundXXX--\r\n", SCTEST_BT_EOS
	};
	//printf(tmp[0]);
	AcBool bl = AC_FALSE;
	apr_bucket_brigade* resultBB = NULL;
	apr_table_t* table = apr_table_make(sctest_global_pool, 5);
	apr_bucket_brigade* brigade = NULL;
	CAnalysisExecutor* executor = NULL;
	//
	apr_bucket_brigade** bb = sctest_createBucketBrigade(sctest_global_pool, ba, tmp);

	_Temp ana;


	//--------------------------------------------------------------------
	//--パターン-------------------------------
	//--------------------------------------------------------------------
	//生成
	executor = (CAnalysisExecutor*)CAnalysisExecutor_Multipart_new(table, "", "\r\n--boundXXX");
	ana.a = CAnalysisManager_new(executor, sctest_global_pool, ba, table, bb[0]);
	//
	brigade = CAnalysisManager_run(ana.a, NULL);
	A_NOT_NULL(brigade, "run()");
	//CANALYSIS_MANAGER_DEBUG_PRINT(ana.a);
	const apr_table_t* params = CAnalysisExecutor_Multipart_getParams((CAnalysisExecutor_Multipart*)executor);

	//
	A_EQUALS_STR("vall", apr_table_get(table, "test"), "");
	A_EQUALS_STR("", apr_table_get(table, "test2"), "");
	A_NULL(apr_table_get(table, "test3"), "");

	apr_brigade_destroy(brigade);
	AcClass_delete(ana.a);
	sctest_destroy_brigades(bb);
};



/**
準正常系のテスト。悪意のあるユーザからの攻撃を想定したテスト。
*/
static void test_AnalysisExecutor_Multipart_invalideBody(){
	apr_bucket_alloc_t* ba = apr_bucket_alloc_create(sctest_global_pool);

	//brigade作成 
	const char* tmp[] = {
		"\r\n", SCTEST_BT_EOS
	};
	//printf(tmp[0]);
	AcBool bl = AC_FALSE;
	apr_bucket_brigade* resultBB = NULL;
	apr_table_t* table = apr_table_make(sctest_global_pool, 5);
	apr_bucket_brigade* brigade = NULL;
	CAnalysisExecutor* executor = NULL;
	//
	apr_bucket_brigade** bb = sctest_createBucketBrigade(sctest_global_pool, ba, tmp);

	_Temp ana;


	//--------------------------------------------------------------------
	//--boundaryもヘッダもないパターン-------------------------------
	//--------------------------------------------------------------------
	//生成
	executor = (CAnalysisExecutor*)CAnalysisExecutor_Multipart_new(table, "", "\r\n--boundXXX");
	ana.a = CAnalysisManager_new(executor, sctest_global_pool, ba, table, bb[0]);
	//
	brigade = CAnalysisManager_run(ana.a, NULL);
	A_NOT_NULL(brigade, "run()");
	//CANALYSIS_MANAGER_DEBUG_PRINT(ana.a);
	const apr_table_t* params = CAnalysisExecutor_Multipart_getParams((CAnalysisExecutor_Multipart*)executor);

	//
	A_TRUE(CAnalysisManager_isEnd(ana.a), "");
	A_NULL(apr_table_get(table, "test"), "");

	apr_brigade_destroy(brigade);
	AcClass_delete(ana.a);
	sctest_destroy_brigades(bb);

	//--------------------------------------------------------------------
	//--ヘッダはあるがboundaryがないパターン-------------------------------
	//--------------------------------------------------------------------
	//brigade作成 
	const char* tmp2[] = {
		"\r\n", "Content-Disposition:nam=test; \r\n", SCTEST_BT_EOS
	};
	bb = sctest_createBucketBrigade(sctest_global_pool, ba, tmp);
	//生成
	executor = (CAnalysisExecutor*)CAnalysisExecutor_Multipart_new(table, "", "\r\n--boundXXX");
	ana.a = CAnalysisManager_new(executor, sctest_global_pool, ba, table, bb[0]);
	//
	brigade = CAnalysisManager_run(ana.a, NULL);
	A_NOT_NULL(brigade, "run()");
	//CANALYSIS_MANAGER_DEBUG_PRINT(ana.a);
	params = CAnalysisExecutor_Multipart_getParams((CAnalysisExecutor_Multipart*)executor);

	//
	A_TRUE(CAnalysisManager_isEnd(ana.a), "");
	A_NULL(apr_table_get(table, "test"), "");

	apr_brigade_destroy(brigade);
	AcClass_delete(ana.a);
	sctest_destroy_brigades(bb);

};





/**
実際にブラウザから届いたbucket。エラーになったもの。
------WebKitFormBoundarycpV3BTp8bY9whOCB\r\nContent-Disposition: form-data; name="me;ssage"\r\n\r\nHe;llo\r\n------WebKitFormBoundarycpV3BTp8bY9whOCB\r\nContent-Disposition: form-data; name="file"; filename=""\r\nContent-Type: application/octet-stream\r\n\r\n\r\n------WebKitFormBoundarycpV3BTp8bY9wh
*/
static void test_AnalysisExecutor_Multipart_errTest(){
	apr_bucket_alloc_t* ba = apr_bucket_alloc_create(sctest_global_pool);

	//brigade作成 
	const char* tmp[] = {
		"------WebKitFormBoundarycpV3BTp8bY9whOCB\r\nContent-Disposition: form-data; name=\"me; ssage\"\r\n\r\nHe;llo\r\n------WebKitFormBoundarycpV3BTp8bY9whOCB\r\nContent-Disposition: form-data; name=\"file\"; filename=\"\"\r\nContent-Type: application/octet-stream\r\n\r\n\r\n------WebKitFormBoundarycpV3BTp8bY9wh--\r\n", 
		SCTEST_BT_EOS
	};
	//printf(tmp[0]);
	AcBool bl = AC_FALSE;
	apr_bucket_brigade* resultBB = NULL;
	apr_table_t* table = apr_table_make(sctest_global_pool, 5);
	apr_bucket_brigade* brigade = NULL;
	CAnalysisExecutor* executor = NULL;
	//
	apr_bucket_brigade** bb = sctest_createBucketBrigade(sctest_global_pool, ba, tmp);

	_Temp ana;


	//--------------------------------------------------------------------
	//--パターン-------------------------------
	//--------------------------------------------------------------------
	//生成
	executor = (CAnalysisExecutor*)CAnalysisExecutor_Multipart_new(table, "", "\r\n------WebKitFormBoundarycpV3BTp8bY9wh");
	ana.a = CAnalysisManager_new(executor, sctest_global_pool, ba, table, bb[0]);
	//
	brigade = CAnalysisManager_run(ana.a, NULL);
	A_NOT_NULL(brigade, "run()");
	A_TRUE(CAnalysisManager_isEnd(ana.a), "isEnd");
	//CANALYSIS_MANAGER_DEBUG_PRINT(ana.a);
	const apr_table_t* params = CAnalysisExecutor_Multipart_getParams((CAnalysisExecutor_Multipart*)executor);

	//
	A_EQUALS_STR("He;llo", apr_table_get(table, "me; ssage"), "");
	A_NULL(apr_table_get(table, "file"), "");

	apr_brigade_destroy(brigade);
	AcClass_delete(ana.a);
	sctest_destroy_brigades(bb);
};



void test_analysis_executor_Multipart(){
	test_AnalysisExecutor_Multipart_run1();
	test_AnalysisExecutor_Multipart_includeFile();
	test_AnalysisExecutor_Multipart_invalideBody();
	//
	test_AnalysisExecutor_Multipart_errTest();
}