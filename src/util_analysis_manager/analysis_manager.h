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
CBucketController_pos()�Ȃǂ�return�l�Ɏg�p����B
*/
extern const int CBUCKET_CONTROLLER_ERR_POS;

/**
@brief brigade���̕�����bucket��bucket�Ԃ̋�؂���C�ɂ����Ɉ������߂̃N���X�B
@n brigade���̕���������ׂĘA���������̂悤�Ɏ�舵���B
*/
typedef struct CBucketController {
	AcCThisIsClass* thisIsClass;
} CBucketController;

/**
@brief CBucketController�R���X�g���N�^�B
@n ���������΂��肾�ƒu���J�n�ʒu��-1�i0�̈�O�̏�ԁj�Ȃ̂ŁA1�i�߂Ă���pos()�Ȃǂ����s���邱�ƁB
@return ���������N���X�B
*/
CBucketController* CBucketController_new(apr_pool_t* pool, apr_bucket_alloc_t* ba, apr_bucket_brigade* bb);

/**for debug*/
void CBucketController__debugPrint(const CBucketController* p_this, const char* file, int line);
#define CBUCKET_CONTROLLER_DEBUG_PRINT(p_this) CBucketController__debugPrint(p_this, __FILE__, __LINE__);

/**
@brief ���݈ʒu���N�_�ɂ����A�w��̈ʒu�̕�����Ԃ��B
@param pos [in]�ʒu���w�肷��B�i0�`�j
@return �w��̈ʒu�̕�����1�o�C�g�Ԃ��Bbrigade�̃G���h�Ȃǂŕ������擾�ł��Ȃ������ꍇ��CBUCKET_CONTROLLER_ERR_POS��Ԃ��B
*/
const int CBucketController_pos(const CBucketController* p_this, const size_t pos);

/**
@brief ���݈ʒu��1�����O�ɐi�߂�
@return ENDBUFFER,EOS�Ȃǂ�1�����i�߂��Ȃ������ꍇFALSE�B
*/
AcBool CBucketController_forward(CBucketController* p_this);

/**
@brief �u�������s���A�u������������̈ʒu�܂ł�bucket��modifiedBrigade(�C����brigade)�Ɉړ�����B
@n �u���Ώۂ́A���݈ʒu����w��̒����̕�����B
@n �����݈ʒu���u���Ō���̕����Ɉړ����邱�Ƃɒ���(��xforward()���Ăяo���Ȃ��Ɛ��������삵�Ȃ�)�B
@param len [in]���݈ʒu����̒u���Ώۂ̒���(0�`)�B
@param replaceStr [in]�u��������
@return �u���Ɏ��s�����Ƃ�FALSE�B����len�̕����񒷂��m�ۂł��Ȃ������ꍇ�iaddBrigade()����K�v������j��A
@n ������Ԃň�x��forward()���Ă��Ȃ����A
@n �u�������΂���ňʒu��������ԂɂȂ��Ă���forward()����x���Ă΂�ĂȂ��ȂǁB
*/
AcBool CBucketController_execReplace(CBucketController* p_this, size_t len, const char* replaceStr);

/**
@brief �C����������brigade�𒊏o����B����bucket�͂Ȃ��Ȃ�B
*/
apr_bucket_brigade* CBucketController_exportModifiedBrigade(CBucketController* p_this);

/**
@brief �C����������brigade�𒊏o���A�w���brigade�ɒǉ�����B�N���X����bucket�͂Ȃ��Ȃ�B
*/
void CBucketController_exportModifiedBrigadeToBrigade(CBucketController* p_this, apr_bucket_brigade* outbb);

/**
@brief brigade��ǉ�����B
@param bb [in]�ǉ�����brigade�B����bucket�͂��ׂē����Ɉړ�����Bbb�͋��brigade���Ԃ�B
@n            bb�͎�����destroy���邱�ƁB
*/
void CBucketController_addBrigade(CBucketController* p_this, apr_bucket_brigade* bb);

/**
@brief modifiedBrigade(�ύX��̕ۊǃo�b�t�@)�̈�Ԍ��Ɏw��̕������ǉ�����B
@n    modifiedBrigade�̈�ԍŌ��bucket��EOS�̏ꍇ�́AEOS�̑O�ɒǉ�����B
@param replaceStr [in]�ǉ����镶����B
*/
void CBucketController_addStringToModifiedBrigade(CBucketController* p_this, const char* replaceStr);


/**
@brief ���͂�bucket�����ׂ�modifiedBrigade�Ɉړ�����B�I�������Ɏg�p����֐��B
*/
void CBucketController_moveAllInputBucketToModifiedBrigade(CBucketController* p_this);

/**
�o�b�t�@�iinputBrigade�j��EOS���܂܂�邩�H
@return �܂܂��ꍇTRUE�B
*/
AcBool CBucketController_isContainingEos(const CBucketController* p_this);


/**
@brief �J�����g��bucket��EOS���ǂ�����Ԃ��B
@return EOS�̂Ƃ�TRUE�B
*/
AcBool CBucketController_isEOS(const CBucketController* p_this);


/**
@brief ���̓o�b�t�@�ɕۑ�����bucket���I���i���ׂď�������j�ɒB��������Ԃ��B
@return �I���ɒB�����Ƃ���TRUE�B
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
@brief ��͎��s�p�̃R�}���h�̎�ނ�\���B
*/
typedef enum AnalysisCommandType {
	///pos(�w��ʒu�̕������擾����)���ĂԃR�}���h�i�u���J�n�ʒu�͈ړ����Ȃ��j
	AC_POS,
	///forward(�J�����g�ʒu��1�����i�߂�)���ĂԃR�}���h�i�u���J�n�ʒu��1�o�C�g�i�߂�j
	AC_FORWARD,
	///forward(�J�����g�ʒu��N(2�ȏ�)�����i�߂�)���ĂԃR�}���h�i�u���J�n�ʒu��N�o�C�g�i�߂�j
	AC_FORWARD_N,
	///replcae(�u��)���ĂԃR�}���h
	AC_REPLACE, 
	///���㉽�����͂������ɂ��ׂ�bucket��f�ʂ肳����R�}���h
	AC_PASS_THROUGH_ALL,
	///�I����m�点��iExceutor���͐ݒ肵�Ȃ����Ɓj
	AC_END
} AnalysisCommandType;


/**
@brief CAnalysisExecutor�̊֐��ɓn���X�e�[�^�X��\���B
*/
typedef enum AanalysisPosStatus {
	AC_POS_STATUS_OK,
	AC_POS_STATUS_EOS,
	AC_POS_STATUS_BUFFEREND
} AanalysisPosStatus;


/**
���Ɏ��{���鑀��� CAnalysisExecutor ����w�肷�邽�߂̏��̋��p�́B
@n #AnalysisCommandType ���Ɏg�p����\���̂��Ⴄ�B
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
		///byte number for forward. it must be over 2. @~japanese �i�߂鐔�B
		size_t len;
	} forward_n;

	struct {
		AnalysisCommandType type;
		size_t len;
		const char* replacement;
	} replace;

	struct {
		AnalysisCommandType type;
		///�N���C�A���g�ɕԂ�HTTP�X�e�[�^�X�i-1�̂Ƃ��w�肵�Ȃ��j
		size_t status;
		///�Ō�ɍ������ޕ�����B�����������܂Ȃ��ꍇ��NULL��ݒ肷�邱�ƁB
		const char* replacement;
	} end;
}AnalysisCommand;


/**
@brief ��������͂��A�u�����̎��s������N���X�̐e�N���X�B
@n CAnalysisManager �����ł���Ăяo�����N���X�����A�����̎��̂͂��̃N���X�ɂ���B
@n ���\�b�h�̎����͔h���N���X�Œ�`�����B
@n CAnalysisManager �ɐݒ肷��h���N���X��ς��邱�Ƃŏ�����ς�����B
@see #AnalysisCommandType
@see AnalysisCommand
@see CAnalysisManager_new()
*/
typedef struct CAnalysisExecutor {
	AcCThisIsClass* thisIsClass;
	//
} CAnalysisExecutor;

/**
@brief �h���N���X�Ōp�����邽�߂ɌĂяo��
*/
void CAnalysisExecutor_init(CAnalysisExecutor* p_this, const char* className, AcDeleteFunc_T deleteFunc,
	AcBool (*startFunc)(CAnalysisExecutor* p_this, const apr_table_t* table),
	const AnalysisCommand(*forwardFunc)(CAnalysisExecutor* p_this, const AnalysisCommand cmd, const AcBool isRejectPreCmd, const char c),
	const AnalysisCommand(*replaceFunc)(CAnalysisExecutor* p_this, const AnalysisCommand cmd, AcBool result),
	const AnalysisCommand(*posFunc)(CAnalysisExecutor* p_this, const AnalysisCommand cmd, const char c, const AcBool isEos),
	const AnalysisCommand(*endFunc)(CAnalysisExecutor* p_this, const AnalysisCommand cmd)
);

/**
@brief �������J�n�����O�ɌĂ΂��B
@param table  [in]httpd.conf�̐ݒ��n���B�ݒ�l���󂯎���ď�����������B
@return ����A�Ԃ�l�ɂ���ĉ������Ȃ��B�������̂��߁B
*/
AcBool CAnalysisExecutor_startFunc(CAnalysisExecutor* p_this, const apr_table_t* table);

/**
@brief 1�o�C�g�i�߂��Ƃ��� CAnalysisManager ����Ă΂��B�h���N���X�Ŏ�������virtual�֐��B
@n     ���ӁF�O��w���������߂����ۂ��ꂽ�ꍇ���Ă΂��B���ۂ��ꂽ�ꍇ�͓����̏�Ԃ����Z�b�g���Č��݈ʒu��������߂�forward�������J�n���邱�ƁB
@param cmd [in]���s��������(AC_FORWARD)�B
@param isRejectPreCmd [in]�O��̖��߂����ۂ���A�N���X�����̏�Ԃ����Z�b�g���Ăق����ꍇTRUE�B
@param c   [in]�擾��������
@return ���� CAnalysisManager �Ɏ��s��������e���w������B
*/
const AnalysisCommand CAnalysisExecutor_forwardFunc(CAnalysisExecutor* p_this, const AnalysisCommand cmd, 
	const AcBool isRejectPreCmd, const char c);

/**
@brief  �u�������s�����Ƃ��� CAnalysisManager ����Ă΂��B�h���N���X�Ŏ�������virtual�֐��B
@param cmd    [in]���s��������(AC_REPLACE)�B
@param result [in]�u���������������ǂ����B���������ꍇtrue�B
@param 
@return ���� CAnalysisManager �Ɏ��s��������e���w������B
*/
const AnalysisCommand CAnalysisExecutor_replaceFunc(CAnalysisExecutor* p_this, const AnalysisCommand cmd,
	AcBool result);

/**
@brief  �J�����g�ʒu���N�_�Ƃ����w��̈ʒu��1�o�C�g���擾�����Ƃ��� CAnalysisManager ����Ă΂��B�h���N���X�Ŏ�������virtual�֐��B
@param cmd [in]���s�������߁iAC_POS)�B
@param c   [in]�擾���������i1�o�C�g�j�BisEnd = true �̏ꍇ��c�ɉ��������Ă��邩�킩��Ȃ��̂Œ��ӁB
@param isEos [in]�w��̈ʒu��EOS�������ꍇtrue�B
@return ���� CAnalysisManager �Ɏ��s��������e���w������B
*/
const AnalysisCommand CAnalysisExecutor_posFunc(CAnalysisExecutor* p_this, const AnalysisCommand cmd,
	const char c, const AcBool isEos);

/**
@brief  �Ō�� CAnalysisManager ����Ă΂��B�h���N���X�Ŏ�������virtual�֐��B
@param cmd  [in]�Ō�Ɏ��s�������߁B
@return ���� CAnalysisManager �Ɏ��s��������e���w������B
*/
const AnalysisCommand CAnalysisExecutor_endFunc(CAnalysisExecutor* p_this, const AnalysisCommand cmd);

/**
@brief �f�o�b�O�p�̓����f�[�^�o�͊֐���o�^����B CAnalysisExecutor �̔h���N���X����Ăяo���A
@n     �e�N���X�Ŏg�p����B CAnalysisExecutor__debugPrint() ���Ăяo�����Ƃ��ɐe�N���X�̓�������
@n     �e�N���X���o�͂��A�q�N���X�̓������͐e�N���X�������œo�^�����֐����Ăяo���A�e�{�q�̏����o�͂���B
*/
void CAnalysisExecutor_setDebugPrintFunc(CAnalysisExecutor* p_this, void (*debugPrintFunc)(const CAnalysisExecutor* p_this));

/**
@brief CAnalysisExecutor �N���X���̃f�o�b�O�p�̏o�͊֐��B�K�� CANALYSIS_EXECUTOR_DEBUG_PRINT() �}�N�����g�p���邱�ƁB
*/
void CAnalysisExecutor__debugPrint(const CAnalysisExecutor* p_this, const char* file, int line);
#define CANALYSIS_EXECUTOR_DEBUG_PRINT(p_this) CAnalysisExecutor__debugPrint(p_this, __FILE__, __LINE__);




//------------------------------------------------------
//----    CAnalysisManager Class       ---------------------
//------------------------------------------------------

/**
@brief ��������͂��A�u�����̑�����Ǘ�����N���X�̐e�N���X(�h���N���X�Ȃ�)
@detail �y�d�l�z
@n ��Ɉȉ��̊T�O������B
@n �E�u���J�n�ʒu�E�E�E���݂̒u���J�n�ʒu�B�O�ɂP�o�C�g���i�߂邱�Ƃ��ł���B�������A���ɂ͐i�߂Ȃ��B
@n �E�ǂݍ��݈ʒu�E�E�E�u���J�n�ʒu���ړ������A�w��̈ʒu�̕������P�o�C�g�ǂݍ��ށB
@n �E�u���@�@�@�@�E�E�E�u���J�n�ʒu����w��̃o�C�g�����폜���A�u�������Œu���ւ���B
@n ��L�̋@�\���g�p���Ēu�����̏���������B
@n ������ CBucketController ��ێ�����bucket�̑�������Ă��邽�߁A���̃N���X���Q�Ƃ̂��ƁB
@n
@n �y����̎����z
@n   �u���Ȃǂ̏�������������ꍇ�́A CAnalysisExecutor �N���X�̔h���N���X�����A���̃N���X�ɐݒ肷��B
@n   ���̂Ƃ��A CAnalysisExecutor �̓X�e�[�^�X�i #AnalysisCommandType )�ɉ����� CAnalysisManager ����Ă΂�郁�\�b�h������B
@n   �Ă΂�郁�\�b�h���������邱�ƂŎ��R�ɓ����ς�����B�p�ӂ��ꂽ���\�b�h�i�X�e�[�^�X�j�͈ȉ��̒ʂ�B
@n   �Eforward�@�@�E�E�E�u���J�n�ʒu��1�o�C�g�i�߂�B
@n   �Epos    �@�@�E�E�E�u���J�n�ʒu��ς����ɁA�w��̈ʒu��1�o�C�g��ǂݎ��B�ʒu��0�`�Ŏw��B0�͌��݂̒u���J�n�ʒu�B
@n   �Ereplace�@�@�E�E�E�u�����s��ꂽ����ɌĂ΂��B
@n   �Estart  �@�@�E�E�E�������J�n�����Ƃ��ɌĂ΂��B
@n   �Eend        �E�E�E�������I�������Ƃ��ɌĂ΂��B���݈ʒu���Abucket��EOS�ɂȂ����Ƃ��ɏI���ƂȂ�B
@n
@n �y���̑��z���x�F20MB/sec���炢�̑��x�B�x���I


@~english 
@brief A class to analyze strings , and do something (replaceing, counting , and so on.)
*/
typedef struct CAnalysisManager {
	AcCThisIsClass* thisIsClass;
	//
} CAnalysisManager;


/**public:
�f�o�b�O�p��Print�֐��B�N���X�����̒l��W���o�͂���B
*/
void CAnalysisManager__debugPrint(const CAnalysisManager* p_this, const char* file, int line);
#define CANALYSIS_MANAGER_DEBUG_PRINT(p_this) CAnalysisManager__debugPrint(p_this, __FILE__, __LINE__);

/**
@brief ��͂����s����BApache����t�b�N�ŌĂ΂�邲�ƂɂP�x�Ăяo���B
@n  �Ԃ��Ă����Ƃ��́A����brigade���~�����Ȃ������AEOS���}�����Ƃ��BEOS�ɂȂ��Ă��Ȃ����m�F�̂��ƁB
@param inputBrigade [in]���͂�brigae�B
@return ���ύς݂�brigade�Bbrigade�͋�̂Ƃ�������B
@see CAnalysisManager_isEnd 
*/
apr_bucket_brigade* CAnalysisManager_run(CAnalysisManager* p_this, apr_bucket_brigade* inputBrigade);

/**
@brief �I����Ԃ��ǂ����B
*/
AcBool CAnalysisManager_isEnd(const CAnalysisManager* p_this);

/**
@brief �R���X�g���N�^�B�N���X�𐶐�����B
@param executor [in]���́E�����̎��s������N���X�B�f�X�g���N�g�i�j��j�͂��̃N���X���s���̂ŊO���ōs��Ȃ����ƁB
@param pool [in]�v�[��
@param ba   [in]bucket�̃A���P�[�^
@param table [in]�ݒ�t�@�C���̓��e
@param bb   [in]���͂�brigade
*/
CAnalysisManager* CAnalysisManager_new(CAnalysisExecutor* executor, apr_pool_t* pool, apr_bucket_alloc_t* ba, apr_table_t* table, apr_bucket_brigade* bb);


#endif  //end __BUCKET_CONTROLLER_H__
