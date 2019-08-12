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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "ac_this_is_class.h"
#include "_ac_this_is_class.h"


struct CDummyClass {
	_AcCThisIsClass* thisIsClass;
};


AcCThisIsClass* AcCThisIsClass_new(const char* name,  void (*deleteFunc)(void*)) {
	_AcCThisIsClass* self = (_AcCThisIsClass*)malloc(sizeof(_AcCThisIsClass));
	if (self == NULL) return NULL;
	self->base.name = name;
	self->deleteFunc = deleteFunc;
	return (AcCThisIsClass*)self;
}


void AcClass_delete(void* c) {
	struct CDummyClass* self = (struct CDummyClass*)c;
	AcDeleteFunc_T deleteFunc;
	deleteFunc = self->thisIsClass->deleteFunc;
	_AcCThisIsClass* thisIsClass = self->thisIsClass;
	//
	deleteFunc(self);
	//
	free(thisIsClass);
}


const char* AcClass_getName(void* p_this){
	struct CDummyClass* dummy = (struct CDummyClass*)p_this;
	_AcCThisIsClass* self = (_AcCThisIsClass*)dummy->thisIsClass;
	return self->base.name;
}


const AcBool ac_checkClass(const char* expected, const char* real, const char* errmsg, const AcBool isExitIfErr) {
	if (strcmp(expected, real) != 0) {
		fprintf(stderr, "\nexpected class is %s but %s. errmsg=[%s]", expected, real, errmsg);
		if (isExitIfErr) exit(1);
		return AC_FALSE;
	}
	return AC_TRUE;
}
