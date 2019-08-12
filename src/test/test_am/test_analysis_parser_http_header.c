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


//�e�X�g�̂��߂�include�B�ʏ��include���Ă͂����Ȃ��B
#include "analysis_parser_.h"
#include "analysis_parser_impl.h"

#include "sctest_utils.h"


typedef union {
	CAnalysisParser* p;
	CAnalysisParser_Super* s;
	CAnalysisParser_HttpHeader* c;
} _Temp;

/**
�m�[�}���e�X�g�B
*/
static void test_CAnalysisParser_HttpHeader_match(){
	CAnalysisParser_HttpHeader* parser = CAnalysisParser_HttpHeader_new("C-D:", 200);
	_Temp v;
	v.c = parser;
	const char* str;
	size_t len;

	//�w�b�_�̕����� "d-d:12\r\n"
	CAnalysisParser_accept(v.p, 'c');
	A_EQUALS(AnalysisParserStatus_Matching, v.s->status, "matching");
	A_FALSE(v.s->isEnd, "isEnd");
	CAnalysisParser_accept(v.p, '-');
	A_EQUALS(AnalysisParserStatus_Matching, v.s->status, "matching");
	A_FALSE(v.s->isEnd, "isEnd");
	CAnalysisParser_accept(v.p, 'd');
	A_EQUALS(AnalysisParserStatus_Matching, v.s->status, "matching");
	A_FALSE(v.s->isEnd, "isEnd");
	CAnalysisParser_accept(v.p, ':');
	A_EQUALS(AnalysisParserStatus_Matching, v.s->status, "matching");
	A_FALSE(v.s->isEnd, "isEnd");
	CAnalysisParser_accept(v.p, '1');
	A_EQUALS(AnalysisParserStatus_Matching, v.s->status, "matching");
	A_FALSE(v.s->isEnd, "isEnd");
	CAnalysisParser_accept(v.p, '2');
	A_EQUALS(AnalysisParserStatus_Matching, v.s->status, "matching");
	A_FALSE(v.s->isEnd, "isEnd");
	CAnalysisParser_accept(v.p, '\r');
	A_EQUALS(AnalysisParserStatus_Matching, v.s->status, "matching");
	A_FALSE(v.s->isEnd, "isEnd");
	CAnalysisParser_accept(v.p, '\n');
	A_EQUALS(AnalysisParserStatus_Matched, v.s->status, "matched");
	A_TRUE(CAnalysisParser_isMatched(v.p), "isMatched");
	A_TRUE(v.s->isEnd, "isEnd");
	A_EQUALS(8, CAnalysisParser_getParsedStrLen(v.p), "len");
	//�w�b�_�l�̎擾
	CAnalysisParser_HttpHeader_getValue(v.c, &str, &len);
	A_EQUALS_NSTR("12\r\n", str, len, "getStockStr");
	A_EQUALS(4, len, "getStockStr");
	//
	//CANALYSIS_PARSER_DEBUG_PRINT(v.p);

	//������
	CAnalysisParser_reset(v.p);
	A_EQUALS(AnalysisParserStatus_Matching, v.s->status, "matching");
	A_FALSE(v.s->isEnd, "isEnd");


	//�w�b�_�̕����� "d-dc:1\r\n"
	CAnalysisParser_accept(v.p, 'c');
	A_EQUALS(AnalysisParserStatus_Matching, v.s->status, "matching");
	CAnalysisParser_accept(v.p, '-');
	A_EQUALS(AnalysisParserStatus_Matching, v.s->status, "matching");
	CAnalysisParser_accept(v.p, 'd');
	A_EQUALS(AnalysisParserStatus_Matching, v.s->status, "matching");
	CAnalysisParser_accept(v.p, 'c');
	A_EQUALS(AnalysisParserStatus_Matching, v.s->status, "matching");
	CAnalysisParser_accept(v.p, ':');
	A_EQUALS(AnalysisParserStatus_Matching, v.s->status, "matching");
	CAnalysisParser_accept(v.p, '1');
	A_EQUALS(AnalysisParserStatus_Matching, v.s->status, "matching");
	CAnalysisParser_accept(v.p, '\r');
	A_EQUALS(AnalysisParserStatus_Matching, v.s->status, "matching");
	CAnalysisParser_accept(v.p, '\n');
	A_EQUALS(AnalysisParserStatus_Matching, v.s->status, "matching");
	A_FALSE(CAnalysisParser_isMatched(v.p), "isMatched");
	A_TRUE(v.s->isEnd, "isEnd");
	A_EQUALS(8, CAnalysisParser_getParsedStrLen(v.p), "len");
	//�w�b�_�l�̎擾
	CAnalysisParser_HttpHeader_getValue(v.c, &str, &len);
	A_EQUALS(0, len, "getStockStr");
	//
	//CANALYSIS_PARSER_DEBUG_PRINT(v.p);

	//
	AcClass_delete(parser);
};




/**
������n�e�X�g�B
*/
static void test_CAnalysisParser_HttpHeader_subnormal(){
	CAnalysisParser_HttpHeader* parser = CAnalysisParser_HttpHeader_new("C-D:", 200);
	_Temp v;
	v.c = parser;
	const char* str;
	size_t len;

	//�w�b�_�̕����� "\r\n"
	CAnalysisParser_accept(v.p, '\r');
	A_EQUALS(AnalysisParserStatus_Matching, v.s->status, "matching");
	A_FALSE(v.s->isEnd, "isEnd");
	CAnalysisParser_accept(v.p, '\n');
	A_EQUALS(AnalysisParserStatus_Matching, v.s->status, "matching");
	A_TRUE(v.s->isEnd, "isEnd");
	A_FALSE(CAnalysisParser_isMatched(v.p), "isMatched");
	A_EQUALS(2, CAnalysisParser_getParsedStrLen(v.p), "len");
	//�w�b�_�l�̎擾(�}�b�`���Ȃ������̂Œ�����0)
	CAnalysisParser_HttpHeader_getValue(v.c, &str, &len);
	A_EQUALS(0, len, "getStockStr");
	//
	//CANALYSIS_PARSER_DEBUG_PRINT(v.p);

	//
	AcClass_delete(parser);
};



void test_analysisParserHttpHeader(){
	test_CAnalysisParser_HttpHeader_match();
	test_CAnalysisParser_HttpHeader_subnormal();

	//
}