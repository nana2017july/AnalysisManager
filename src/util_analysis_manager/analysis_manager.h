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

#ifndef __BUCKET_CONTROLLER_H__
#define __BUCKET_CONTROLLER_H__


#include "apr_buckets.h"
#include "apr_hash.h"
#include "ac_this_is_class.h"



//------------------------------------------------------
//----    CBucketController Class       ---------------------
//------------------------------------------------------

/**
CBucketController_pos()などでreturn値に使用する。
*/
extern const int CBUCKET_CONTROLLER_ERR_POS;

/**
@brief brigade内の複数のbucketをbucket間の区切りを気にせずに扱うためのクラス。
@n brigade内の文字列をすべて連結したかのように取り扱う。
*/
typedef struct CBucketController {
	AcCThisIsClass* thisIsClass;
} CBucketController;

/**
@brief CBucketControllerコンストラクタ。
@n 生成したばかりだと置換開始位置は-1（0の一つ前の状態）なので、1つ進めてからpos()などを実行すること。
@return 生成したクラス。
*/
CBucketController* CBucketController_new(apr_pool_t* pool, apr_bucket_alloc_t* ba, apr_bucket_brigade* bb);

/**for debug*/
void CBucketController__debugPrint(const CBucketController* p_this, const char* file, int line);
#define CBUCKET_CONTROLLER_DEBUG_PRINT(p_this) CBucketController__debugPrint(p_this, __FILE__, __LINE__);

/**
@brief 現在位置を起点にした、指定の位置の文字を返す。
@param pos [in]位置を指定する。（0～）
@return 指定の位置の文字を1バイト返す。brigadeのエンドなどで文字が取得できなかった場合はCBUCKET_CONTROLLER_ERR_POSを返す。
*/
const int CBucketController_pos(const CBucketController* p_this, const size_t pos);

/**
@brief 現在位置を1文字前に進める
@return ENDBUFFER,EOSなどで1文字進められなかった場合FALSE。
*/
AcBool CBucketController_forward(CBucketController* p_this);

/**
@brief 置換を実行し、置換した文字列の位置までのbucketをmodifiedBrigade(修正後brigade)に移動する。
@n 置換対象は、現在位置から指定の長さの文字列。
@n ※現在位置が置換最後尾の文字に移動することに注意(一度forward()を呼び出さないと正しく動作しない)。
@param len [in]現在位置からの置換対象の長さ(0～)。
@param replaceStr [in]置換文字列
@return 置換に失敗したときFALSE。長さlenの文字列長を確保できなかった場合（addBrigade()する必要がある）や、
@n 初期状態で一度もforward()していないか、
@n 置換したばかりで位置が初期状態になっていてforward()が一度も呼ばれてないなど。
*/
AcBool CBucketController_execReplace(CBucketController* p_this, size_t len, const char* replaceStr);

/**
@brief 修正をかけたbrigadeを抽出する。中のbucketはなくなる。
*/
apr_bucket_brigade* CBucketController_exportModifiedBrigade(CBucketController* p_this);

/**
@brief 修正をかけたbrigadeを抽出し、指定のbrigadeに追加する。クラス内のbucketはなくなる。
*/
void CBucketController_exportModifiedBrigadeToBrigade(CBucketController* p_this, apr_bucket_brigade* outbb);

/**
@brief brigadeを追加する。
@param bb [in]追加するbrigade。中のbucketはすべて内部に移動する。bbは空のbrigadeが返る。
@n            bbは自分でdestroyすること。
*/
void CBucketController_addBrigade(CBucketController* p_this, apr_bucket_brigade* bb);

/**
@brief modifiedBrigade(変更後の保管バッファ)の一番後ろに指定の文字列を追加する。
@n    modifiedBrigadeの一番最後のbucketがEOSの場合は、EOSの前に追加する。
@param replaceStr [in]追加する文字列。
*/
void CBucketController_addStringToModifiedBrigade(CBucketController* p_this, const char* replaceStr);


/**
@brief 入力のbucketをすべてmodifiedBrigadeに移動する。終了処理に使用する関数。
*/
void CBucketController_moveAllInputBucketToModifiedBrigade(CBucketController* p_this);

/**
バッファ（inputBrigade）にEOSが含まれるか？
@return 含まれる場合TRUE。
*/
AcBool CBucketController_isContainingEos(const CBucketController* p_this);


/**
@brief カレントのbucketがEOSかどうかを返す。
@return EOSのときTRUE。
*/
AcBool CBucketController_isEOS(const CBucketController* p_this);


/**
@brief 入力バッファに保存したbucketが終了（すべて消費したか）に達したかを返す。
@return 終了に達したときにTRUE。
*/
AcBool CBucketController_isEndBuffer(const CBucketController* p_this);

/**
@brief 
*/
AcBool CBucketController_isInputEmpty(const CBucketController* p_this);
AcBool CBucketController_isModifiedEmpty(const CBucketController* p_this);


//------------------------------------------------------
//----    CAnalysisExecutor Class       ---------------------
//------------------------------------------------------

/**
@brief 解析実行用のコマンドの種類を表す。
*/
typedef enum AnalysisCommandType {
	///pos(指定位置の文字を取得する)を呼ぶコマンド（置換開始位置は移動しない）
	AC_POS,
	///forward(カレント位置を1文字進める)を呼ぶコマンド（置換開始位置を1バイト進める）
	AC_FORWARD,
	///forward(カレント位置をN(2以上)文字進める)を呼ぶコマンド（置換開始位置をNバイト進める）
	AC_FORWARD_N,
	///replcae(置換)を呼ぶコマンド
	AC_REPLACE, 
	///今後何も分析をせずにすべてbucketを素通りさせるコマンド
	AC_PASS_THROUGH_ALL,
	///終了を知らせる（Exceutor側は設定しないこと）
	AC_END
} AnalysisCommandType;


/**
@brief CAnalysisExecutorの関数に渡すステータスを表す。
*/
typedef enum AanalysisPosStatus {
	AC_POS_STATUS_OK,
	AC_POS_STATUS_EOS,
	AC_POS_STATUS_BUFFEREND
} AanalysisPosStatus;


/**
次に実施する操作を CAnalysisExecutor から指定するための情報の共用体。
@n #AnalysisCommandType 毎に使用する構造体が違う。
*/
typedef union AnalysisCommand {
	AnalysisCommandType type;

	struct {
		AnalysisCommandType type;
		size_t pos;
	} pos;

	struct {
		AnalysisCommandType type;
	} forward;

	struct {
		AnalysisCommandType type;
		///byte number for forward. it must be over 2. @~japanese 進める数。
		size_t len;
	} forward_n;

	struct {
		AnalysisCommandType type;
		size_t len;
		const char* replacement;
	} replace;

	struct {
		AnalysisCommandType type;
		///クライアントに返すHTTPステータス（-1のとき指定しない）
		size_t status;
		///最後に差し込む文字列。何も差し込まない場合はNULLを設定すること。
		const char* replacement;
	} end;
}AnalysisCommand;


/**
@brief 文字を解析し、置換等の実行をするクラスの親クラス。
@n CAnalysisManager 内部でから呼び出されるクラスだが、処理の実体はこのクラスにある。
@n メソッドの実装は派生クラスで定義される。
@n CAnalysisManager に設定する派生クラスを変えることで処理を変えられる。
@see #AnalysisCommandType
@see AnalysisCommand
@see CAnalysisManager_new()
*/
typedef struct CAnalysisExecutor {
	AcCThisIsClass* thisIsClass;
	//
} CAnalysisExecutor;

/**
@brief 派生クラスで継承するために呼び出す
*/
void CAnalysisExecutor_init(CAnalysisExecutor* p_this, const char* className, AcDeleteFunc_T deleteFunc,
	AcBool (*startFunc)(CAnalysisExecutor* p_this, const apr_table_t* table),
	const AnalysisCommand(*forwardFunc)(CAnalysisExecutor* p_this, const AnalysisCommand cmd, const AcBool isRejectPreCmd, const char c),
	const AnalysisCommand(*replaceFunc)(CAnalysisExecutor* p_this, const AnalysisCommand cmd, AcBool result),
	const AnalysisCommand(*posFunc)(CAnalysisExecutor* p_this, const AnalysisCommand cmd, const char c, const AcBool isEos),
	const AnalysisCommand(*endFunc)(CAnalysisExecutor* p_this, const AnalysisCommand cmd)
);

/**
@brief 処理が開始される前に呼ばれる。
@param table  [in]httpd.confの設定を渡す。設定値を受け取って初期化をする。
@return 現状、返り値によって何もしない。将来性のため。
*/
AcBool CAnalysisExecutor_startFunc(CAnalysisExecutor* p_this, const apr_table_t* table);

/**
@brief 1バイト進めたときに CAnalysisManager から呼ばれる。派生クラスで実装するvirtual関数。
@n     注意：前回指示した命令が拒否された場合も呼ばれる。拒否された場合は内部の状態をリセットして現在位置から改ためてforward処理を開始すること。
@param cmd [in]実行した命令(AC_FORWARD)。
@param isRejectPreCmd [in]前回の命令が拒否され、クラス内部の状態をリセットしてほしい場合TRUE。
@param c   [in]取得した文字
@return 次に CAnalysisManager に実行させる内容を指示する。
*/
const AnalysisCommand CAnalysisExecutor_forwardFunc(CAnalysisExecutor* p_this, const AnalysisCommand cmd, 
	const AcBool isRejectPreCmd, const char c);

/**
@brief  置換を実行したときに CAnalysisManager から呼ばれる。派生クラスで実装するvirtual関数。
@param cmd    [in]実行した命令(AC_REPLACE)。
@param result [in]置換が成功したかどうか。成功した場合true。
@param 
@return 次に CAnalysisManager に実行させる内容を指示する。
*/
const AnalysisCommand CAnalysisExecutor_replaceFunc(CAnalysisExecutor* p_this, const AnalysisCommand cmd,
	AcBool result);

/**
@brief  カレント位置を起点とした指定の位置の1バイトを取得したときに CAnalysisManager から呼ばれる。派生クラスで実装するvirtual関数。
@param cmd [in]実行した命令（AC_POS)。
@param c   [in]取得した文字（1バイト）。isEnd = true の場合はcに何が入っているかわからないので注意。
@param isEos [in]指定の位置がEOSだった場合true。
@return 次に CAnalysisManager に実行させる内容を指示する。
*/
const AnalysisCommand CAnalysisExecutor_posFunc(CAnalysisExecutor* p_this, const AnalysisCommand cmd,
	const char c, const AcBool isEos);

/**
@brief  最後に CAnalysisManager から呼ばれる。派生クラスで実装するvirtual関数。
@param cmd  [in]最後に実行した命令。
@return 次に CAnalysisManager に実行させる内容を指示する。
*/
const AnalysisCommand CAnalysisExecutor_endFunc(CAnalysisExecutor* p_this, const AnalysisCommand cmd);

/**
@brief デバッグ用の内部データ出力関数を登録する。 CAnalysisExecutor の派生クラスから呼び出し、
@n     親クラスで使用する。 CAnalysisExecutor__debugPrint() を呼び出したときに親クラスの内部情報は
@n     親クラスが出力し、子クラスの内部情報は親クラスがここで登録した関数を呼び出し、親＋子の情報を出力する。
*/
void CAnalysisExecutor_setDebugPrintFunc(CAnalysisExecutor* p_this, void (*debugPrintFunc)(const CAnalysisExecutor* p_this));

/**
@brief CAnalysisExecutor クラス情報のデバッグ用の出力関数。必ず CANALYSIS_EXECUTOR_DEBUG_PRINT() マクロを使用すること。
*/
void CAnalysisExecutor__debugPrint(const CAnalysisExecutor* p_this, const char* file, int line);
#define CANALYSIS_EXECUTOR_DEBUG_PRINT(p_this) CAnalysisExecutor__debugPrint(p_this, __FILE__, __LINE__);




//------------------------------------------------------
//----    CAnalysisManager Class       ---------------------
//------------------------------------------------------

/**
@brief 文字を解析し、置換等の操作を管理するクラスの親クラス(派生クラスなし)
@detail 【仕様】
@n 主に以下の概念がある。
@n ・置換開始位置・・・現在の置換開始位置。前に１バイトずつ進めることができる。ただし、後ろには進めない。
@n ・読み込み位置・・・置換開始位置を移動せず、指定の位置の文字を１バイト読み込む。
@n ・置換　　　　・・・置換開始位置から指定のバイト数を削除し、置換文字で置き替える。
@n 上記の機能を使用して置換等の処理をする。
@n 内部に CBucketController を保持してbucketの操作をしているため、このクラスも参照のこと。
@n
@n 【動作の実装】
@n   置換などの処理を実装する場合は、 CAnalysisExecutor クラスの派生クラスを作り、このクラスに設定する。
@n   このとき、 CAnalysisExecutor はステータス（ #AnalysisCommandType )に応じて CAnalysisManager から呼ばれるメソッドがある。
@n   呼ばれるメソッドを実装することで自由に動作を変えられる。用意されたメソッド（ステータス）は以下の通り。
@n   ・forward　　・・・置換開始位置を1バイト進める。
@n   ・pos    　　・・・置換開始位置を変えずに、指定の位置の1バイトを読み取る。位置は0～で指定。0は現在の置換開始位置。
@n   ・replace　　・・・置換が行われた直後に呼ばれる。
@n   ・start  　　・・・処理が開始したときに呼ばれる。
@n   ・end        ・・・処理が終了したときに呼ばれる。現在位置が、bucketのEOSになったときに終了となる。
@n
@n 【その他】速度：20MB/secくらいの速度。遅い！


@~english 
@brief A class to analyze strings , and do something (replaceing, counting , and so on.)
*/
typedef struct CAnalysisManager {
	AcCThisIsClass* thisIsClass;
	//
} CAnalysisManager;


/**public:
デバッグ用のPrint関数。クラス内部の値を標準出力する。
*/
void CAnalysisManager__debugPrint(const CAnalysisManager* p_this, const char* file, int line);
#define CANALYSIS_MANAGER_DEBUG_PRINT(p_this) CAnalysisManager__debugPrint(p_this, __FILE__, __LINE__);

/**
@brief 解析を実行する。Apacheからフックで呼ばれるごとに１度呼び出す。
@n  返ってきたときは、次のbrigadeが欲しくなったか、EOSを迎えたとき。EOSになっていないか確認のこと。
@param inputBrigade [in]入力のbrigae。
@return 改変済みのbrigade。brigadeは空のときもある。
@see CAnalysisManager_isEnd 
*/
apr_bucket_brigade* CAnalysisManager_run(CAnalysisManager* p_this, apr_bucket_brigade* inputBrigade);

/**
@brief 終了状態かどうか。
*/
AcBool CAnalysisManager_isEnd(const CAnalysisManager* p_this);

/**
@brief コンストラクタ。クラスを生成する。
@param executor [in]分析・処理の実行をするクラス。デストラクト（破壊）はこのクラスが行うので外部で行わないこと。
@param pool [in]プール
@param ba   [in]bucketのアロケータ
@param table [in]設定ファイルの内容
@param bb   [in]入力のbrigade
*/
CAnalysisManager* CAnalysisManager_new(CAnalysisExecutor* executor, apr_pool_t* pool, apr_bucket_alloc_t* ba, apr_table_t* table, apr_bucket_brigade* bb);


#endif  //end __BUCKET_CONTROLLER_H__
