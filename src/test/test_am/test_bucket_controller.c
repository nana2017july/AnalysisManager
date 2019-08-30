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
#include "analysis_manager_.h"

#include "sctest_utils.h"





//テストのために型定義
typedef union _TempBuf {
	CBucketController_Super* s;
	CBucketController* b;
}_TempBuf;



/**
CBucketControllerのノーマルテスト。
Brigade：１つだけ
・置換しないでEndBufferに達した場合のテスト
・置換を１回だけした場合のテスト
*/
static void test_BucketController_pos() {
	apr_bucket_alloc_t* ba = apr_bucket_alloc_create(sctest_global_pool);

	//brigade作成 
	const char* tmp[] = {
		"abc","de",SCTEST_BT_EOS
	};
	printf(tmp[0]);
	AcBool bl = AC_FALSE;
	apr_bucket_brigade* resultBB = NULL;
	//
	apr_bucket_brigade** bb = sctest_createBucketBrigade(sctest_global_pool, ba, tmp);

	_TempBuf buf;


	//--------------------------------------------------------------------
	//--普通のpos()を使用--------------------------------------------
	//--------------------------------------------------------------------
	//バッファ生成
	buf.s = (CBucketController_Super*)CBucketController_new(sctest_global_pool, ba, bb[0]);
	CBUCKET_CONTROLLER_DEBUG_PRINT(buf.b);
	//
	char c = '\0'; // 
	CBucketController_forward(buf.b);
	A_EQUALS_C('a', CBucketController_pos(buf.b, 0), "pos()");
	A_EQUALS_C('b', CBucketController_pos(buf.b, 1), "pos()");
	A_EQUALS_C('c', CBucketController_pos(buf.b, 2), "pos()");
	A_EQUALS_C('d', CBucketController_pos(buf.b, 3), "pos()");
	A_EQUALS_C('e', CBucketController_pos(buf.b, 4), "pos()");
	A_EQUALS(CBUCKET_CONTROLLER_ERR_POS, CBucketController_pos(buf.b, 5), "pos()");
	//
	printf("-resultBB--------------------\n");
	resultBB = CBucketController_exportModifiedBrigade(buf.b);
	ac_printBrigade(resultBB);
	//
	apr_brigade_destroy(resultBB);
	AcClass_delete(buf.b);
	sctest_destroy_brigades(bb);


	//--------------------------------------------------------------------
	//--forwardとの組み合わせ場合--------------------------------------------
	//--------------------------------------------------------------------
	bb = sctest_createBucketBrigade(sctest_global_pool, ba, tmp);

	//バッファ生成
	buf.s = (CBucketController_Super*)CBucketController_new(sctest_global_pool, ba, bb[0]);
	//CBucketController_addBrigade(buf.b, bb[1]);
	//CBUCKET_CONTROLLER_DEBUG_PRINT(buf.b);
	//
	CBucketController_forward(buf.b);
	A_EQUALS_C('a', CBucketController_pos(buf.b, 0), "pos()");
	//1文字進める
	CBucketController_forward(buf.b);
	A_EQUALS_C('b', CBucketController_pos(buf.b, 0), "pos()");
	A_EQUALS_C('c', CBucketController_pos(buf.b, 1), "pos()");
	A_EQUALS(CBUCKET_CONTROLLER_ERR_POS, CBucketController_pos(buf.b, 4), "pos()");
	//
	printf("-resultBB--------------------\n");
	resultBB = CBucketController_exportModifiedBrigade(buf.b);
	ac_printBrigade(resultBB);
	//
	apr_brigade_destroy(resultBB);
	AcClass_delete(buf.b);
	sctest_destroy_brigades(bb);

};



/**
CBucketControllerのbrigade追加テスト。
*/
static void test_BucketController_addBucket() {
	apr_bucket_alloc_t* ba = apr_bucket_alloc_create(sctest_global_pool);

	//brigade作成 
	const char* tmp[] = {
		"ab","cd",SCTEST_BT_FLUSH,NULL,
		"ef", SCTEST_BT_EOS
	};
	printf(tmp[0]);
	AcBool bl = AC_FALSE;
	apr_bucket_brigade* resultBB = NULL;
	//
	apr_bucket_brigade** bb = sctest_createBucketBrigade(sctest_global_pool, ba, tmp);

	_TempBuf buf;

	//--------------------------------------------------------------------
	//-brigadeの追加---------------------------------------------
	//--------------------------------------------------------------------

	//バッファ生成
	buf.s = (CBucketController_Super*)CBucketController_new(sctest_global_pool, ba, bb[0]);
	//addテスト
	CBucketController_addBrigade(buf.b,bb[1]);
	{
		const char* expectedBucketList[] = { "ab","cd","ef",SCTEST_BT_EOS, NULL };
		A_TRUE(sctest_printNotEquals(expectedBucketList, buf.s->inputBrigade), "exportBrigade");
	}
	//CBucketController_addBrigade(buf.b, bb[1]);
	//CBUCKET_CONTROLLER_DEBUG_PRINT(buf.b);
	//

	//
	//apr_brigade_destroy(resultBB);
	AcClass_delete(buf.b);
	sctest_destroy_brigades(bb);


};



/**
CBucketControllerの置換テスト。
Brigade：１つだけ
・置換しないでEndBufferに達した場合のテスト
・置換を１回だけした場合のテスト
*/
static void test_BucketController_execReplace() {
	apr_bucket_alloc_t* ba = apr_bucket_alloc_create(sctest_global_pool);

	//brigade作成 
	const char* tmp[] = {
		"ab","cd",SCTEST_BT_FLUSH,NULL,
		"ef", SCTEST_BT_EOS
	};
	printf(tmp[0]);
	apr_bucket_brigade* resultBB = NULL;
	AcBool bl = AC_FALSE;
	//
	apr_bucket_brigade** bb = sctest_createBucketBrigade(sctest_global_pool, ba, tmp);

	_TempBuf buf;

	//--------------------------------------------------------------------
	//-1つめのbucketで置換実行---------------------------------------------
	//--------------------------------------------------------------------

	//バッファ生成
	buf.s = (CBucketController_Super*)CBucketController_new(sctest_global_pool, ba, bb[0]);

	//CBucketController_addBrigade(buf.b, bb[1]);
	//CBUCKET_CONTROLLER_DEBUG_PRINT(buf.b);
	//
	char c = '\0'; // 
	bl = CBucketController_forward(buf.b);
	A_TRUE(bl, "forward() return");
	A_EQUALS_C('a', CBucketController_pos(buf.b, 0), "pos()");
	//先頭から1文字を置換
	bl = CBucketController_execReplace(buf.b, 1, "###");
	A_TRUE(bl, "execReplace() return");
	{
		const char* expectedBucketList[] = { "###", NULL };
		A_TRUE(sctest_printNotEquals(expectedBucketList, buf.s->modifiedBrigate), "execReplace");
	}
	{
		const char* expectedBucketList[] = { "b","cd", NULL };
		A_TRUE(sctest_printNotEquals(expectedBucketList, buf.s->inputBrigade), "execReplace");
	}
	//
	//apr_brigade_destroy(resultBB);
	AcClass_delete(buf.b);
	sctest_destroy_brigades(bb);


	//--------------------------------------------------------------------
	//--先頭bucketから、bucketを跨いで置換する場合---------------------------------
	//--------------------------------------------------------------------
	bb = sctest_createBucketBrigade(sctest_global_pool, ba, tmp);

	//バッファ生成
	buf.s = (CBucketController_Super*)CBucketController_new(sctest_global_pool, ba, bb[0]);
	//CBUCKET_CONTROLLER_DEBUG_PRINT(buf.b);
	//
	bl = CBucketController_forward(buf.b);
	bl = CBucketController_forward(buf.b);
	A_TRUE(bl, "forward() return");
	A_EQUALS_C('b', CBucketController_pos(buf.b, 0), "pos()");
	//bucketを跨いで置換
	bl = CBucketController_execReplace(buf.b, 3, "###");
	A_TRUE(bl, "execReplace() return");
	{
		const char* expectedBucketList[] = { "a","###", NULL };
		A_TRUE(sctest_printNotEquals(expectedBucketList, buf.s->modifiedBrigate), "execReplace");
	}
	{
		const char* expectedBucketList[] = {NULL };
		A_TRUE(sctest_printNotEquals(expectedBucketList, buf.s->inputBrigade), "execReplace");
	}
	CBUCKET_CONTROLLER_DEBUG_PRINT(buf.b);
	//
	//
	//apr_brigade_destroy(resultBB);
	AcClass_delete(buf.b);
	sctest_destroy_brigades(bb);



	//--------------------------------------------------------------------
	//--内部に保持しているbucketを超える長さを指定して置換----------------
	//--------------------------------------------------------------------
	bb = sctest_createBucketBrigade(sctest_global_pool, ba, tmp);

	//バッファ生成
	buf.s = (CBucketController_Super*)CBucketController_new(sctest_global_pool, ba, bb[0]);
	//CBUCKET_CONTROLLER_DEBUG_PRINT(buf.b);
	//
	bl = CBucketController_forward(buf.b);
	bl = CBucketController_forward(buf.b);
	A_TRUE(bl, "forward() return");
	A_EQUALS_C('b', CBucketController_pos(buf.b, 0), "pos()");
	//bucketの文字列の長さが足りない長さを指定して置換
	bl = CBucketController_execReplace(buf.b, 4, "###");
	//エラーになることの確認
	A_FALSE(bl, "execReplace() return");
	{
		const char* expectedBucketList[] = { "a", NULL };
		A_TRUE(sctest_printNotEquals(expectedBucketList, buf.s->modifiedBrigate), "execReplace");
	}
	{
		const char* expectedBucketList[] = { "b","cd",NULL };
		A_TRUE(sctest_printNotEquals(expectedBucketList, buf.s->inputBrigade), "execReplace");
	}
	//CBUCKET_CONTROLLER_DEBUG_PRINT(buf.b);
	//置換直後はカレントposが-1になっていることの確認
	A_EQUALS(-1, buf.s->currentBucketStrPos, "check pos==-1 after replace.");

	//長さ0の置換のテスト-------------
	//1文字進める
	CBucketController_forward(buf.b);
	bl = CBucketController_execReplace(buf.b, 0, "###");
	//エラーにならないことの確認
	A_TRUE(bl, "execReplace() return");
	{
		const char* expectedBucketList[] = { "a","###", NULL };
		A_TRUE(sctest_printNotEquals(expectedBucketList, buf.s->modifiedBrigate), "execReplace");
	}
	{
		const char* expectedBucketList[] = { "b","cd",NULL };
		A_TRUE(sctest_printNotEquals(expectedBucketList, buf.s->inputBrigade), "execReplace");
	}
	CBUCKET_CONTROLLER_DEBUG_PRINT(buf.b);

	//
	//apr_brigade_destroy(resultBB);
	AcClass_delete(buf.b);
	sctest_destroy_brigades(bb);
};




static void test_BucketController_forward() {
	apr_bucket_alloc_t* ba = apr_bucket_alloc_create(sctest_global_pool);

	//brigade作成 
	const char* tmp[] = {
		"ab","cd",NULL,
		"ef", SCTEST_BT_EOS
	};
	printf(tmp[0]);
	AcBool bl = AC_FALSE;
	apr_bucket_brigade* resultBB = NULL;
	//
	apr_bucket_brigade** bb = sctest_createBucketBrigade(sctest_global_pool, ba, tmp);

	_TempBuf buf;


	//--------------------------------------------------------------------
	//--forward()をテスト--------------------------------------------
	//--------------------------------------------------------------------
	//バッファ生成
	buf.s = (CBucketController_Super*)CBucketController_new(sctest_global_pool, ba, bb[0]);
	//
	char c = '\0'; // 
	bl = CBucketController_forward(buf.b);
	A_TRUE(bl, "forward()");
	A_EQUALS_C('a', CBucketController_pos(buf.b, 0), "pos()");
	//
	bl = CBucketController_forward(buf.b);
	A_TRUE(bl, "forward()");
	A_EQUALS_C('b', CBucketController_pos(buf.b, 0), "pos()");
	//
	bl = CBucketController_forward(buf.b);
	A_TRUE(bl, "forward()");
	A_EQUALS_C('c', CBucketController_pos(buf.b, 0), "pos()");
	//
	
	bl = CBucketController_forward(buf.b);
	A_TRUE(bl, "forward()");
	A_EQUALS_C('d', CBucketController_pos(buf.b, 0), "pos()");
	//
	bl = CBucketController_forward(buf.b);
	A_FALSE(bl, "forward()");
	A_EQUALS(CBUCKET_CONTROLLER_ERR_POS, CBucketController_pos(buf.b, 0), "pos()");
	A_FALSE(CBucketController_isEOS(buf.b), "isEOS()");
	A_TRUE(CBucketController_isEndBuffer(buf.b), "isEndBuffer()");
	
	//brigadeを追加しても問題なく動作するか？
	CBucketController_addBrigade(buf.b, bb[1]);
	bl = CBucketController_forward(buf.b);
	A_TRUE(bl, "forward()");
	A_EQUALS_C('e', CBucketController_pos(buf.b, 0), "pos()");
	A_FALSE(CBucketController_isEOS(buf.b), "isEOS()");
	A_FALSE(CBucketController_isEndBuffer(buf.b), "isEndBuffer()");
	//
	bl = CBucketController_forward(buf.b);
	A_TRUE(bl, "forward()");
	A_EQUALS_C('f', CBucketController_pos(buf.b, 0), "pos()");
	//
	bl = CBucketController_forward(buf.b);
	A_FALSE(bl, "forward()");
	A_EQUALS(CBUCKET_CONTROLLER_ERR_POS, CBucketController_pos(buf.b, 0), "pos()");
	A_TRUE(CBucketController_isEOS(buf.b), "isEOS()");
	A_TRUE(CBucketController_isEndBuffer(buf.b), "isEndBuffer()");

	//
	//apr_brigade_destroy(resultBB);
	AcClass_delete(buf.b);
	sctest_destroy_brigades(bb);

	//--------------------------------------------------------------------
	//--bucket跨ぎの置換をした後forward()をテスト--------------------------------------------
	//--------------------------------------------------------------------
	bb = sctest_createBucketBrigade(sctest_global_pool, ba, tmp);
	//バッファ生成
	buf.s = (CBucketController_Super*)CBucketController_new(sctest_global_pool, ba, bb[0]);
	//
	bl = CBucketController_forward(buf.b);
	bl = CBucketController_forward(buf.b);
	CBucketController_execReplace(buf.b, 2, "##");
	bl = CBucketController_forward(buf.b);
	A_TRUE(bl, "forward()");
	A_EQUALS_C('d', CBucketController_pos(buf.b, 0), "pos()");
	

	//
	//apr_brigade_destroy(resultBB);
	AcClass_delete(buf.b);
	sctest_destroy_brigades(bb);
};














/**
CBucketControllerのexportテスト。
*/
static void test_BucketController_export(){
	apr_bucket_alloc_t* ba = apr_bucket_alloc_create(sctest_global_pool);

	//brigade作成 
	const char* tmp[] = {
		"ab", "c", SCTEST_BT_FLUSH, SCTEST_BT_EOS
	};
	printf(tmp[0]);
	apr_bucket_brigade* resultBB = NULL;
	AcBool bl = AC_FALSE;
	//
	apr_bucket_brigade** bb = sctest_createBucketBrigade(sctest_global_pool, ba, tmp);

	_TempBuf buf;

	//--------------------------------------------------------------------
	//-1つめのbucketで置換実行---------------------------------------------
	//--------------------------------------------------------------------

	//バッファ生成
	buf.s = (CBucketController_Super*)CBucketController_new(sctest_global_pool, ba, bb[0]);

	//CBucketController_addBrigade(buf.b, bb[1]);
	//CBUCKET_CONTROLLER_DEBUG_PRINT(buf.b);
	//
	char c = '\0'; // 
	bl = CBucketController_forward(buf.b);
	A_EQUALS_C('a', CBucketController_pos(buf.b, 0), "pos()");
	bl = CBucketController_forward(buf.b);
	A_EQUALS_C('b', CBucketController_pos(buf.b, 0), "pos()");
	bl = CBucketController_forward(buf.b);
	A_EQUALS_C('c', CBucketController_pos(buf.b, 0), "pos()");
	bl = CBucketController_forward(buf.b);
	//
	resultBB = apr_brigade_create(sctest_global_pool, ba);
	CBucketController_moveAllInputBucketToModifiedBrigade(buf.b);
	CBucketController_exportModifiedBrigadeToBrigade(buf.b, resultBB);
	ac_printBrigade(resultBB);
	{
		const char* expectedBucketList[] = {"ab", "c", SCTEST_BT_EOS, NULL};
		A_TRUE(sctest_printNotEquals(expectedBucketList, resultBB), "exportModifiedBrigadeToBrigade");
	}
	//
	//apr_brigade_destroy(resultBB);
	AcClass_delete(buf.b);
	sctest_destroy_brigades(bb);
};











/**
CBucketControllerの性能テスト
*/
static void test_BucketController_PerformanceTest() {
	apr_bucket_alloc_t* ba = apr_bucket_alloc_create(sctest_global_pool);

	//brigade作成 
	const char* tmp[] = {
		"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890",
		"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890",
		"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890",
		"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890",
		"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890",
		"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890",
		"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890",
		"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890",
		"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890",
		"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890",
		SCTEST_BT_EOS
	};

	AcBool bl = AC_FALSE;
	apr_bucket_brigade* resultBB = NULL;
	//
	apr_bucket_brigade** bb = sctest_createBucketBrigade(sctest_global_pool, ba, tmp);

	_TempBuf buf;


	//--------------------------------------------------------------------
	//--普通のpos()を使用--------------------------------------------
	//--------------------------------------------------------------------
	//バッファ生成
	buf.s = (CBucketController_Super*)CBucketController_new(sctest_global_pool, ba, bb[0]);
	CBUCKET_CONTROLLER_DEBUG_PRINT(buf.b);
	//
	char c = '\0'; //
	time_t startTime = time(NULL);

	CBucketController_forward(buf.b);
	int i = 0;
	for (i = 0; i < 10000; ++i) {
		int cnt = 0;
		int ret = 0;
		while ((ret = CBucketController_pos(buf.b, cnt)) != CBUCKET_CONTROLLER_ERR_POS) {
			//if (cnt == 900) printf("i=%d, str=%c, cnt=%d\n", i, (char)ret, cnt);
			++cnt;
		}
	}
	printf("performance test: i=%d -------------------------", i);

	time_t endTime = time(NULL);    // 終了時間
	printf("duration = %d , %f MB/sec\n", (int)(endTime - startTime), (float)i/(float)(endTime - startTime)/1024);
	
	//---------------------------------------------
	startTime = time(NULL);
	i = 0;
	const char* p=tmp[0];
	for (i = 0; i < 1000000; ++i) {
		int cnt = 0;
		int ret = 0;
		for (int j = 0; j < 1000; ++j) {
			ret = p[j/10];
			++cnt;
		}
	}
	printf("performance test: i=%d -------------------------", i);

	endTime = time(NULL);    // 終了時間
	printf("duration = %d , %f Kb/sec\n", (int)(endTime - startTime), (float)i / (float)(endTime - startTime));

	//
	AcClass_delete(buf.b);
	sctest_destroy_brigades(bb);
}


/*
テストメイン
*/
void test_bucketController() {
	test_BucketController_pos();
	test_BucketController_addBucket();
	test_BucketController_execReplace();
	test_BucketController_forward();
	test_BucketController_export();
	//性能テスト
	test_BucketController_PerformanceTest();
}

