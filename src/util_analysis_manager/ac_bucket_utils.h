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

#ifndef __AC_BUCKET_UTILS_H__
#define __AC_BUCKET_UTILS_H__



#include <string.h>
#include <stdio.h> 

#include "ac_this_is_class.h"

/**
@file
�����apr_bucket�֘A�̃��[�e�B���e�B�ł��B
�e�X�g�p���܂�ł��܂��B

@~english 
This is utilities assciated with apr_bucket or something.
Testing is inclueded, too.

*/



/* apr��httpd�̃��C�u�������g����Linux�n�̏ꍇ�A�ȉ��̃G���[����������B
�@no decision has been made on APR_PATH_MAX for your platform
 ���̉���̂��߈ȉ���include���L�q�B
 */
#ifndef _WIN32 
#include <linux/limits.h>
#endif
	//
#include "apr_buckets.h"

extern apr_pool_t* sctest_global_pool;

#define SCTEST_BT_FLUSH ((const char*)2)
#define SCTEST_BT_EOS ((const char*)1)
#define SCTEST_BT_FILE ((const char*)3)
#define SCTEST_BT_TRANSIENT ((const char*)4)
#define SCTEST_BT_OTHER ((const char*)5)


void sctest_initialize();
void sctest_destoroy();

apr_bucket_brigade** sctest_createBucketBrigade(
	apr_pool_t* pool, apr_bucket_alloc_t* ba, const char* src[]);


void sctest_destroy_brigades(apr_bucket_brigade** bb);

const char* sctest_getBucketType(apr_bucket* b);
void ac_printBrigade(apr_bucket_brigade* bb);

int sctest_printNotEquals(const char** expectedStrs, apr_bucket_brigade* bb);


/**
�������ɕϊ����ăR�s�[����B
@param str [in]�R�s�[�Ώە�����B
@return �R�s�[����������i������free���邱�Ɓj�B�������m�ۂɎ��s�����ꍇ��NULL���Ԃ�B
*/
char* ac_copyToLowerCase(const char* str);


/**
���͕�������������ɕϊ�����B�}���`�o�C�g�͍l�����Ȃ��B
@param str [in]�Ώە�����B
@n         [out]�������ɕϊ����ꂽ������B
*/
void ac_toLowerCase(char* str);


/**
�G�X�P�[�v��������菜���B
��F"ab\c" ��"abc"
@param str [in]�Ώە�����
@n         [out]�A���G�X�P�[�v����������
*/
void ac_unescapeChar(char* str, const char escape);


/**
userdata�ɃN���X��o�^���A�����Pool��cleanup�ɃN���X��delete��o�^����B
�o�^�����Pool���j�������Ƃ��ɃN���X���j�������B
�o�^�����N���X�́A ac_getClassFromPoolUserData() �Ŏ擾����B
@param pool [in]�o�^��̃v�[��
@param userDataKey [in]userdata�̓o�^�L�[
@param clazz [in]�o�^����N���X
@see ac_getClassFromPoolUserData
*/
void ac_registClassToUserDataAndPoolCleanup(apr_pool_t* pool, const char* userDataKey, void* clazz);

/**
ac_registClassToUserDataAndPoolCleanup() �œo�^�����N���X��userdata����擾����B
@param pool [in]�o�^��̃v�[��
@param userDataKey [in]userdata�̓o�^�L�[
@see ac_registClassToUserDataAndPoolCleanup
*/
void* ac_getClassFromPoolUserData(const char* userDataKey, apr_pool_t* pool);


/**
�o�͗p�̊֐���ݒ肷��B�f�t�H���g�ł� sctest_printfFunc() (=static�֐�)���ݒ肳��Ă���B
@n �����printf�ŕW���o�͂ɕ�������o�͂���B
@n �ȉ��̂悤�ɂ����Apache��error���O�ɏo�͂����B
@code
static void printFunc(const char* msg, va_list args){
	char str[301];
	vsnprintf(str, 300, msg, args);
	ap_log_error(APLOG_MARK, APLOG_NOTICE, 0, NULL, str);
}

static void mod_replace_content_register_hooks(apr_pool_t* p){
	ac_setPrintFunc(printFunc);
}
@endcode

@param printFunc [in]�ǂ����ɏo�͂���֐��BNULL���w�肷��ƕW���o�͂ɏ������ފ֐�( sctest_printfFunc() ) ���w�肳���B
*/
int ac_setPrintFunc(void (*printFunc)(const char* msg, va_list args));

/**
�������printf�t�H�[�}�b�g�Ŏw�肵�A�o�͂���B
ac_setPrintFunc() �Ŏw�肵���o�͊֐�������ŌĂяo���B
@see #ac_setPrintFunc() 
*/
void ac_printf(const char* msg, ...);



/**
�ua=1;b;c="3;";�v�̌`���̕�����𕪉�����B{���O1, �l1, ���O2, �l2, ... , NULL}�̌`���Ō��ʂ��Ԃ�B
@n  �_�u���N�H�[�g���l�������(�G�X�P�[�v'\'���l��)�B �A�������؂蕶���͖��������i�܂�Ablank�͖��������B)
@n  �ua=1;b;c="3;";;�v �ˌ��ʁF{"a", "1", "b","", "c", "\"3;\"", NULL}
@param str       [in]�����̑Ώە�����
@param seps      [in]��؂蕶��";", "()"�Ȃǂ��w�肷��B
@param isSplitEq [in]"="�𕪊����邩���w�肷��BAC_TRUE�̂Ƃ��A��������B
@param maxSize   [in]�������̍ő吔�B�w��̐��ȏ�ɋ�؂蕶��������ꍇ�A���������B
@return strKeyValueArray�^�C�v�̔z��{"a", "1", "b",NULL, "c", "3", NULL}�̂悤�Ȕz�񂪕Ԃ�Bfree(ret)�ŉ���ł���B
@n     �̈�m�ۂɎ��s�����ꍇ��NULL�B
@note   trim�͂��Ȃ��B
*/
char** ac_splitKeyValueArrayWithQuote(const char* str, const char* seps, AcBool isSplitEq, size_t maxSize);


/**
@brief HTTP�w�b�_�l�� key=value; �`�����p�[�X���Ĕz��ɂ��ĕԂ��B
@n     �O��̋󔒂��폜����A�_�u���N�H�[�g���폜���ꂽ�����񂪕Ԃ�B�N�H�[�g���̃G�X�P�[�v����\���폜�����B
@n     ac_splitKeyValueArrayWithQuote() ��trim�ƃ_�u���N�H�[�g�̍폜�����s�������ʂ�Ԃ��B
@return KeyValueArray�^�C�v�̔z��{"a", "1", "b",NULL, "c", "3", NULL}�̂悤�Ȕz�񂪕Ԃ�Bfree(ret)�ŉ���ł���B
@see   ac_splitKeyValueArrayWithQuote
@see   ac_trimArray
@see   ac_removeQuoteArray
*/
char** ac_splitHttpHeaderValue(const char* str, const char* seps, AcBool isSplitEq, size_t maxSize);


/**
"a=1;b;c=3;"�̌`���̕�����𕪉�����B
{"a", "1", "b","", "c", "3", NULL}
@param str [in]�����̑Ώە�����
@param eq [in]"="�Ȃǂ��w�肷��B
@param delimiter [in]";"�Ȃǂ��w�肷��B
@return KeyValueArray�^�C�v�̔z��{"a", "1", "b",NULL, "c", "3", NULL}�̂悤�Ȕz�񂪕Ԃ�Bfree(ret)�ŉ���ł���B
@n     �̈�m�ۂɎ��s�����ꍇ��NULL�B
*/
char** ac_split2(const char* str, const char eq, const char delimiter, size_t maxSize);


/**
{�L�[���A�l�A...}�̏��Őݒ肳�ꂽ�z��̃L�[���Ɉ�v����l�̈ʒu���擾����B
@param strKeyValueArray  [in]�����Ώۂ̔z��BKeyValueArray�^�C�v�̔z��
@param key               [in]��������L�[��
@return �v�f�̈ʒu�BstrKeyArray[i]���ړI�̒l�ɂȂ�B������Ȃ������Ƃ�-1��Ԃ��B
*/
int ac_getIndexFromKeyValueArray(char** strKeyValueArray, const char* key);



/**
ac_split2() �� ac_splitHttpHeaderValue() �̌��ʂ̔z��̌`�����̕�����̑O��̋󔒂��폜����B
��F{"a", " 1 ", "b ","", "c", "3", NULL}
@param strArray [in]�Ώ۔z��B
@n         [out]���ꂼ��O��̋󔒂��폜����
@note �J�n�|�C���^�̈ʒu�����炵�A���͘A������󔒂̍ŏ��̕�����\0�Œu���ς���B���̂��߁Afree()�ł��ꂼ��J�����邱�Ƃ͂ł��Ȃ��B
@n    ac_split2() ��return�l�̂悤�ɑ��̗̈�ɍ쐬���ꂽ������̈ʒu�݂̂��|�C���g�����z�����͂ɂ��Ȃ���΂Ȃ�Ȃ��B
*/
void ac_trimArray(char** strArray);


/**
ac_split2() �� ac_splitHttpHeaderValue() �̌��ʂ̔z����̕�����̑O��̃N�H�[�g���폜����B
@n  �擪��quote������ꍇ�̂ݎ��s�����B����ȊO�͕������ύX���Ȃ��B
@n  �܂��A�擪��quote�̏ꍇ�̓G�X�P�[�v�����̃A���G�X�P�[�v�i�폜�j���s���B
��F{"\"a\"", "\"1 ", "b ","", "c", "3", NULL}
�ˌ��ʁF{"a", "1 ", "b ","", "c", "3", NULL}
@param strArray [in]�Ώ۔z��
@n         [out]���ꂼ��O��̃N�H�[�g���폜����
@note ac_trimArray() ��note���Q�ƁB
*/
void ac_removeQuoteArray(char** strArray, char quote, char escape);



#endif  //end __AC_BUCKET_UTILS_H__


