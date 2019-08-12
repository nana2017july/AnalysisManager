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

#ifdef _WIN32 
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif //_WIN32
//
#include <stdlib.h>
#include <stdio.h>

//
#include "ac_bucket_utils.h"

///Apache系の共通インクルード 
#include "apr.h"
#include "apr_general.h"
#include "apr_pools.h"

//
#include "sctest_utils.h"


//void test_bucketBuffer();
void test_bucketController();
void test_analysis_manager();
void test_analysis_executor_htmltag_replace();
void test_analysisParserInsensitive();
void test_analysisParserSplit();
void test_analysisParserHttpHeader();
void test_analysis_executor_Multipart();
void test_BucketUtils();


int main()
{
	//aprの初期化などの処理を呼び出す
	sctest_initialize();


	//
	//----test_bucketBuffer();
	test_bucketController();
	test_analysis_manager();
	test_analysis_executor_htmltag_replace();
	test_analysisParserInsensitive();
	test_analysisParserSplit();
	test_analysisParserHttpHeader();
	test_analysis_executor_Multipart();
	test_BucketUtils();

	//テスト結果出力
	sctest_printResult();

	sctest_destoroy();

	//
#ifdef _WIN32 
	_CrtDumpMemoryLeaks();
#endif
}