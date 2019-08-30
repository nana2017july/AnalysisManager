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
#include "analysis_manager_.h"

#include "sctest_utils.h"





//�e�X�g�̂��߂Ɍ^��`
typedef union _TempBuf {
	CBucketController_Super* s;
	CBucketController* b;
}_TempBuf;



/**
CBucketController�̃m�[�}���e�X�g�B
Brigade�F�P����
�E�u�����Ȃ���EndBuffer�ɒB�����ꍇ�̃e�X�g
�E�u�����P�񂾂������ꍇ�̃e�X�g
*/
static void test_BucketController_pos() {
	apr_bucket_alloc_t* ba = apr_bucket_alloc_create(sctest_global_pool);

	//brigade�쐬 
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
	//--���ʂ�pos()���g�p--------------------------------------------
	//--------------------------------------------------------------------
	//�o�b�t�@����
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
	//--forward�Ƃ̑g�ݍ��킹�ꍇ--------------------------------------------
	//--------------------------------------------------------------------
	bb = sctest_createBucketBrigade(sctest_global_pool, ba, tmp);

	//�o�b�t�@����
	buf.s = (CBucketController_Super*)CBucketController_new(sctest_global_pool, ba, bb[0]);
	//CBucketController_addBrigade(buf.b, bb[1]);
	//CBUCKET_CONTROLLER_DEBUG_PRINT(buf.b);
	//
	CBucketController_forward(buf.b);
	A_EQUALS_C('a', CBucketController_pos(buf.b, 0), "pos()");
	//1�����i�߂�
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
CBucketController��brigade�ǉ��e�X�g�B
*/
static void test_BucketController_addBucket() {
	apr_bucket_alloc_t* ba = apr_bucket_alloc_create(sctest_global_pool);

	//brigade�쐬 
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
	//-brigade�̒ǉ�---------------------------------------------
	//--------------------------------------------------------------------

	//�o�b�t�@����
	buf.s = (CBucketController_Super*)CBucketController_new(sctest_global_pool, ba, bb[0]);
	//add�e�X�g
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
CBucketController�̒u���e�X�g�B
Brigade�F�P����
�E�u�����Ȃ���EndBuffer�ɒB�����ꍇ�̃e�X�g
�E�u�����P�񂾂������ꍇ�̃e�X�g
*/
static void test_BucketController_execReplace() {
	apr_bucket_alloc_t* ba = apr_bucket_alloc_create(sctest_global_pool);

	//brigade�쐬 
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
	//-1�߂�bucket�Œu�����s---------------------------------------------
	//--------------------------------------------------------------------

	//�o�b�t�@����
	buf.s = (CBucketController_Super*)CBucketController_new(sctest_global_pool, ba, bb[0]);

	//CBucketController_addBrigade(buf.b, bb[1]);
	//CBUCKET_CONTROLLER_DEBUG_PRINT(buf.b);
	//
	char c = '\0'; // 
	bl = CBucketController_forward(buf.b);
	A_TRUE(bl, "forward() return");
	A_EQUALS_C('a', CBucketController_pos(buf.b, 0), "pos()");
	//�擪����1������u��
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
	//--�擪bucket����Abucket���ׂ��Œu������ꍇ---------------------------------
	//--------------------------------------------------------------------
	bb = sctest_createBucketBrigade(sctest_global_pool, ba, tmp);

	//�o�b�t�@����
	buf.s = (CBucketController_Super*)CBucketController_new(sctest_global_pool, ba, bb[0]);
	//CBUCKET_CONTROLLER_DEBUG_PRINT(buf.b);
	//
	bl = CBucketController_forward(buf.b);
	bl = CBucketController_forward(buf.b);
	A_TRUE(bl, "forward() return");
	A_EQUALS_C('b', CBucketController_pos(buf.b, 0), "pos()");
	//bucket���ׂ��Œu��
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
	//--�����ɕێ����Ă���bucket�𒴂��钷�����w�肵�Ēu��----------------
	//--------------------------------------------------------------------
	bb = sctest_createBucketBrigade(sctest_global_pool, ba, tmp);

	//�o�b�t�@����
	buf.s = (CBucketController_Super*)CBucketController_new(sctest_global_pool, ba, bb[0]);
	//CBUCKET_CONTROLLER_DEBUG_PRINT(buf.b);
	//
	bl = CBucketController_forward(buf.b);
	bl = CBucketController_forward(buf.b);
	A_TRUE(bl, "forward() return");
	A_EQUALS_C('b', CBucketController_pos(buf.b, 0), "pos()");
	//bucket�̕�����̒���������Ȃ��������w�肵�Ēu��
	bl = CBucketController_execReplace(buf.b, 4, "###");
	//�G���[�ɂȂ邱�Ƃ̊m�F
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
	//�u������̓J�����gpos��-1�ɂȂ��Ă��邱�Ƃ̊m�F
	A_EQUALS(-1, buf.s->currentBucketStrPos, "check pos==-1 after replace.");

	//����0�̒u���̃e�X�g-------------
	//1�����i�߂�
	CBucketController_forward(buf.b);
	bl = CBucketController_execReplace(buf.b, 0, "###");
	//�G���[�ɂȂ�Ȃ����Ƃ̊m�F
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

	//brigade�쐬 
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
	//--forward()���e�X�g--------------------------------------------
	//--------------------------------------------------------------------
	//�o�b�t�@����
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
	
	//brigade��ǉ����Ă����Ȃ����삷�邩�H
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
	//--bucket�ׂ��̒u����������forward()���e�X�g--------------------------------------------
	//--------------------------------------------------------------------
	bb = sctest_createBucketBrigade(sctest_global_pool, ba, tmp);
	//�o�b�t�@����
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
CBucketController��export�e�X�g�B
*/
static void test_BucketController_export(){
	apr_bucket_alloc_t* ba = apr_bucket_alloc_create(sctest_global_pool);

	//brigade�쐬 
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
	//-1�߂�bucket�Œu�����s---------------------------------------------
	//--------------------------------------------------------------------

	//�o�b�t�@����
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
CBucketController�̐��\�e�X�g
*/
static void test_BucketController_PerformanceTest() {
	apr_bucket_alloc_t* ba = apr_bucket_alloc_create(sctest_global_pool);

	//brigade�쐬 
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
	//--���ʂ�pos()���g�p--------------------------------------------
	//--------------------------------------------------------------------
	//�o�b�t�@����
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

	time_t endTime = time(NULL);    // �I������
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

	endTime = time(NULL);    // �I������
	printf("duration = %d , %f Kb/sec\n", (int)(endTime - startTime), (float)i / (float)(endTime - startTime));

	//
	AcClass_delete(buf.b);
	sctest_destroy_brigades(bb);
}


/*
�e�X�g���C��
*/
void test_bucketController() {
	test_BucketController_pos();
	test_BucketController_addBucket();
	test_BucketController_execReplace();
	test_BucketController_forward();
	test_BucketController_export();
	//���\�e�X�g
	test_BucketController_PerformanceTest();
}

