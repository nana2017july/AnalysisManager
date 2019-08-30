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


//テストのためにinclude。通常はincludeしてはいけない。
#include "analysis_parser_.h"
#include "analysis_parser_impl.h"

#include "sctest_utils.h"


typedef union {
	CAnalysisParser* p;
	CAnalysisParser_Super* s;
	CAnalysisParser_Split* c;
} _Temp;


/**
ノーマルテスト。
*/
static void test_CAnalysisParser_Split_match(){
	CAnalysisParser_Split* parser = CAnalysisParser_Split_new("\r\n", 10);
	_Temp v;
	v.c = parser;
	const char* str;
	size_t len;
	//
	CAnalysisParser_accept(v.p, 'a');
	A_EQUALS(AnalysisParserStatus_Searching, v.s->status, "searching");
	CAnalysisParser_accept(v.p, '\r');
	A_EQUALS(AnalysisParserStatus_Matching, v.s->status, "matching");
	CAnalysisParser_accept(v.p, '\n');
	A_EQUALS(AnalysisParserStatus_Matched, v.s->status, "matching");
	A_TRUE(v.s->isEnd, "isEnd");
	A_EQUALS(3, CAnalysisParser_getParsedStrLen(v.p), "len");
	CAnalysisParser_Split_getStockStr(v.c, &str, &len);
	A_EQUALS_STR("a\r\n", str, "getStockStr");
	A_EQUALS(3, len, "getStockStr");
	//
	//CANALYSIS_PARSER_DEBUG_PRINT(v.p);
	//

	CAnalysisParser_reset(v.p);
	A_EQUALS(0, CAnalysisParser_getParsedStrLen(v.p), "len");
	A_EQUALS(AnalysisParserStatus_Searching, v.s->status, "searching");
	CAnalysisParser_accept(v.p, 'x');
	A_EQUALS(AnalysisParserStatus_Searching, v.s->status, "searching");
	CAnalysisParser_accept(v.p, ':');
	A_EQUALS(AnalysisParserStatus_Searching, v.s->status, "searching");
	CAnalysisParser_accept(v.p, '1');
	A_EQUALS(AnalysisParserStatus_Searching, v.s->status, "searching");
	CAnalysisParser_accept(v.p, 'b');
	A_EQUALS(AnalysisParserStatus_Searching, v.s->status, "searching");
	CAnalysisParser_accept(v.p, '\r');
	A_EQUALS(AnalysisParserStatus_Matching, v.s->status, "matching");
	CAnalysisParser_accept(v.p, '\n');
	A_EQUALS(AnalysisParserStatus_Matched, v.s->status, "matched");
	A_TRUE(v.s->isEnd, "isEnd");
	CAnalysisParser_accept(v.p, 'c');
	A_EQUALS(AnalysisParserStatus_Matched, v.s->status, "matched");
	A_TRUE(v.s->isEnd, "isEnd");
	A_EQUALS(7, CAnalysisParser_getParsedStrLen(v.p), "len");

	//
	AcClass_delete(parser);
};



/**
ストックのバッファを超えた場合のテスト。
*/
static void test_CAnalysisParser_Split_overStockBuffer(){
	CAnalysisParser_Split* parser = CAnalysisParser_Split_new("\r\n", 3);
	_Temp v;
	v.c = parser;
	const char* str;
	size_t len;
	//
	CAnalysisParser_accept(v.p, 'a');
	A_EQUALS(AnalysisParserStatus_Searching, v.s->status, "searching");
	CAnalysisParser_accept(v.p, '\r');
	A_EQUALS(AnalysisParserStatus_Matching, v.s->status, "matching");
	CAnalysisParser_accept(v.p, '\n');
	A_EQUALS(AnalysisParserStatus_Matched, v.s->status, "matching");
	A_TRUE(v.s->isEnd, "isEnd");
	A_EQUALS(3, CAnalysisParser_getParsedStrLen(v.p), "len");
	CAnalysisParser_Split_getStockStr(v.c, &str, &len);
	A_EQUALS_STR("a\r\n", str, "getStockStr");
	A_EQUALS(3, len, "getStockStr");
	//
	//CANALYSIS_PARSER_DEBUG_PRINT(v.p);
	
	//バッファサイズ3に対して、4文字が投入された場合
	CAnalysisParser_reset(v.p);
	A_EQUALS(0, CAnalysisParser_getParsedStrLen(v.p), "len");
	A_EQUALS(AnalysisParserStatus_Searching, v.s->status, "searching");
	CAnalysisParser_accept(v.p, '1');
	A_EQUALS(AnalysisParserStatus_Searching, v.s->status, "searching");
	CAnalysisParser_accept(v.p, '2');
	A_EQUALS(AnalysisParserStatus_Searching, v.s->status, "searching");
	CAnalysisParser_accept(v.p, '\r');
	A_EQUALS(AnalysisParserStatus_Matching, v.s->status, "matching");
	A_FALSE(v.s->isEnd, "isEnd");
	CAnalysisParser_accept(v.p, '\n');
	A_EQUALS(AnalysisParserStatus_Matched, v.s->status, "matched");
	A_TRUE(v.s->isEnd, "isEnd");
	A_EQUALS(4, CAnalysisParser_getParsedStrLen(v.p), "len");
	CAnalysisParser_Split_getStockStr(v.c, &str, &len);
	A_EQUALS_NSTR("12\r", str, 3, "getStockStr");
	A_EQUALS(3, len, "getStockStr");

	//
	AcClass_delete(parser);
};




void test_analysisParserSplit(){
	test_CAnalysisParser_Split_match();
	test_CAnalysisParser_Split_overStockBuffer();
}