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


//テストのためにinclude。通常はincludeしてはいけない。
#include "analysis_parser_.h"
#include "analysis_parser_impl.h"

#include "sctest_utils.h"


typedef union {
	CAnalysisParser* p;
	CAnalysisParser_Super* s;
	CAnalysisParser_CaseInsensitive* c;
} _Temp;

/**
ノーマルテスト。
*/
static void test_CAnalysisParser_CaseInsensitive_match(){
	CAnalysisParser_CaseInsensitive* parser = CAnalysisParser_CaseInsensitive_new("Abc");
	_Temp v;
	v.c = parser;
	//
	CAnalysisParser_accept(v.p, 'a');
	A_EQUALS(AnalysisParserStatus_Matching, v.s->status, "matching");
	CAnalysisParser_accept(v.p, 'b');
	A_EQUALS(AnalysisParserStatus_Matching, v.s->status, "matching");
	CAnalysisParser_accept(v.p, 'c');
	A_EQUALS(AnalysisParserStatus_Matched, v.s->status, "matching");
	A_TRUE(v.s->isEnd,"isEnd");
	A_EQUALS(3, CAnalysisParser_getParsedStrLen(v.p), "len");
	//
	CANALYSIS_PARSER_DEBUG_PRINT(v.p);
	//

	CAnalysisParser_reset(v.p);
	A_EQUALS(0, CAnalysisParser_getParsedStrLen(v.p), "len");
	A_EQUALS(AnalysisParserStatus_Matching, v.s->status, "matching");
	CAnalysisParser_accept(v.p, 'x');
	A_EQUALS(AnalysisParserStatus_Matching, v.s->status, "matching");
	A_TRUE(v.s->isEnd, "isEnd");
	CAnalysisParser_accept(v.p, 'a');
	A_EQUALS(AnalysisParserStatus_Matching, v.s->status, "matching");
	CAnalysisParser_accept(v.p, 'x');
	A_EQUALS(AnalysisParserStatus_Matching, v.s->status, "matching");
	CAnalysisParser_accept(v.p, 'b');
	A_EQUALS(AnalysisParserStatus_Matching, v.s->status, "matching");
	CAnalysisParser_accept(v.p, 'a');
	A_EQUALS(AnalysisParserStatus_Matching, v.s->status, "matching");
	CAnalysisParser_accept(v.p, 'b');
	A_EQUALS(AnalysisParserStatus_Matching, v.s->status, "matching");
	CAnalysisParser_accept(v.p, 'c');
	A_EQUALS(AnalysisParserStatus_Matching, v.s->status, "matched");
	A_TRUE(v.s->isEnd, "isEnd");
	A_EQUALS(7, CAnalysisParser_getParsedStrLen(v.p), "len");
	
	//
	CAnalysisParser_reset(v.p);
	CAnalysisParser_accept(v.p, 'a');
	A_EQUALS(AnalysisParserStatus_Matching, v.s->status, "matching");
	A_FALSE(v.s->isEnd, "isEnd");
	CAnalysisParser_accept(v.p, 'B');
	A_EQUALS(AnalysisParserStatus_Matching, v.s->status, "matching");
	A_FALSE(v.s->isEnd, "isEnd");
	CAnalysisParser_accept(v.p, 'C');
	A_EQUALS(AnalysisParserStatus_Matched, v.s->status, "matching");
	A_TRUE(v.s->isEnd, "isEnd");
	A_EQUALS(3, CAnalysisParser_getParsedStrLen(v.p), "len");
	
	
	//
	AcClass_delete(parser);
};



void test_analysisParserInsensitive(){
	test_CAnalysisParser_CaseInsensitive_match();

}