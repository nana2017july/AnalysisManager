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
これはapr_bucket関連のユーティリティです。
テスト用も含んでいます。

@~english 
This is utilities assciated with apr_bucket or something.
Testing is inclueded, too.

*/



/* aprかhttpdのライブラリを使うとLinux系の場合、以下のエラーが発生する。
　no decision has been made on APR_PATH_MAX for your platform
 その回避のため以下のincludeを記述。
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
小文字に変換してコピーする。
@param str [in]コピー対象文字列。
@return コピーした文字列（自分でfreeすること）。メモリ確保に失敗した場合はNULLが返る。
*/
char* ac_copyToLowerCase(const char* str);


/**
入力文字列を小文字に変換する。マルチバイトは考慮しない。
@param str [in]対象文字列。
@n         [out]小文字に変換された文字列。
*/
void ac_toLowerCase(char* str);


/**
エスケープ文字を取り除く。
例："ab\c" ⇒"abc"
@param str [in]対象文字列
@n         [out]アンエスケープした文字列
*/
void ac_unescapeChar(char* str, const char escape);


/**
userdataにクラスを登録し、さらにPoolのcleanupにクラスのdeleteを登録する。
登録するとPoolが破棄されるときにクラスも破棄される。
登録したクラスは、 ac_getClassFromPoolUserData() で取得する。
@param pool [in]登録先のプール
@param userDataKey [in]userdataの登録キー
@param clazz [in]登録するクラス
@see ac_getClassFromPoolUserData
*/
void ac_registClassToUserDataAndPoolCleanup(apr_pool_t* pool, const char* userDataKey, void* clazz);

/**
ac_registClassToUserDataAndPoolCleanup() で登録したクラスをuserdataから取得する。
@param pool [in]登録先のプール
@param userDataKey [in]userdataの登録キー
@see ac_registClassToUserDataAndPoolCleanup
*/
void* ac_getClassFromPoolUserData(const char* userDataKey, apr_pool_t* pool);


/**
出力用の関数を設定する。デフォルトでは sctest_printfFunc() (=static関数)が設定されている。
@n これはprintfで標準出力に文字列を出力する。
@n 以下のようにすればApacheのerrorログに出力される。
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

@param printFunc [in]どこかに出力する関数。NULLを指定すると標準出力に書き込む関数( sctest_printfFunc() ) が指定される。
*/
int ac_setPrintFunc(void (*printFunc)(const char* msg, va_list args));

/**
文字列をprintfフォーマットで指定し、出力する。
ac_setPrintFunc() で指定した出力関数を内部で呼び出す。
@see #ac_setPrintFunc() 
*/
void ac_printf(const char* msg, ...);



/**
「a=1;b;c="3;";」の形式の文字列を分解する。{名前1, 値1, 名前2, 値2, ... , NULL}の形式で結果が返る。
@n  ダブルクォートも考慮される(エスケープ'\'も考慮)。 連続する区切り文字は無視される（つまり、blankは無視される。)
@n  「a=1;b;c="3;";;」 ⇒結果：{"a", "1", "b","", "c", "\"3;\"", NULL}
@param str       [in]分解の対象文字列
@param seps      [in]区切り文字";", "()"などを指定する。
@param isSplitEq [in]"="を分割するかを指定する。AC_TRUEのとき、分割する。
@param maxSize   [in]分割個数の最大数。指定の数以上に区切り文字がある場合、無視される。
@return strKeyValueArrayタイプの配列{"a", "1", "b",NULL, "c", "3", NULL}のような配列が返る。free(ret)で解放できる。
@n     領域確保に失敗した場合はNULL。
@note   trimはしない。
*/
char** ac_splitKeyValueArrayWithQuote(const char* str, const char* seps, AcBool isSplitEq, size_t maxSize);


/**
@brief HTTPヘッダ値の key=value; 形式をパースして配列にして返す。
@n     前後の空白が削除され、ダブルクォートが削除された文字列が返る。クォート内のエスケープ文字\も削除される。
@n     ac_splitKeyValueArrayWithQuote() にtrimとダブルクォートの削除を実行した結果を返す。
@return KeyValueArrayタイプの配列{"a", "1", "b",NULL, "c", "3", NULL}のような配列が返る。free(ret)で解放できる。
@see   ac_splitKeyValueArrayWithQuote
@see   ac_trimArray
@see   ac_removeQuoteArray
*/
char** ac_splitHttpHeaderValue(const char* str, const char* seps, AcBool isSplitEq, size_t maxSize);


/**
"a=1;b;c=3;"の形式の文字列を分解する。
{"a", "1", "b","", "c", "3", NULL}
@param str [in]分解の対象文字列
@param eq [in]"="などを指定する。
@param delimiter [in]";"などを指定する。
@return KeyValueArrayタイプの配列{"a", "1", "b",NULL, "c", "3", NULL}のような配列が返る。free(ret)で解放できる。
@n     領域確保に失敗した場合はNULL。
*/
char** ac_split2(const char* str, const char eq, const char delimiter, size_t maxSize);


/**
{キー名、値、...}の順で設定された配列のキー名に一致する値の位置を取得する。
@param strKeyValueArray  [in]検索対象の配列。KeyValueArrayタイプの配列
@param key               [in]検索するキー名
@return 要素の位置。strKeyArray[i]が目的の値になる。見つからなかったとき-1を返す。
*/
int ac_getIndexFromKeyValueArray(char** strKeyValueArray, const char* key);



/**
ac_split2() や ac_splitHttpHeaderValue() の結果の配列の形式内の文字列の前後の空白を削除する。
例：{"a", " 1 ", "b ","", "c", "3", NULL}
@param strArray [in]対象配列。
@n         [out]それぞれ前後の空白を削除する
@note 開始ポインタの位置をずらし、後ろは連続する空白の最初の文字を\0で置き変える。そのため、free()でそれぞれ開放することはできない。
@n    ac_split2() のreturn値のように他の領域に作成された文字列の位置のみをポイントした配列を入力にしなければならない。
*/
void ac_trimArray(char** strArray);


/**
ac_split2() や ac_splitHttpHeaderValue() の結果の配列内の文字列の前後のクォートを削除する。
@n  先頭にquoteがある場合のみ実行される。それ以外は文字列を変更しない。
@n  また、先頭がquoteの場合はエスケープ文字のアンエスケープ（削除）も行う。
例：{"\"a\"", "\"1 ", "b ","", "c", "3", NULL}
⇒結果：{"a", "1 ", "b ","", "c", "3", NULL}
@param strArray [in]対象配列
@n         [out]それぞれ前後のクォートを削除する
@note ac_trimArray() のnoteも参照。
*/
void ac_removeQuoteArray(char** strArray, char quote, char escape);



#endif  //end __AC_BUCKET_UTILS_H__


