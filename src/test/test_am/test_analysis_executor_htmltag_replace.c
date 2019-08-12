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
typedef union _TempAna {
	CAnalysisManager_Super* s;
	CAnalysisManager* a;
}_TempAna;



/**
タグ自体を置換するパターンのテスト。１つのbrigadeに1つの置換対象。
*/
static void test_AnalysisExecutor_HtmlTgaReplace_run1(){
	apr_bucket_alloc_t* ba = apr_bucket_alloc_create(sctest_global_pool);

	//brigade作成 
	const char* tmp[] = {
		"  <input type='' >kkk", "<input >kkkk", SCTEST_BT_EOS
	};
	//printf(tmp[0]);
	AcBool bl = AC_FALSE;
	apr_bucket_brigade* resultBB = NULL;
	apr_table_t* table = NULL;
	apr_bucket_brigade* brigade = NULL;
	CAnalysisExecutor* executor = NULL;
	//
	apr_bucket_brigade** bb = sctest_createBucketBrigade(sctest_global_pool, ba, tmp);

	_TempAna ana;


	//--------------------------------------------------------------------
	//--1つのbrigadeで終了(EOS)するパターン-------------------------------
	//--------------------------------------------------------------------
	//生成
	executor = (CAnalysisExecutor*)CAnalysisExecutor_HtmlTagReplace_new("INPUT", "###", AC_TRUE);
	ana.a = CAnalysisManager_new(executor, sctest_global_pool, ba, table, bb[0]);
	//
	brigade = CAnalysisManager_run(ana.a, NULL);
	A_NOT_NULL(brigade, "run()");
	//CANALYSIS_MANAGER_DEBUG_PRINT(ana.a);
	//ac_printBrigade(brigade);
	{
		const char* expectedBucketList[] = {"  ", "###", "kkk", "###", "kkkk", SCTEST_BT_EOS, NULL};
		A_TRUE(sctest_printNotEquals(expectedBucketList, brigade), "run()");
	}

	apr_brigade_destroy(brigade);
	AcClass_delete(ana.a);
	sctest_destroy_brigades(bb);


	//--------------------------------------------------------------------
	//--対象のタグ以外が存在し、それが置換されないことを確認する--------
	//--------------------------------------------------------------------
	//brigade作成 
	const char* tmp2[] = {
		"  <input type='' >kkk", "<inputs src='' >kkkk<img src=''><input > ", SCTEST_BT_EOS
	};
	bb = sctest_createBucketBrigade(sctest_global_pool, ba, tmp2);
	//生成
	executor = (CAnalysisExecutor*)CAnalysisExecutor_HtmlTagReplace_new("INpUT", "###", AC_TRUE);
	ana.a = CAnalysisManager_new(executor, sctest_global_pool, ba, table, bb[0]);
	//
	brigade = CAnalysisManager_run(ana.a, NULL);
	A_NOT_NULL(brigade, "run()");
	//CANALYSIS_MANAGER_DEBUG_PRINT(ana.a);
	//ac_printBrigade(brigade);
	{
		const char* expectedBucketList[] = {"  ", "###", "kkk", "<inputs src='' >kkkk<img src=''>","###", " ", SCTEST_BT_EOS, NULL};
		A_TRUE(sctest_printNotEquals(expectedBucketList, brigade), "run()");
	}

	apr_brigade_destroy(brigade);
	AcClass_delete(ana.a);
	sctest_destroy_brigades(bb);
};



/**
タグ自体を置換するパターンのテスト。brigadeをまたぐ置換対象。
*/
static void test_AnalysisExecutor_HtmlTgaReplace_between(){
	apr_bucket_alloc_t* ba = apr_bucket_alloc_create(sctest_global_pool);


	AcBool bl = AC_FALSE;
	apr_bucket_brigade* resultBB = NULL;
	apr_table_t* table = NULL;
	apr_bucket_brigade* brigade = NULL;
	CAnalysisExecutor* executor = NULL;
	//
	//brigade作成 
	const char* tmp[] = {
		"  <input type='' >kkk", "<input sr", NULL,
		"c='aaa' >", "<input>", SCTEST_BT_EOS
	};
	apr_bucket_brigade** bb = sctest_createBucketBrigade(sctest_global_pool, ba, tmp);

	_TempAna ana;


	//--------------------------------------------------------------------
	//--1つのbrigadeで終了(EOS)するパターン-------------------------------
	//--------------------------------------------------------------------
	//生成
	executor = (CAnalysisExecutor*)CAnalysisExecutor_HtmlTagReplace_new("INPUT", "###", AC_TRUE);
	ana.a = CAnalysisManager_new(executor, sctest_global_pool, ba, table, bb[0]);
	
	//実行
	brigade = CAnalysisManager_run(ana.a, NULL);
	A_NOT_NULL(brigade, "run()");
	//CANALYSIS_MANAGER_DEBUG_PRINT(ana.a);
	//結果が正しいか検証
	//ac_printBrigade(brigade);
	{
		const char* expectedBucketList[] = {"  ", "###", "kkk", NULL};
		A_TRUE(sctest_printNotEquals(expectedBucketList, brigade), "run()");
	}
	//次のbrigadeを渡す
	brigade = CAnalysisManager_run(ana.a, bb[1]);
	A_NOT_NULL(brigade, "run()");
	//結果が正しいか検証
	printf("---------------------\n");
	//ac_printBrigade(brigade);
	{
		const char* expectedBucketList[] = {"###", "###", SCTEST_BT_EOS, NULL};
		A_TRUE(sctest_printNotEquals(expectedBucketList, brigade), "run()");
	}

	apr_brigade_destroy(brigade);
	AcClass_delete(ana.a);
	sctest_destroy_brigades(bb);
};





/**
タグの後ろに挿入するパターンのテスト。１つのbrigadeに1つの置換対象。
*/
static void test_AnalysisExecutor_HtmlTgaReplace_afterTag(){
	apr_bucket_alloc_t* ba = apr_bucket_alloc_create(sctest_global_pool);

	//brigade作成 
	const char* tmp[] = {
		"  <input type='' >kkk", "<input >kkkk", SCTEST_BT_EOS
	};
	//printf(tmp[0]);
	AcBool bl = AC_FALSE;
	apr_bucket_brigade* resultBB = NULL;
	apr_table_t* table = NULL;
	apr_bucket_brigade* brigade = NULL;
	CAnalysisExecutor* executor = NULL;
	//
	apr_bucket_brigade** bb = sctest_createBucketBrigade(sctest_global_pool, ba, tmp);

	_TempAna ana;


	//--------------------------------------------------------------------
	//--1つのbrigadeで終了(EOS)するパターン（挿入）-------------------------------
	//--------------------------------------------------------------------
	//生成
	executor = (CAnalysisExecutor*)CAnalysisExecutor_HtmlTagReplace_new("INPUT", "###", AC_FALSE);
	ana.a = CAnalysisManager_new(executor, sctest_global_pool, ba, table, bb[0]);
	//
	brigade = CAnalysisManager_run(ana.a, NULL);
	A_NOT_NULL(brigade, "run()");
	//CANALYSIS_MANAGER_DEBUG_PRINT(ana.a);
	//ac_printBrigade(brigade);
	{
		const char* expectedBucketList[] = {"  <input type='' >", "###", "kkk","<input >", "###", "kkkk", SCTEST_BT_EOS, NULL};
		A_TRUE(sctest_printNotEquals(expectedBucketList, brigade), "run()");
	}

	apr_brigade_destroy(brigade);
	AcClass_delete(ana.a);
	sctest_destroy_brigades(bb);



	//--------------------------------------------------------------------
	//--対象のタグ以外が存在し、それが無視されることを確認（挿入）---------
	//--------------------------------------------------------------------
	//brigade作成 
	const char* tmp2[] = {
		"  <input type='' >kkk", "<inputs src='' >kkkk<img src=''><input > ", SCTEST_BT_EOS
	};
	bb = sctest_createBucketBrigade(sctest_global_pool, ba, tmp2);
	//生成
	executor = (CAnalysisExecutor*)CAnalysisExecutor_HtmlTagReplace_new("INPUT", "###", AC_FALSE);
	ana.a = CAnalysisManager_new(executor, sctest_global_pool, ba, table, bb[0]);
	//
	brigade = CAnalysisManager_run(ana.a, NULL);
	A_NOT_NULL(brigade, "run()");
	//CANALYSIS_MANAGER_DEBUG_PRINT(ana.a);
	//ac_printBrigade(brigade);
	{
		const char* expectedBucketList[] = {"  <input type='' >", "###", "kkk", 
			"<inputs src='' >kkkk<img src=''><input >", "###", " ", SCTEST_BT_EOS, NULL};
		A_TRUE(sctest_printNotEquals(expectedBucketList, brigade), "run()");
	}

	apr_brigade_destroy(brigade);
	AcClass_delete(ana.a);
	sctest_destroy_brigades(bb);
};






/**
Endのテスト。＞が見つかる間に途中でEoSになった場合などに正しく処理されるか？
*/
static void test_AnalysisExecutor_HtmlTgaReplace_end(){
	apr_bucket_alloc_t* ba = apr_bucket_alloc_create(sctest_global_pool);

	//brigade作成 
	const char* tmp[] = {
		"  <input type='' kkk", "<input kkkk", SCTEST_BT_EOS
	};
	//printf(tmp[0]);
	AcBool bl = AC_FALSE;
	apr_bucket_brigade* resultBB = NULL;
	apr_table_t* table = NULL;
	apr_bucket_brigade* brigade = NULL;
	CAnalysisExecutor* executor = NULL;
	//
	apr_bucket_brigade** bb = sctest_createBucketBrigade(sctest_global_pool, ba, tmp);

	_TempAna ana;


	//--------------------------------------------------------------------
	//--＞が見つからないパターン-------------------------------
	//--------------------------------------------------------------------
	//生成
	executor = (CAnalysisExecutor*)CAnalysisExecutor_HtmlTagReplace_new("INPUT", "###", AC_FALSE);
	ana.a = CAnalysisManager_new(executor, sctest_global_pool, ba, table, bb[0]);
	//
	brigade = CAnalysisManager_run(ana.a, NULL);
	A_NOT_NULL(brigade, "run()");
	A_TRUE(CAnalysisManager_isEnd(ana.a), "isEnd");
	//CANALYSIS_MANAGER_DEBUG_PRINT(ana.a);
	//ac_printBrigade(brigade);
	{
		const char* expectedBucketList[] = {"  <input type='' kkk", "<input kkkk", SCTEST_BT_EOS, NULL};
		A_TRUE(sctest_printNotEquals(expectedBucketList, brigade), "run()");
	}

	apr_brigade_destroy(brigade);
	AcClass_delete(ana.a);
	sctest_destroy_brigades(bb);



};





void test_analysis_executor_htmltag_replace(){
	test_AnalysisExecutor_HtmlTgaReplace_run1();
	test_AnalysisExecutor_HtmlTgaReplace_between();
	//
	test_AnalysisExecutor_HtmlTgaReplace_afterTag();
	test_AnalysisExecutor_HtmlTgaReplace_end();
}