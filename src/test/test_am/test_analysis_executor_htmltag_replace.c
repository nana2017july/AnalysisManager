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
�^�O���̂�u������p�^�[���̃e�X�g�B�P��brigade��1�̒u���ΏہB
*/
static void test_AnalysisExecutor_HtmlTgaReplace_run1(){
	apr_bucket_alloc_t* ba = apr_bucket_alloc_create(sctest_global_pool);

	//brigade�쐬 
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
	//--1��brigade�ŏI��(EOS)����p�^�[��-------------------------------
	//--------------------------------------------------------------------
	//����
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
	//--�Ώۂ̃^�O�ȊO�����݂��A���ꂪ�u������Ȃ����Ƃ��m�F����--------
	//--------------------------------------------------------------------
	//brigade�쐬 
	const char* tmp2[] = {
		"  <input type='' >kkk", "<inputs src='' >kkkk<img src=''><input > ", SCTEST_BT_EOS
	};
	bb = sctest_createBucketBrigade(sctest_global_pool, ba, tmp2);
	//����
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
�^�O���̂�u������p�^�[���̃e�X�g�Bbrigade���܂����u���ΏہB
*/
static void test_AnalysisExecutor_HtmlTgaReplace_between(){
	apr_bucket_alloc_t* ba = apr_bucket_alloc_create(sctest_global_pool);


	AcBool bl = AC_FALSE;
	apr_bucket_brigade* resultBB = NULL;
	apr_table_t* table = NULL;
	apr_bucket_brigade* brigade = NULL;
	CAnalysisExecutor* executor = NULL;
	//
	//brigade�쐬 
	const char* tmp[] = {
		"  <input type='' >kkk", "<input sr", NULL,
		"c='aaa' >", "<input>", SCTEST_BT_EOS
	};
	apr_bucket_brigade** bb = sctest_createBucketBrigade(sctest_global_pool, ba, tmp);

	_TempAna ana;


	//--------------------------------------------------------------------
	//--1��brigade�ŏI��(EOS)����p�^�[��-------------------------------
	//--------------------------------------------------------------------
	//����
	executor = (CAnalysisExecutor*)CAnalysisExecutor_HtmlTagReplace_new("INPUT", "###", AC_TRUE);
	ana.a = CAnalysisManager_new(executor, sctest_global_pool, ba, table, bb[0]);
	
	//���s
	brigade = CAnalysisManager_run(ana.a, NULL);
	A_NOT_NULL(brigade, "run()");
	//CANALYSIS_MANAGER_DEBUG_PRINT(ana.a);
	//���ʂ�������������
	//ac_printBrigade(brigade);
	{
		const char* expectedBucketList[] = {"  ", "###", "kkk", NULL};
		A_TRUE(sctest_printNotEquals(expectedBucketList, brigade), "run()");
	}
	//����brigade��n��
	brigade = CAnalysisManager_run(ana.a, bb[1]);
	A_NOT_NULL(brigade, "run()");
	//���ʂ�������������
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
�^�O�̌��ɑ}������p�^�[���̃e�X�g�B�P��brigade��1�̒u���ΏہB
*/
static void test_AnalysisExecutor_HtmlTgaReplace_afterTag(){
	apr_bucket_alloc_t* ba = apr_bucket_alloc_create(sctest_global_pool);

	//brigade�쐬 
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
	//--1��brigade�ŏI��(EOS)����p�^�[���i�}���j-------------------------------
	//--------------------------------------------------------------------
	//����
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
	//--�Ώۂ̃^�O�ȊO�����݂��A���ꂪ��������邱�Ƃ��m�F�i�}���j---------
	//--------------------------------------------------------------------
	//brigade�쐬 
	const char* tmp2[] = {
		"  <input type='' >kkk", "<inputs src='' >kkkk<img src=''><input > ", SCTEST_BT_EOS
	};
	bb = sctest_createBucketBrigade(sctest_global_pool, ba, tmp2);
	//����
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
End�̃e�X�g�B����������Ԃɓr����EoS�ɂȂ����ꍇ�Ȃǂɐ�������������邩�H
*/
static void test_AnalysisExecutor_HtmlTgaReplace_end(){
	apr_bucket_alloc_t* ba = apr_bucket_alloc_create(sctest_global_pool);

	//brigade�쐬 
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
	//--����������Ȃ��p�^�[��-------------------------------
	//--------------------------------------------------------------------
	//����
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