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

#include "sctest_utils.h"

static int m_errCnt = 0;
static int m_assertCnt = 0;

void sctest_output(const char* file, const int line) {
	++m_errCnt;
	printf("file=%s, line=%d, ", file, line);
}

void sctest_incrementCnt(){
	++m_assertCnt;
}

void sctest_equalsC(const char expected, const char actual, const char* msg, const char* file, const int line) {
	++m_assertCnt;
	if (expected != actual) {
		sctest_output(file, line);
		printf(", expected:'%c', actual:'%c', %s\n", expected, actual, msg);
	}
}

void sctest_equalsStr(const char* expected, const char* actual,const char* msg, const char* file, const int line) {
	++m_assertCnt;
	if (strcmp(expected, actual) != 0) {
		sctest_output(file, line);
		printf(", expected:'%s', actual:'%s', %s\n", expected, actual, msg);
	}
}


void sctest_equalsNStr(const char* expected, const char* actual, size_t len, const char* msg, const char* file, const int line) {
	++m_assertCnt;
	if (strncmp(expected, actual, len) != 0) {
		sctest_output(file, line);
		printf(", expected:'%s', actual:'%s', %s\n", expected, actual, msg);
	}
}


void sctest_state(const int result, const char* msg, const char* file, const int line){
	++m_assertCnt;
	if(!(result)){
		sctest_output(file, line);
		printf(", state error, %s\n", msg);
	}
};	
void sctest_a_true(const int result, const char* msg, const char* file, const int line){
	++m_assertCnt;
	if(!(result)){
		sctest_output(file, line);
		printf(", expected true error, %s\n", msg);
	}
};
void sctest_a_false(const int result, const char* msg, const char* file, const int line){
	++m_assertCnt;
	if(result){
		sctest_output(file, line);
		printf(", expected false error, %s\n", msg);
	}
};	


///
void sctest_null(const void* v1, const char* msg, const char* file, const int line){
	++m_assertCnt;
	if(v1 != NULL){
		sctest_output(file, line);
		printf(", expected: NULL, actual: NOT NULL, %s\n", msg);
	}
};



///
void sctest_notNull(void* v1, const char* msg, const char* file, const int line){
	++m_assertCnt;
	if(v1 == NULL){
		sctest_output(file, line);
		printf(", expected: NOT NULL, actual: NULL, %s\n", msg);
	}
};




void sctest_printResult(){
	printf("tests are finished.------- ");
	printf("of %d/%d assert errors.\n", m_errCnt, m_assertCnt);
};


