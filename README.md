# ボディ部を分析・置換するApache Module用ライブラリ
主にApache Moduleなどで使用することをイメージして作成しているが、基本的にはAPRの補助ライブラリ。<br>
apr_bucket_brigadeは、文字列を連続で扱えない問題があり、Apache Moduleのフック関数も断続的に呼ばれる。<br>
フック関数は1回のリクエストもしくはレスポンスでそれぞれ1度だけ呼ばれると勘違いされている方も多いかと思われる。<br>
しかし、実際には、HTTPボディ部を分割してbriagdeに入れて、何度もフック関数が呼ばれるため、文字列が断続する。<br>
例えば、"012345678"というHTTPボディ部があった場合、1度目はフック関数に"012"がbrigadeで渡され、<br>
2度目はフック関数に"345678"が渡されるという具合である。<br>
もしこの場合に"23"という文字列を検索しようとすると、"2"で途切れてしまうのでフック関数は次に呼ばれたときにやってくるbrigadeに
"3"が含まれているかをチェックしなければならない。<br>
これはかなり難しい。<br>
<br>
このライブラリはbrigadeの断続性を気にせずに処理を行える、便利なツールである。<br>
<br>

## このライブラリができること
派生クラスを作成するとbrigadeの断続性を気にせずに、以下のことができます。<br>
* HTTPボディ文字列の検索
* HTTPボディ文字列の置換
* ライブラリを使用した部分一致、HTMLタグの置換をするApache Moduleサンプル

提供しているApaceh Moduleは、ライブラリを使用したサンプルです。
<br>

## はじめに
ライセンスは、Apache License, Version 2.0とします。<br>
<br>
**【関数・クラスのドキュメント（doxygen）】**<br>
<!--https://nana2017july.github.io/html_parser/index.html--><br> 
<br>


## 変更履歴
ver.1.0 新規作成<br>

## 動作環境
**【コンパイラ】**<br>
以下のコンパイル環境で使用できることを条件にしました。<br>
* Microsoft Visual Studio 2019 
* CentOS7 g++(4.8.5)
<br>
<br>

**【使用したもの】**<br>
* Apache Httpd (2.4.39(Win64)、2.4.6(CentOS) )
* C言語

Apache2.4を各自の環境にインストールしておくこと。<br>
<br>
<br>

**【Microsoft Visual Studio のビルドの補足】**<br>
Microsoft Visual Studio の場合、以下のことをしてからビルドをしてください。
* 「mod_replace_content.sln」を起動
* プロジェクトの設定を開き、「C/C++」⇒「追加のインクルード ディレクトリ」にApache Httpdフォルダ内のincludesフォルダを追記。
* プロジェクトの設定を開き、「リンカー」⇒「追加のライブラリ ディレクトリ」にApache Httpdフォルダ内のlibsフォルダを追記。
* プロジェクトの設定を開き、「デバッグ」⇒「環境」に「PATH=%PATH%;Apache Httpdフォルダ内のbinフォルダ」を追記

また、ビルド後は、作成された「mod_replace_content.so」をApache Httpdフォルダのmodulesにコピーして、httpd.confに設定を追記してから
httpdを起動してください。<br>
httpd.confの書き方は後ほど示します。<br>
<br>
　※テストを走らせたい場合は、「test\test_am\test_am.sln」を起動して、上記と同じ設定をします。
<br>

**【CentOS のビルドの補足】**<br>
CentOS の場合、以下の手順でビルドをしてください。
* common.mkを開き、先頭のAPR_LIB_DIR などが自身の環境にあっているかを確認し、必要であれば修正する。
* make test_am を実行し、エラーが0件であることを確認。
* make all
* make install

上記でApache Moduleへコピーされているので、httpd.confに設定を追記してからhttpdを起動してください。<br>
httpd.confの書き方は後ほど示します。<br>

**用意されているターゲット**<br>

|**ターゲット**|**説明**|
|---|---|
|make all |Apache Moduleのサンプルをビルドする。mod_replace_content.soが作成される。|
|make clean|.oなどの生成ファイルをすべて削除する。|
|make test_am|CAnalysisManager関連のテストをビルドして実行する。|
|make check_apxs_vars|makefile内で定義されているapxs系の値を表示する。|
|make install|生成した.soをApache Moduleにインストールする。|
|make start|httpdを開始する。|
|make restart|httpdをリスタートする。|
|make stop|httpdを停止する。|

<br>


## Apaceh Moduleサンプルを利用するためのhttpd.conf記述
```Apache
# Load created module
LoadModule replace_content_module  modules/mod_replace_content.so
# Output filter setting
SetOutputFilter REPLACE_CONTENT_OUTPUTFILTER
Header set Last-Modified "Sat, 19 Apr 2014 21:53:07 GMT"
# replace content setting
ReplaceContent "tag:true" "head" "<banking >"
```


OutputFilterの設定と、ReplaceContentディレクティブの設定をします。<br>
ReplaceContentの書式は以下です。<br>
```Apache
	ReplaceContent partial_match|tag[:true|false] target replacement
```
|**項目**|**説明**|
|---|---|
|項目1|partial_match ...部分一致置換を指定。<br>tag...HTMLタグ置換を指定。後ろにtrueを指定するとタグ自体を置換。falseはタグの次の位置に挿入。|
|項目2|置換対象文字列|
|項目3|置換後の文字列|


* example1<br>
ReplaceContent "partial_match" "a" "x"<br>
入力HTTPボディ："123a45"<br>
結果HTTPボディ："123x45"<br>

* example2<br>
ReplaceContent "tag:false" "head" "<meta >"<br>
入力HTTPボディ："\<head\>\</head\>"<br>
結果HTTPボディ："\<head\>\<meta \>\</head\>"<br>
<br>

## クラスの簡易な説明<br>
C言語ですが、ファイルとstructをうまく使い、C++のclassに近い動作をしています。<br>
classを使用するためのルールがいくつかあり、従う必要があります。<br>

* クラスはstructで表現する。
* クラスstructのメンバは順番が厳格。先頭に必ずAcCThisIsClassを定義する。
* クラスメソッド名の先頭は必ずクラス名にし、アンダーバーでうしろにメソッド名を連結する。
* クラスメソッドの先頭の引数は必ず対象クラスのポインタ。
* privateメソッドは.cファイル内でstatic関数にすることで実装する。
* 先頭が_のヘッダファイルはprivateなので利用者側がincludeしてはならない。

**記述例**
```cpp
// クラスの公開する定義。クラス内部の定義は.cファイル内などで隠蔽して定義する。
typedef struct CBucketController {
	AcCThisIsClass* thisIsClass;
} CBucketController;

// クラスメソッド宣言
void CBucketController_exportModifiedBrigadeToBrigade(CBucketController* p_this, apr_bucket_brigade* outbb);
```


とりあえず覚えておくクラスは以下のクラスだけで十分です。<br>

|**クラス名**|**説明**|
|---|---|
|***CAnalysisManagerクラス***| brigadeの管理を行い、CAnalysisExecutorへの指示を出すクラス。|
|***CAnalysisExecutorクラス***| HTTPボディの文字列の分析や置換を行うクラス。自作する。<br>CAnalysisManagerが適切なメソッドを呼び出す。|


<br>


**【シンプルな使用例】**<br>

```cpp
//解析者の生成(このクラスを自作することで様々な機能を実装できる)
CAnalysisExecutor* executor = (CAnalysisExecutor*)CAnalysisExecutor_PartialMatch_new("a", "x");
//マネージャーの生成
CAnalysisManager* ana = CAnalysisManager_new(executor, pool, bucket_alloc, NULL, in_bucket_brigade);

//空のbrigadeを破棄する
apr_brigade_destroy(in_bucket_brigade);

//1回目のbrigadeを受け取る
apr_bucket_brigade* brigade = CAnalysisManager_run(ana, NULL);
```
Apache Moduleとして作成するときには注意点があるが、使用例の骨組みとしては上記。<br>
CAnalysisExecutor_PartialMatchは CAnalysisExecutor の派生クラスで、派生クラスの作り方は後ほど記述。<br>
CAnalysisExecutor_PartialMatchは指定の文字列"a"が、部分一致したときに置換文字"x"で置換する。<br>
<br>

**【CAnalysisExecutorの派生クラスの作り方】**<br>
CAnalysisExecutorの派生クラスは、メソッドをstaticで作成し、関数ポインタを親クラスに設定して作成する。<br>

**公開するクラス宣言structの作成の例: header.h**

```cpp
///publicの宣言
typedef struct CAnalysisExecutor_PartialMatch {
	CAnalysisExecutor parentMember;
} CAnalysisExecutor_PartialMatch;
```

**隠蔽するクラス宣言structと実装の作成の例: partial_match.c**

```cpp
///privateなクラス構造の宣言
typedef struct CAnalysisExecutor_PartialMatch_Super {
	CAnalysisExecutor_Super parentMember;
	//
	char* targetStr;
	...
} CAnalysisExecutor_PartialMatch_Super;

///デバッグ用のクラス内部の状態を出力する。
static void CAnalysisExecutor_PartialMatch__debugPrint(const CAnalysisExecutor* p_this) {
	...
}

///brigade処理の一番最初に呼ばれる。クラスの初期化などを行う。
static AcBool CAnalysisExecutor_PartialMatch_start(CAnalysisExecutor* p_this, const apr_table_t* table) {
	...
}

///置換開始位置を1文字進めた場合に呼ばれる。ここでは主に置換対象の開始位置を探すステータスになる。
static const AnalysisCommand CAnalysisExecutor_PartialMatch_forward(CAnalysisExecutor* p_this, 
	const AnalysisCommand cmd, const AcBool isRejectPreCmd, const char c) 
{
	...
}

///置換が実行された後に呼ばれる。置換後の処理をする。
static const AnalysisCommand CAnalysisExecutor_PartialMatch_replace(CAnalysisExecutor* p_this, 
	const AnalysisCommand cmd, AcBool result) 
{
	...
}

///指定の位置の文字を取得する場合に呼ばれる。
static const AnalysisCommand CAnalysisExecutor_PartialMatch_pos(CAnalysisExecutor* p_this, 
	const AnalysisCommand cmd, const char c) 
{
	...
}

///brigade処理の終わり（EOS）のときに呼ばれる。最後に文字列を追加することも指示できる。
static const AnalysisCommand CAnalysisExecutor_PartialMatch_end(CAnalysisExecutor* p_this, const AnalysisCommand cmd) {
	...
}

///デストラクタ
static void CAnalysisExecutor_PartialMatch_delete(void* p_this) {
	...
}

///コンストラクタ
CAnalysisExecutor_PartialMatch* CAnalysisExecutor_PartialMatch_new(const char* targetStr, const char* replaceStr) {
	CAnalysisExecutor_PartialMatch_Super* self = (CAnalysisExecutor_PartialMatch_Super*)malloc(sizeof(CAnalysisExecutor_PartialMatch_Super));
	if (self == NULL) return NULL;
	//親メンバーの初期化
	CAnalysisExecutor_init((CAnalysisExecutor*)self, gClassName_AnalysisExecutor_PartialMatch, 
		CAnalysisExecutor_PartialMatch_delete,
		CAnalysisExecutor_PartialMatch_start, 
		CAnalysisExecutor_PartialMatch_forward,
		CAnalysisExecutor_PartialMatch_replace,
		CAnalysisExecutor_PartialMatch_pos,
		CAnalysisExecutor_PartialMatch_end
		);
	...
	//デバッグ出力用の関数の設定
	CAnalysisExecutor_setDebugPrintFunc((CAnalysisExecutor*)self, CAnalysisExecutor_PartialMatch__debugPrint);
	return (CAnalysisExecutor_PartialMatch*)self;
}

```
<br>

**各メソッドの説明**<br>
CAnalysisExecutorは受動的なクラスで、イベントドリブンに近い動作をする。<br>
各メソッドがCAnalysisManagerにreturnで次の指示を出すと、指示に応じてCAnalysisExecutorのメソッドを呼び出してくれる。<br>

|**メソッド名**|**説明**|
|---|---|
|***CAnalysisExecutor_Xxxx_start()***| 処理の一番最初の開始時に呼ばれる。自身の初期化処理を実装する。|
|***CAnalysisExecutor_Xxxx_forward()***| 置換開始位置を1バイト進めたときに、読み込んだ1バイトを引数にして呼ばれる。<br>置換開始位置となりうるかを判別する処理を実装する。<br>置換開始となりうる場合、通常はposで次の1文字を読み込む指示を出す。|
|***CAnalysisExecutor_Xxxx_pos()***| 置換開始位置を移動させずに、指定の位置の1バイトを読み込んだときに呼ばれる。<br>読み込んだ文字が置換対象とマッチするかを検査する。マッチすれば置換指示を出す。<br>マッチせずさらに読み込みが必要な場合はさらにposで次の1文字を読み込む指示を出す。|
|***CAnalysisExecutor_Xxxx_replace()***| 置換実行後に呼ばれる。<br>通常は置換開始位置を1バイト進めて、置換開始位置をさがす処理に移行させる。|
|***CAnalysisExecutor_Xxxx_end()***| birgade処理が終わったときに呼ばれる。<br>HTTPボディの一番最後の位置に文字列を追加する処理などができる。|

<br>
<br>



