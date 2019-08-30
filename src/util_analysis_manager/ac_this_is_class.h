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

#ifndef __AC_THIS_IS_CLASS_H__
#define __AC_THIS_IS_CLASS_H__


/**
@file
Anddy's Custumizationライブラリ。
C言語でクラスっぽいことをするためのライブラリです。
このファイルはAPRに依存しません。独立しています。

@~english
Anddy's Custumization libraries.
This is library whitch do class-like programming in c-language.
This file dose not depend on apr libraries.

*/

/**
@struct AcCThisIsClass
@brief 構造体をクラスとして扱うためのマーカーであり、基本処理をするためのクラス。
@detail <pre>
・1回の継承が可能（複数回はできない）
・多重継承はできない。
・多態性は持たせられる。

【実装ルール】
・必ず構造体の先頭に宣言する。
・派生クラスでは親クラスの*_Superを構造体の先頭に宣言する。

【命名ルール】
クラス名：
　・先頭大文字
　・派生クラスは後ろに名前を付ける。例：「TestSimple」
　・privateメンバーも含めた親クラスは後ろに_Superをつける。例：「Test_Super」
メソッド名
　・クラス名とメソッド名をアンダーバーでつなぐ。例：「Test_calc」「TestSimple_calc」
　・メソッド名の先頭は小文字
例：
 　・TestSimple ⇒親クラスTestのSimple派生クラス
 　・Test_calc　⇒Testクラスのメンバーのcalc関数
 　・Test_HHH　⇒クラスとは無関係。
</pre>
@code
【使い方の例】
//Testクラス-----------------------
//クラスの宣言をする
typedef struct CTest {
	AcCThisIsClass* thisIsClass;
} CTest;

//privateのメンバーを追加した親クラスを作成する。
typedef struct CTest_Super {
	CTest publicMember;
	int (*calc)(int h);
	int len;
	AcDeleteFunc_T childDeleteFunc;
} CTest_Super;


//publicメソッドを作成する。
static const int CTest_calc(Test* p_this, int h) {
	CTestSimple* self = (CTestSimple*)p_this;
	return self->calc((CTest*)self, h);
};

//親クラスのデストラクタを隠蔽(static)で作成する。
//thisIsClassの破壊はdeleteClass側で行うのでこの中では記述不要
static void CTest_delete(void* p_this) {
	CTest_Super* self = (CTest_Super*)p_this;
	self->childDeleteFunc(self);
	//この後、親クラスのデストラクト処理を書く
}

//親クラスの初期化関数を作成する。この関数は派生クラスのコンストラクタから呼び出す。
static void CTest_init(void* p_this, const char* className, DeleteFunc deleteFunc, int (*calc)(int)){
	CTest_Super* self = (CTest_Super*)p_this;
	self->publicMember.thisIsClass = AcCThisIsClass_new(className, Test_delete);
	self->len = 0;
	self->calc = calc;
	self->childDeleteFunc = deleteFunc;
}


//派生クラス------------------------
//空の定義を書く
typedef struct CTestSimple {
	CTest publicMember;
}CTestSimple;

//派生クラスのprivateを作成する。必ず親クラスを先頭に書く。
typedef struct CTestSimple_Super {
	CTest_Super super;
	char* str;
}CTestSimple_Super;

//派生クラスのオーバーライドするメソッドを作成する。
static const int CTestSimple_calc(CTest* p_this, int h) {
	CTestSimple_Super* self = (CTestSimple_Super*)p_this;
	return self->super.len * h;
};

//デストラクタを作成する。派生クラス側の破壊処理だけ行う。親クラスの破壊は親クラスが行うのでここでは書かない。
static void CTestSimple_delete(void* p_this) {
	CTestSimple_Super* self = (CTestSimple_Super*)p_this;
	//デバッグのため引数のクラスがこのクラスかチェックしてもよい。
	ac_checkClass("TestSimple", self->super.publicMember.thisIsClass->name, "in CTestSimple_delete()", AC_FALSE);
	free(self->str);
	free(self);
}

//派生クラスのコンストラクタを作成する。
CTest* CTestSimple_new(cost char* str) {
	CTestSimple_Super* self = (CTestSimple_Super*)malloc(sizeof(CTestSimple_Super));
	if (self == NULL) return NULL;
	//クラス基底を作成
	CTest_init(self, "CTestSimple", CTestSimple_delete,  CTestSimple_calc);
	//派生クラスのメンバの初期化
	self->str = malloc(strlen(str)+1);
	strcpy(self->str, str);
	//
	return ((CTest*)self);
}

//使い方例------------
//派生クラスを生成
CTest* t = CTestSimple_new("aaa");
//親のメソッドに派生クラスを引数にして呼び出す。
int i = CTest_calc(t, 10);

@endcode
*/



#define AC_TRUE 1
#define AC_FALSE 0

typedef int AcBool;



typedef struct AcCThisIsClass {
	const char* name;
} AcCThisIsClass;

/** 削除関数ポインタお型定義 */
typedef void (*AcDeleteFunc_T)(void*);

/**
クラス基盤となるクラスの生成
*/
AcCThisIsClass* AcCThisIsClass_new(const char* name, void (*deleteFunc)(void*));

/**
AcCThisIsClassを搭載したクラスを破棄する。
このメソッド以外でfreeなどで破棄しないこと。
*/
void AcClass_delete(void*);

/**
クラス名を取得する。 AcCThisIsClass を継承している必要ある。継承していない場合は動作不定。
*/
const char* AcClass_getName(void*);

/** クラス名をチェックする。
@param ecpected [in]期待するクラス名。
@param real [in]実際のクラス名。
@param errmsg [in]返り値がAC_FALSEのとき、標準エラー出力するエラーメッセージ。
@oaram isExitIfErr [in]AC_TRUEの場合、返り値がAC_FALSEのとき、exit();を呼び出す。
@return 期待するクラス名と違う場合、AC_FALSE
*/
const AcBool ac_checkClass(const char* ecpected, const char* real, const char* errmsg, const AcBool isExitIfErr);

#endif  //end __AC_THIS_IS_CLASS_H__

