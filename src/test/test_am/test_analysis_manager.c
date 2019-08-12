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

///Apache�n�̋��ʃC���N���[�h 
#include "apr.h"
#include "apr_general.h"
#include "apr_pools.h"
#include "apr_file_io.h"
#include "apr_tables.h"
#include "apr_strings.h" 
#include "apr_buckets.h"

//
#include "analysis_manager.h"
//�e�X�g�̂��߂�include�B�ʏ��include���Ă͂����Ȃ��B
#include "analysis_manager_.h"
#include "analysis_executor_impl.h"

#include "sctest_utils.h"



//�e�X�g�̂��߂Ɍ^��`
typedef union _TempAna {
	CAnalysisManager_Super* s;
	CAnalysisManager* a;
}_TempAna;



/**
CAnalysisManager_run�̃m�[�}���e�X�g�B�P��brigade��1�̒u���ΏہB
*/
static void test_AnalysisManager_run1() {
	apr_bucket_alloc_t* ba = apr_bucket_alloc_create(sctest_global_pool);

	//brigade�쐬 
	const char* tmp[] = {
		"abc","de",SCTEST_BT_EOS
	};
	printf(tmp[0]);
	AcBool bl = FALSE;
	apr_bucket_brigade* resultBB = NULL;
	apr_table_t* table = NULL;
	//
	apr_bucket_brigade** bb = sctest_createBucketBrigade(sctest_global_pool, ba, tmp);

	_TempAna ana;


	//--------------------------------------------------------------------
	//--1��brigade�ŏI��(EOS)����p�^�[��-------------------------------
	//--------------------------------------------------------------------
	//����
	CAnalysisExecutor* executor = (CAnalysisExecutor*)CAnalysisExecutor_PartialMatch_new("bc", "###");
	ana.a = CAnalysisManager_new(executor, sctest_global_pool, ba, table, bb[0]);
	//
	apr_bucket_brigade* brigade = CAnalysisManager_run(ana.a, NULL);
	A_NOT_NULL(brigade, "run()");
	CANALYSIS_MANAGER_DEBUG_PRINT(ana.a);
	ac_printBrigade(brigade);
	{
		const char* expectedBucketList[] = { "a","###", "de", SCTEST_BT_EOS, NULL };
		A_TRUE(sctest_printNotEquals(expectedBucketList, brigade), "run()");
	}

	apr_brigade_destroy(brigade);
	AcClass_delete(ana.a);
	sctest_destroy_brigades(bb);
};



/**
CAnalysisManager_run�̃m�[�}���e�X�g�B������brigade�ɕ����̒u���ΏہB
*/
static void test_AnalysisManager_run2() {
	apr_bucket_alloc_t* ba = apr_bucket_alloc_create(sctest_global_pool);

	//brigade�쐬 
	const char* tmp[] = {
		"abc","de",NULL,
		"fb","cg","bcbcggg", SCTEST_BT_EOS
	};
	printf(tmp[0]);
	AcBool bl = FALSE;
	apr_table_t* table = NULL;
	apr_bucket_brigade* resultBB = NULL;
	//
	apr_bucket_brigade** bb = sctest_createBucketBrigade(sctest_global_pool, ba, tmp);

	_TempAna ana;


	//--------------------------------------------------------------------
	//--������brigade�ŏI��(EOS)����p�^�[��-------------------------------
	//--------------------------------------------------------------------
	//����
	CAnalysisExecutor* executor = (CAnalysisExecutor*)CAnalysisExecutor_PartialMatch_new("bc", "###");
	CANALYSIS_EXECUTOR_DEBUG_PRINT(executor);
	ana.a = CAnalysisManager_new(executor, sctest_global_pool, ba, table, bb[0]);
	//
	apr_bucket_brigade* brigade = CAnalysisManager_run(ana.a, NULL);
	A_NOT_NULL(brigade, "run()");
	CANALYSIS_MANAGER_DEBUG_PRINT(ana.a);
	//ac_printBrigade(brigade);
	{
		const char* expectedBucketList[] = { "a","###", "de", NULL };
		A_TRUE(sctest_printNotEquals(expectedBucketList, brigade), "run()");
	}
	//
	apr_brigade_destroy(brigade);

	//2�ڂ�brigade����
	brigade = CAnalysisManager_run(ana.a, bb[1]);
	A_NOT_NULL(brigade, "run()");
	CANALYSIS_MANAGER_DEBUG_PRINT(ana.a);
	//ac_printBrigade(brigade);
	{
		const char* expectedBucketList[] = { "f","###", "g", "###", "###", "ggg", SCTEST_BT_EOS, NULL };
		A_TRUE(sctest_printNotEquals(expectedBucketList, brigade), "run()");
	}


	apr_brigade_destroy(brigade);
	AcClass_delete(ana.a);
	sctest_destroy_brigades(bb);
};






/**
CAnalysisManager�̐��\�e�X�g�B
*/
static void test_AnalysisManager_PerformanceTest() {
	apr_bucket_alloc_t* ba = apr_bucket_alloc_create(sctest_global_pool);

	//brigade�쐬 
	const char* tmp[] = {
		"12345678901234abc89012345678901234567890123456789012345678901234567890123456789012345678901234567890",
		"123456789012345abc9012345678901234567890123456789012345678901234567890123456789012345678901234567890",
		"123456789012345abc9012345678901234567890123456789012345678901234567890123456789012345678901234567890",
		"123456789012345abc9012345678901234567890123456789012345678901234567890123456789012345678901234567890",
		"123456789012345abc9012345678901234567890123456789012345678901234567890123456789012345678901234567890",
		"123456789012345abc9012345678901234567890123456789012345678901234567890123456789012345678901234567890",
		"123456789012345abc9012345678901234567890123456789012345678901234567890123456789012345678901234567890",
		"123456789012345abc9012345678901234567890123456789012345678901234567890123456789012345678901234567890",
		"123456789012345abc9012345678901234567890123456789012345678901234567890123456789012345678901234567890",
		"123456789012345abc9012345678901234567890123456789012345678901234567890123456789012345678901234567890",
		SCTEST_BT_EOS
	};
	printf(tmp[0]);
	AcBool bl = FALSE;
	apr_table_t* table = NULL;
	apr_bucket_brigade* resultBB = NULL;
	//
	apr_bucket_brigade** bb = NULL;

	_TempAna ana;


	//--------------------------------------------------------------------
	//--���x����-------------------------------
	//--------------------------------------------------------------------
	//����
	time_t startTime = time(NULL);

	int i = 0;
	for (i = 0; i < 100000; ++i) {
		bb = sctest_createBucketBrigade(sctest_global_pool, ba, tmp);
		CAnalysisExecutor* executor = (CAnalysisExecutor*)CAnalysisExecutor_PartialMatch_new("abc", "###");
		ana.a = CAnalysisManager_new(executor, sctest_global_pool, ba, table, bb[0]);
		//
		apr_bucket_brigade* brigade = CAnalysisManager_run(ana.a, NULL);

		if(i==999) ac_printBrigade(brigade);
		//
		apr_brigade_destroy(brigade);
		AcClass_delete(ana.a);
		sctest_destroy_brigades(bb);
	}
	printf("performance test: i=%d ---[CAnalysisManager]----------------------", i);

	time_t endTime = time(NULL);    // �I������
	printf("duration = %d , %f MB/sec\n", (int)(endTime - startTime), (float)i / (float)(endTime - startTime) / 1024.0f);

};


/**
CAnalysisManager�̐��\�e�X�g�B�������Ȃ�Executor���Z�b�g�����Ƃ��ɂǂ̂��炢�̎��Ԃ������邩�𒲂ׂ�B���͂��邽�߁B
*/
static void test_AnalysisManager_PerformanceTest_NoExecutor(){
	apr_bucket_alloc_t* ba = apr_bucket_alloc_create(sctest_global_pool);

	//brigade�쐬 
	const char* tmp[] = {
		"12345678901234abc89012345678901234567890123456789012345678901234567890123456789012345678901234567890",
		"123456789012345abc9012345678901234567890123456789012345678901234567890123456789012345678901234567890",
		"123456789012345abc9012345678901234567890123456789012345678901234567890123456789012345678901234567890",
		"123456789012345abc9012345678901234567890123456789012345678901234567890123456789012345678901234567890",
		"123456789012345abc9012345678901234567890123456789012345678901234567890123456789012345678901234567890",
		"123456789012345abc9012345678901234567890123456789012345678901234567890123456789012345678901234567890",
		"123456789012345abc9012345678901234567890123456789012345678901234567890123456789012345678901234567890",
		"123456789012345abc9012345678901234567890123456789012345678901234567890123456789012345678901234567890",
		"123456789012345abc9012345678901234567890123456789012345678901234567890123456789012345678901234567890",
		"123456789012345abc9012345678901234567890123456789012345678901234567890123456789012345678901234567890",
		SCTEST_BT_EOS
	};
	printf(tmp[0]);
	AcBool bl = FALSE;
	apr_table_t* table = NULL;
	apr_bucket_brigade* resultBB = NULL;
	//
	apr_bucket_brigade** bb = NULL;

	_TempAna ana;


	//--------------------------------------------------------------------
	//--���x����-------------------------------
	//--------------------------------------------------------------------
	//����
	time_t startTime = time(NULL);

	int i = 0;
	for(i = 0; i < 100000; ++i){
		bb = sctest_createBucketBrigade(sctest_global_pool, ba, tmp);
		CAnalysisExecutor* executor = (CAnalysisExecutor*)CAnalysisExecutor_DoNothing_new();
		ana.a = CAnalysisManager_new(executor, sctest_global_pool, ba, table, bb[0]);
		//
		apr_bucket_brigade* brigade = CAnalysisManager_run(ana.a, NULL);

		//if(i == 999) ac_printBrigade(brigade);
		//
		apr_brigade_destroy(brigade);
		AcClass_delete(ana.a);
		sctest_destroy_brigades(bb);
	}
	printf("performance test: i=%d ---[CAnalysisManager with CAnalysisExecutor_DoNothing]----------------------", i);

	time_t endTime = time(NULL);    // �I������
	printf("duration = %d , %f MB/sec\n", (int)(endTime - startTime), (float)i / (float)(endTime - startTime) / 1024.0f);

};




void test_analysis_manager() {
	test_AnalysisManager_run1(); 
	test_AnalysisManager_run2();
	//���\�e�X�g
	//test_AnalysisManager_PerformanceTest();
	//test_AnalysisManager_PerformanceTest_NoExecutor();
}