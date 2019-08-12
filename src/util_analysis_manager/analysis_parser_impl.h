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

#ifndef __ANALYSIS_PARSER_IMPL_H__
#define __ANALYSIS_PARSER_IMPL_H__


#include "analysis_parser.h"





//------------------------------------------------------
//----    CAnalysisParser_split Class       ---------------------
//------------------------------------------------------
/**
@~japanese
@brief �w��̋�؂蕶�������������N���X�B��؂蕶������������ʒu��isEnd=true�ƂȂ�B
@n     ��؂蕶����������܂ŕ����������ɃX�g�b�N����B�ݒ肵���X�g�b�N�̃T�C�Y�𒴂����ꍇ�ł��p�[�X�͑�����B

@~english
@brief A class that searches delimitar string.It becomes isEnd = true at the position where it finds the delimiter string.
@n Stock strings internally until it finds a delimiter. It continues parsing even if over stock size.
*/
typedef struct {
	CAnalysisParser parentMember;
} CAnalysisParser_Split;


/**
@~japanese
@brief �R���X�g���N�^�B
@param targetStr [in]��؂蕶����B

@~english
@brief Constructor.
@param targetStr [in]target delimiter string.
*/
CAnalysisParser_Split* CAnalysisParser_Split_new(const char* delimiterStr, size_t stockStrSize);


/**
@~japanese
@brief �X�g�b�N������������擾����B
@param str [out]�X�g�b�N����������̃|�C���^���Z�b�g���ĕԂ��B
@param len [out]�X�g�b�N����������̒������Z�b�g���ĕԂ��B

@~english
@brief Get the stocked string.
@ param str [out] Set and return a pointer to the stocked string.
@ param len [out] Set and return the length of the stocked string.
*/
void CAnalysisParser_Split_getStockStr(CAnalysisParser_Split* p_this, const char** str, size_t* len);



//------------------------------------------------------
//----    CAnalysisParser_CaseInsensitive Class       ---------------------
//------------------------------------------------------
/**
@~japanese
@brief �啶�����������C�ɂ����Ɍ������A�w��̕����񂪌����������ǂ������`�F�b�N����N���X�B
@n     ���݈ʒu����}�b�`���J�n����̂ŁA������v����������ꍇ�A�}�b�`���Ȃ������s�x�A���Z�b�g������K�v������B

@~english
@brief A class that searches case-insensitively and checks if the specified string is found.
@n   Starting with current position, So if you want to do partial matching, you neet to reset at unmatched.
*/
typedef struct {
	CAnalysisParser parentMember;
} CAnalysisParser_CaseInsensitive;


/**
@~japanese
@brief �R���X�g���N�^�B
@param targetStr [in]�����Ώۂ̕�����B

@~english
@brief Constructor.
@param targetStr [in]target string for search.
*/
CAnalysisParser_CaseInsensitive* CAnalysisParser_CaseInsensitive_new(const char* targetStr);







//------------------------------------------------------
//----    CAnalysisParser_HttpHeader Class       ---------------------
//------------------------------------------------------
/**
@~japanese
@brief Http�w�b�_�̌`���Ŏw��̑������������邩��͂���B���������ꍇ�̓X�g�b�N�ɒl���R�s�[����B
@n   �w��̃X�g�b�N�T�C�Y�𒴂��ăR�s�[�͂��Ȃ��B
@n   �����Ƀ}�b�`���O���J�n�����̂ŁA�s�̐擪�ʒu�Ń��Z�b�g����K�v������B

@~english
@brief Analyzes whether the specified attribute name is found in the form of Http header. If found, copy the value to stock string.
@n Do not copy beyond the specified stock size.
@n Starting matching soon, it should reset at top of the line. 
*/
typedef struct {
	CAnalysisParser parentMember;
} CAnalysisParser_HttpHeader;


/**
@~japanese
@brief �R���X�g���N�^�B
@param headerName [in]HTTP�w�b�_�������B(��F"Content-Disposition:")
@param stockStrSize [in]�X�g�b�N�T�C�Y�B�w�b�_�����l�̃R�s�[��o�b�t�@�̍ő�T�C�Y�B

@~english
@brief Constructor.
@param headearName [in]HTTP header attribute name .(ex."Content-Disposition:")
@param stockStrSize [in]Stock size. Max size of copying HTTP header value.
*/
CAnalysisParser_HttpHeader* CAnalysisParser_HttpHeader_new(const char* headearName, size_t stockStrSize);

/**
@~japanese
@brief �X�g�b�N������������擾����B
@param str [out]�X�g�b�N����������̃|�C���^���Z�b�g���ĕԂ��B
@param len [out]�X�g�b�N����������̒������Z�b�g���ĕԂ��B

@~english
@brief Get the stocked string.
@ param str [out] Set and return a pointer to the stocked string.
@ param len [out] Set and return the length of the stocked string.
*/
void CAnalysisParser_HttpHeader_getValue(CAnalysisParser_HttpHeader* p_this, const char** str, size_t* len);







//------------------------------------------------------
//----    CAnalysisParser_LineUp Class       ---------------------
//------------------------------------------------------
/**
@~japanese
@brief CAnalysisParser �����ԂɎ��s����N���X�B�����̃p�[�T������ɕێ����AisEnd��true�ɂȂ����玟�̃p�[�T�����s����B
@n     �Ō�̃p�[�T��isEnd��true�ɂȂ������_�ŁA���̃p�[�T��isEnd��true�ɂȂ�B

@~english
@brief Class that executes CAnalysisParser clsses in order. Hold multiple parsers internally, and execute the next parser when isEnd becomes true.
@n     When the last parser is isEnd = true, this parser is also isEnd = true.
*/
typedef struct {
	CAnalysisParser parentMember;
} CAnalysisParser_LineUp;


/**
@~japanese
@brief �R���X�g���N�^�B
@param targetStr [in]�����Ώۂ̕�����B

@~english
@brief Constructor.
@param targetStr [in]target string for search.
*/
CAnalysisParser_LineUp* CAnalysisParser_LineUp_new(const char* targetStr);






#endif  //end __ANALYSIS_PARSER_IMPL_H__