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

#include "sctest_utils.h"






//other test---------------------------------------------------------------------------------


static void test_ac_toLowerCase(){
	char str[] = "ABC\";abc!;:[@XZY";
	//
	ac_toLowerCase(str);
	A_EQUALS_STR("abc\";abc!;:[@xzy", str, "");

	//
	*str = '\0';
	ac_toLowerCase(str);
	A_EQUALS_STR("", str, "");

}


static void test_ac_unescapeChar(){
	char str[] = "ABC\\\";a\\b\\\\c!;:[@XZY";
	ac_unescapeChar(str, '\\');
	A_EQUALS_STR("ABC\";ab\\c!;:[@XZY", str, "");
	//
	*str = '\0';
	ac_unescapeChar(str, '\\');
	A_EQUALS_STR("", str, "");
}


static void test_ac_splitKeyValueArrayWithQuote(){
	char** ret = ac_splitKeyValueArrayWithQuote("a=1;b;c=3", ";", AC_TRUE, 10);
	//
	A_EQUALS_STR("a", ret[0], "");
	A_EQUALS_STR("1", ret[1], "");
	A_EQUALS_STR("b", ret[2], "");
	A_EQUALS_STR("", ret[3], "");
	A_EQUALS_STR("c", ret[4], "");
	A_EQUALS_STR("3", ret[5], "");
	A_NULL(ret[6], "");

	free(ret);


	//-----------------------------
	ret = ac_splitKeyValueArrayWithQuote(" a=\";;; 1;  \";Mozilla/4.0 (Compatible; MSIE 6.0;);", ";()", AC_TRUE, 10);
	//
	A_EQUALS_STR(" a", ret[0], "");
	A_EQUALS_STR("\";;; 1;  \"", ret[1], "");
	A_EQUALS_STR("Mozilla/4.0 ", ret[2], "");
	A_EQUALS_STR("", ret[3], "");
	A_EQUALS_STR("Compatible", ret[4], "");
	A_EQUALS_STR("", ret[5], "");
	A_EQUALS_STR(" MSIE 6.0", ret[6], "");
	A_EQUALS_STR("", ret[7], "");
	A_NULL(ret[8], "");

	free(ret);




	//クォートの中にエスケープ-----------------------------
	ret = ac_splitKeyValueArrayWithQuote(" a=\";;\\\"; 1;  \";Mozilla/4.0 ", ";()", AC_TRUE, 10);
	//
	A_EQUALS_STR(" a", ret[0], "");
	A_EQUALS_STR("\";;\\\"; 1;  \"", ret[1], "");
	A_EQUALS_STR("Mozilla/4.0 ", ret[2], "");
	A_EQUALS_STR("", ret[3], "");
	A_NULL(ret[4], "");

	free(ret);
}




static void test_ac_splitHttpHeaderValue(){
	char** ret = ac_splitHttpHeaderValue("a = 1 ;b;c=\"3\"", ";", AC_TRUE, 10);
	//
	A_EQUALS_STR("a", ret[0], "");
	A_EQUALS_STR("1", ret[1], "");
	A_EQUALS_STR("b", ret[2], "");
	A_EQUALS_STR("", ret[3], "");
	A_EQUALS_STR("c", ret[4], "");
	A_EQUALS_STR("3", ret[5], "");
	A_NULL(ret[6], "");

	free(ret);


	//-----------------------------
	ret = ac_splitHttpHeaderValue(" a=\";;; 1;  \" ; Mozilla/4.0 (Compatible; MSIE 6.0;);", ";()", AC_TRUE, 10);
	//
	A_EQUALS_STR("a", ret[0], "");
	A_EQUALS_STR(";;; 1;  ", ret[1], "");
	A_EQUALS_STR("Mozilla/4.0", ret[2], "");
	A_EQUALS_STR("", ret[3], "");
	A_EQUALS_STR("Compatible", ret[4], "");
	A_EQUALS_STR("", ret[5], "");
	A_EQUALS_STR("MSIE 6.0", ret[6], "");
	A_EQUALS_STR("", ret[7], "");
	A_NULL(ret[8], "");

	free(ret);




	//クォートの中にエスケープ-----------------------------
	ret = ac_splitHttpHeaderValue(" a=\";;\\\"; 1;  \";Mozilla/4.0 = \"", ";()", AC_TRUE, 10);
	//
	A_EQUALS_STR("a", ret[0], "");
	A_EQUALS_STR(";;\"; 1;  ", ret[1], "");
	A_EQUALS_STR("Mozilla/4.0", ret[2], "");
	A_EQUALS_STR("", ret[3], "");
	A_NULL(ret[4], "");

	free(ret);


	//空文字の場合-----------------------------
	ret = ac_splitHttpHeaderValue("", ";()", AC_TRUE, 10);
	//
	A_NULL(ret[0], "");

	free(ret);
}

static void test_ac_split2(){
	char** ret = ac_split2("a=1;b;c=3", '=', ';', 3);
	//
	A_EQUALS_STR(ret[0], "a", "");
	A_EQUALS_STR(ret[1], "1", "");
	A_EQUALS_STR(ret[2], "b", "");
	A_EQUALS_STR(ret[3], "", "");
	A_EQUALS_STR(ret[4], "c", "");
	A_EQUALS_STR(ret[5], "3", "");
	A_NULL(ret[6], "");

	free((void*)ret);


	//
	ret = ac_split2("a=1;b;c=3", '=', ';', 2);
	//
	A_EQUALS_STR(ret[0], "a", "");
	A_EQUALS_STR(ret[1], "1", "");
	A_EQUALS_STR(ret[2], "b", "");
	A_EQUALS_STR(ret[3], "", "");
	A_NULL(ret[4], "");

	free((void*)ret);


	//
	ret = ac_split2("a=1;b;", '=', ';', 4);
	//
	A_EQUALS_STR(ret[0], "a", "");
	A_EQUALS_STR(ret[1], "1", "");
	A_EQUALS_STR(ret[2], "b", "");
	A_EQUALS_STR(ret[3], "", "");
	A_EQUALS_STR(ret[4], "", "");
	A_EQUALS_STR(ret[5], "", "");
	A_NULL(ret[6], "");

	free((void*)ret);
}



static void test_ac_getValueFromSplit(){
	char** ret = ac_split2(" a  =1;b;c  dgg  =  3  g", '=', ';', 4);
	//
	int i;
	i = ac_getIndexFromKeyValueArray(ret, " a  ");
	A_EQUALS(1, i, "");
	A_EQUALS_STR("1", ret[i], "");
	//
	i = ac_getIndexFromKeyValueArray(ret, "b");
	A_EQUALS(3, i, "");
	A_EQUALS_STR("", ret[i], "");
	//
	i = ac_getIndexFromKeyValueArray(ret, "c  dgg  ");
	A_EQUALS(5, i, "");
	A_EQUALS_STR("  3  g", ret[i], "");
	//
	i = ac_getIndexFromKeyValueArray(ret, "c");
	A_EQUALS(-1, i, "");

	free(ret);

	//形式が間違っている場合に-1となるか？
	ret = ac_split2("a=1", '=', ';', 4);
	ret[1] = NULL;
	i = ac_getIndexFromKeyValueArray(ret, "a");
	A_EQUALS(-1, i, "");

	free(ret);
}



static void test_ac_trimArray(){
	char** ret = ac_split2(" a  =1;b;c  dgg  =  3  g;   ", '=', ';', 4);
	ac_trimArray(ret);
	//
	A_EQUALS_STR("a", ret[0], "");
	A_EQUALS_STR("1", ret[1], "");
	A_EQUALS_STR("b", ret[2], "");
	A_EQUALS_STR("", ret[3], "");
	A_EQUALS_STR("c  dgg", ret[4], "");
	A_EQUALS_STR("3  g", ret[5], "");
	A_EQUALS_STR("", ret[6], "");
	A_EQUALS_STR("", ret[7], "");

	free(ret);
}



static void test_ac_removeQuoteArray(){
	char** ret = ac_split2(" a  ='1';b;c  dgg  =\" 3  \\g\";   ", '=', ';', 4);
	ac_removeQuoteArray(ret, '"', '\\');
	//
	A_EQUALS_STR(" a  ", ret[0], "");
	A_EQUALS_STR("'1'", ret[1], "");
	A_EQUALS_STR("b", ret[2], "");
	A_EQUALS_STR("", ret[3], "");
	A_EQUALS_STR("c  dgg  ", ret[4], "");
	A_EQUALS_STR(" 3  g", ret[5], "");
	A_EQUALS_STR("   ", ret[6], "");
	A_EQUALS_STR("", ret[7], "");

	free(ret);
}



void test_BucketUtils(){
	//
	test_ac_toLowerCase();
	test_ac_unescapeChar();
	test_ac_splitKeyValueArrayWithQuote();
	test_ac_splitHttpHeaderValue();
	test_ac_split2();
	test_ac_getValueFromSplit();
	test_ac_trimArray();
	test_ac_removeQuoteArray();
}