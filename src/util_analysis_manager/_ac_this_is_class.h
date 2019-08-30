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

#ifndef __PRIVATE_AC_CTHIS_IS_CLASS_H__
#define __PRIVATE_AC_CTHIS_IS_CLASS_H__

#include "ac_this_is_class.h"

typedef struct _AcCThisIsClass {
	struct AcCThisIsClass base;
	AcDeleteFunc_T deleteFunc;
} _AcCThisIsClass;



#endif  //end __PRIVATE_AC_CTHIS_IS_CLASS_H__

