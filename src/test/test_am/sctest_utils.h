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

#ifndef __SCTEST_UTILS_H__
#define __SCTEST_UTILS_H__


#include <string.h>
#include <stdio.h> 


/**
@file
単体テスト用のユーティリティ(Simple Customization Test)。
APRライブラリなど他のライブラリと独立しています。

~english
This is unit test utilities (Simple Customization Test).
This independent from other liblaries like APR lib.
*/

void sctest_equalsC(const char expected, const char actual, const char* msg, const char* file, const int line);
void sctest_equalsStr(const char* expected, const char* actual, const char* msg, const char* file, const int line);
void sctest_equalsNStr(const char* expected, const char* actual, size_t len, const char* msg, const char* file, const int line);
void sctest_state(const int result, const char* msg, const char* file, const int line);
void sctest_a_true(const int result, const char* msg, const char* file, const int line);
void sctest_a_false(const int result, const char* msg, const char* file, const int line);
//
void sctest_null(const void* v1, const char* msg, const char* file, const int line);
void sctest_notNull(void* v1, const char* msg,
	const char* file, const int line);
	

	
///
void sctest_output(const char* file, const int line);
void sctest_printResult();
void sctest_incrementCnt();


#define A_EQUALS(expected, actual, msg)  {sctest_incrementCnt(); if((int)(expected) != (int)(actual)){\
	sctest_output(__FILE__, __LINE__);printf(", expected:'%d', actual:'%d', %s\n", (int)(expected), (int)(actual), msg);}}
#define A_EQUALS_C(expected, actual, msg) sctest_equalsC(expected, actual, msg, __FILE__, __LINE__);
#define A_EQUALS_STR(expected, actual, msg) sctest_equalsStr(expected, actual, msg, __FILE__, __LINE__);
#define A_EQUALS_NSTR(expected, actual, len, msg) sctest_equalsNStr(expected, actual, len, msg, __FILE__, __LINE__);
#define A_STATE(state1, msg) sctest_state(state1, msg, __FILE__, __LINE__);
#define A_TRUE(state1, msg) sctest_a_true(state1, msg, __FILE__, __LINE__);
#define A_FALSE(state1, msg) sctest_a_false(state1, msg, __FILE__, __LINE__);
#define A_NOT_NULL(value, msg) sctest_notNull(value, msg, __FILE__, __LINE__);
#define A_NULL(value, msg) sctest_null(value, msg, __FILE__, __LINE__);



#endif   //end __SCTEST_UTILS_H__


