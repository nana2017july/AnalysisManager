# �{�f�B���𕪐́E�u������Apache Module�p���C�u����
���Apache Module�ȂǂŎg�p���邱�Ƃ��C���[�W���č쐬���Ă��邪�A��{�I�ɂ�APR�̕⏕���C�u�����B<br>
apr_bucket_brigade�́A�������A���ň����Ȃ���肪����AApache Module�̃t�b�N�֐����f���I�ɌĂ΂��B<br>
�t�b�N�֐���1��̃��N�G�X�g�������̓��X�|���X�ł��ꂼ��1�x�����Ă΂��Ɗ��Ⴂ����Ă�������������Ǝv����B<br>
�������A���ۂɂ́AHTTP�{�f�B���𕪊�����briagde�ɓ���āA���x���t�b�N�֐����Ă΂�邽�߁A�����񂪒f������B<br>
�Ⴆ�΁A"012345678"�Ƃ���HTTP�{�f�B�����������ꍇ�A1�x�ڂ̓t�b�N�֐���"012"��brigade�œn����A<br>
2�x�ڂ̓t�b�N�֐���"345678"���n�����Ƃ�����ł���B<br>
�������̏ꍇ��"23"�Ƃ�����������������悤�Ƃ���ƁA"2"�œr�؂�Ă��܂��̂Ńt�b�N�֐��͎��ɌĂ΂ꂽ�Ƃ��ɂ���Ă���brigade��
"3"���܂܂�Ă��邩���`�F�b�N���Ȃ���΂Ȃ�Ȃ��B<br>
����͂��Ȃ����B<br>
<br>
���̃��C�u������brigade�̒f�������C�ɂ����ɏ������s����A�֗��ȃc�[���ł���B<br>
<br>

## ���̃��C�u�������ł��邱��
�h���N���X���쐬�����brigade�̒f�������C�ɂ����ɁA�ȉ��̂��Ƃ��ł��܂��B<br>
* HTTP�{�f�B������̌���
* HTTP�{�f�B������̒u��
* ���C�u�������g�p����������v�AHTML�^�O�̒u��������Apache Module�T���v��

�񋟂��Ă���Apaceh Module�́A���C�u�������g�p�����T���v���ł��B
<br>

## �͂��߂�
���C�Z���X�́AApache License, Version 2.0�Ƃ��܂��B<br>
<br>
**�y�֐��E�N���X�̃h�L�������g�idoxygen�j�z**<br>
<!--https://nana2017july.github.io/html_parser/index.html--><br> 
<br>


## �ύX����
ver.1.0 �V�K�쐬<br>

## �����
**�y�R���p�C���z**<br>
�ȉ��̃R���p�C�����Ŏg�p�ł��邱�Ƃ������ɂ��܂����B<br>
* Microsoft Visual Studio 2019 
* CentOS7 g++(4.8.5)
<br>
<br>

**�y�g�p�������́z**<br>
* Apache Httpd (2.4.39(Win64)�A2.4.6(CentOS) )
* C����

Apache2.4���e���̊��ɃC���X�g�[�����Ă������ƁB<br>
<br>
<br>

**�yMicrosoft Visual Studio �̃r���h�̕⑫�z**<br>
Microsoft Visual Studio �̏ꍇ�A�ȉ��̂��Ƃ����Ă���r���h�����Ă��������B
* �umod_replace_content.sln�v���N��
* �v���W�F�N�g�̐ݒ���J���A�uC/C++�v�ˁu�ǉ��̃C���N���[�h �f�B���N�g���v��Apache Httpd�t�H���_����includes�t�H���_��ǋL�B
* �v���W�F�N�g�̐ݒ���J���A�u�����J�[�v�ˁu�ǉ��̃��C�u���� �f�B���N�g���v��Apache Httpd�t�H���_����libs�t�H���_��ǋL�B
* �v���W�F�N�g�̐ݒ���J���A�u�f�o�b�O�v�ˁu���v�ɁuPATH=%PATH%;Apache Httpd�t�H���_����bin�t�H���_�v��ǋL

�܂��A�r���h��́A�쐬���ꂽ�umod_replace_content.so�v��Apache Httpd�t�H���_��modules�ɃR�s�[���āAhttpd.conf�ɐݒ��ǋL���Ă���
httpd���N�����Ă��������B<br>
httpd.conf�̏������͌�قǎ����܂��B<br>
<br>
�@���e�X�g�𑖂点�����ꍇ�́A�utest\test_am\test_am.sln�v���N�����āA��L�Ɠ����ݒ�����܂��B
<br>

**�yCentOS �̃r���h�̕⑫�z**<br>
CentOS �̏ꍇ�A�ȉ��̎菇�Ńr���h�����Ă��������B
* common.mk���J���A�擪��APR_LIB_DIR �Ȃǂ����g�̊��ɂ����Ă��邩���m�F���A�K�v�ł���ΏC������B
* make test_am �����s���A�G���[��0���ł��邱�Ƃ��m�F�B
* make all
* make install

��L��Apache Module�փR�s�[����Ă���̂ŁAhttpd.conf�ɐݒ��ǋL���Ă���httpd���N�����Ă��������B<br>
httpd.conf�̏������͌�قǎ����܂��B<br>

**�p�ӂ���Ă���^�[�Q�b�g**<br>

|**�^�[�Q�b�g**|**����**|
|---|---|
|make all |Apache Module�̃T���v�����r���h����Bmod_replace_content.so���쐬�����B|
|make clean|.o�Ȃǂ̐����t�@�C�������ׂč폜����B|
|make test_am|CAnalysisManager�֘A�̃e�X�g���r���h���Ď��s����B|
|make check_apxs_vars|makefile���Œ�`����Ă���apxs�n�̒l��\������B|
|make install|��������.so��Apache Module�ɃC���X�g�[������B|
|make start|httpd���J�n����B|
|make restart|httpd�����X�^�[�g����B|
|make stop|httpd���~����B|

<br>


## Apaceh Module�T���v���𗘗p���邽�߂�httpd.conf�L�q
```Apache
# Load created module
LoadModule replace_content_module  modules/mod_replace_content.so
# Output filter setting
SetOutputFilter REPLACE_CONTENT_OUTPUTFILTER
Header set Last-Modified "Sat, 19 Apr 2014 21:53:07 GMT"
# replace content setting
ReplaceContent "tag:true" "head" "<banking >"
```


OutputFilter�̐ݒ�ƁAReplaceContent�f�B���N�e�B�u�̐ݒ�����܂��B<br>
ReplaceContent�̏����͈ȉ��ł��B<br>
```Apache
	ReplaceContent partial_match|tag[:true|false] target replacement
```
|**����**|**����**|
|---|---|
|����1|partial_match ...������v�u�����w��B<br>tag...HTML�^�O�u�����w��B����true���w�肷��ƃ^�O���̂�u���Bfalse�̓^�O�̎��̈ʒu�ɑ}���B|
|����2|�u���Ώە�����|
|����3|�u����̕�����|


* example1<br>
ReplaceContent "partial_match" "a" "x"<br>
����HTTP�{�f�B�F"123a45"<br>
����HTTP�{�f�B�F"123x45"<br>

* example2<br>
ReplaceContent "tag:false" "head" "<meta >"<br>
����HTTP�{�f�B�F"\<head\>\</head\>"<br>
����HTTP�{�f�B�F"\<head\>\<meta \>\</head\>"<br>
<br>

## �N���X�̊ȈՂȐ���<br>
C����ł����A�t�@�C����struct�����܂��g���AC++��class�ɋ߂���������Ă��܂��B<br>
class���g�p���邽�߂̃��[��������������A�]���K�v������܂��B<br>

* �N���X��struct�ŕ\������B
* �N���Xstruct�̃����o�͏��Ԃ����i�B�擪�ɕK��AcCThisIsClass���`����B
* �N���X���\�b�h���̐擪�͕K���N���X���ɂ��A�A���_�[�o�[�ł�����Ƀ��\�b�h����A������B
* �N���X���\�b�h�̐擪�̈����͕K���ΏۃN���X�̃|�C���^�B
* private���\�b�h��.c�t�@�C������static�֐��ɂ��邱�ƂŎ�������B
* �擪��_�̃w�b�_�t�@�C����private�Ȃ̂ŗ��p�ґ���include���Ă͂Ȃ�Ȃ��B

**�L�q��**
```cpp
// �N���X�̌��J�����`�B�N���X�����̒�`��.c�t�@�C�����ȂǂŉB�����Ē�`����B
typedef struct CBucketController {
	AcCThisIsClass* thisIsClass;
} CBucketController;

// �N���X���\�b�h�錾
void CBucketController_exportModifiedBrigadeToBrigade(CBucketController* p_this, apr_bucket_brigade* outbb);
```


�Ƃ肠�����o���Ă����N���X�͈ȉ��̃N���X�����ŏ\���ł��B<br>

|**�N���X��**|**����**|
|---|---|
|***CAnalysisManager�N���X***| brigade�̊Ǘ����s���ACAnalysisExecutor�ւ̎w�����o���N���X�B|
|***CAnalysisExecutor�N���X***| HTTP�{�f�B�̕�����̕��͂�u�����s���N���X�B���삷��B<br>CAnalysisManager���K�؂ȃ��\�b�h���Ăяo���B|


<br>


**�y�V���v���Ȏg�p��z**<br>

```cpp
//��͎҂̐���(���̃N���X�����삷�邱�Ƃŗl�X�ȋ@�\�������ł���)
CAnalysisExecutor* executor = (CAnalysisExecutor*)CAnalysisExecutor_PartialMatch_new("a", "x");
//�}�l�[�W���[�̐���
CAnalysisManager* ana = CAnalysisManager_new(executor, pool, bucket_alloc, NULL, in_bucket_brigade);

//���brigade��j������
apr_brigade_destroy(in_bucket_brigade);

//1��ڂ�brigade���󂯎��
apr_bucket_brigade* brigade = CAnalysisManager_run(ana, NULL);
```
Apache Module�Ƃ��č쐬����Ƃ��ɂ͒��ӓ_�����邪�A�g�p��̍��g�݂Ƃ��Ă͏�L�B<br>
CAnalysisExecutor_PartialMatch�� CAnalysisExecutor �̔h���N���X�ŁA�h���N���X�̍����͌�قǋL�q�B<br>
CAnalysisExecutor_PartialMatch�͎w��̕�����"a"���A������v�����Ƃ��ɒu������"x"�Œu������B<br>
<br>

**�yCAnalysisExecutor�̔h���N���X�̍����z**<br>
CAnalysisExecutor�̔h���N���X�́A���\�b�h��static�ō쐬���A�֐��|�C���^��e�N���X�ɐݒ肵�č쐬����B<br>

**���J����N���X�錾struct�̍쐬�̗�: header.h**

```cpp
///public�̐錾
typedef struct CAnalysisExecutor_PartialMatch {
	CAnalysisExecutor parentMember;
} CAnalysisExecutor_PartialMatch;
```

**�B������N���X�錾struct�Ǝ����̍쐬�̗�: partial_match.c**

```cpp
///private�ȃN���X�\���̐錾
typedef struct CAnalysisExecutor_PartialMatch_Super {
	CAnalysisExecutor_Super parentMember;
	//
	char* targetStr;
	...
} CAnalysisExecutor_PartialMatch_Super;

///�f�o�b�O�p�̃N���X�����̏�Ԃ��o�͂���B
static void CAnalysisExecutor_PartialMatch__debugPrint(const CAnalysisExecutor* p_this) {
	...
}

///brigade�����̈�ԍŏ��ɌĂ΂��B�N���X�̏������Ȃǂ��s���B
static AcBool CAnalysisExecutor_PartialMatch_start(CAnalysisExecutor* p_this, const apr_table_t* table) {
	...
}

///�u���J�n�ʒu��1�����i�߂��ꍇ�ɌĂ΂��B�����ł͎�ɒu���Ώۂ̊J�n�ʒu��T���X�e�[�^�X�ɂȂ�B
static const AnalysisCommand CAnalysisExecutor_PartialMatch_forward(CAnalysisExecutor* p_this, 
	const AnalysisCommand cmd, const AcBool isRejectPreCmd, const char c) 
{
	...
}

///�u�������s���ꂽ��ɌĂ΂��B�u����̏���������B
static const AnalysisCommand CAnalysisExecutor_PartialMatch_replace(CAnalysisExecutor* p_this, 
	const AnalysisCommand cmd, AcBool result) 
{
	...
}

///�w��̈ʒu�̕������擾����ꍇ�ɌĂ΂��B
static const AnalysisCommand CAnalysisExecutor_PartialMatch_pos(CAnalysisExecutor* p_this, 
	const AnalysisCommand cmd, const char c) 
{
	...
}

///brigade�����̏I���iEOS�j�̂Ƃ��ɌĂ΂��B�Ō�ɕ������ǉ����邱�Ƃ��w���ł���B
static const AnalysisCommand CAnalysisExecutor_PartialMatch_end(CAnalysisExecutor* p_this, const AnalysisCommand cmd) {
	...
}

///�f�X�g���N�^
static void CAnalysisExecutor_PartialMatch_delete(void* p_this) {
	...
}

///�R���X�g���N�^
CAnalysisExecutor_PartialMatch* CAnalysisExecutor_PartialMatch_new(const char* targetStr, const char* replaceStr) {
	CAnalysisExecutor_PartialMatch_Super* self = (CAnalysisExecutor_PartialMatch_Super*)malloc(sizeof(CAnalysisExecutor_PartialMatch_Super));
	if (self == NULL) return NULL;
	//�e�����o�[�̏�����
	CAnalysisExecutor_init((CAnalysisExecutor*)self, gClassName_AnalysisExecutor_PartialMatch, 
		CAnalysisExecutor_PartialMatch_delete,
		CAnalysisExecutor_PartialMatch_start, 
		CAnalysisExecutor_PartialMatch_forward,
		CAnalysisExecutor_PartialMatch_replace,
		CAnalysisExecutor_PartialMatch_pos,
		CAnalysisExecutor_PartialMatch_end
		);
	...
	//�f�o�b�O�o�͗p�̊֐��̐ݒ�
	CAnalysisExecutor_setDebugPrintFunc((CAnalysisExecutor*)self, CAnalysisExecutor_PartialMatch__debugPrint);
	return (CAnalysisExecutor_PartialMatch*)self;
}

```
<br>

**�e���\�b�h�̐���**<br>
CAnalysisExecutor�͎󓮓I�ȃN���X�ŁA�C�x���g�h���u���ɋ߂����������B<br>
�e���\�b�h��CAnalysisManager��return�Ŏ��̎w�����o���ƁA�w���ɉ�����CAnalysisExecutor�̃��\�b�h���Ăяo���Ă����B<br>

|**���\�b�h��**|**����**|
|---|---|
|***CAnalysisExecutor_Xxxx_start()***| �����̈�ԍŏ��̊J�n���ɌĂ΂��B���g�̏�������������������B|
|***CAnalysisExecutor_Xxxx_forward()***| �u���J�n�ʒu��1�o�C�g�i�߂��Ƃ��ɁA�ǂݍ���1�o�C�g�������ɂ��ČĂ΂��B<br>�u���J�n�ʒu�ƂȂ肤�邩�𔻕ʂ��鏈������������B<br>�u���J�n�ƂȂ肤��ꍇ�A�ʏ��pos�Ŏ���1������ǂݍ��ގw�����o���B|
|***CAnalysisExecutor_Xxxx_pos()***| �u���J�n�ʒu���ړ��������ɁA�w��̈ʒu��1�o�C�g��ǂݍ��񂾂Ƃ��ɌĂ΂��B<br>�ǂݍ��񂾕������u���Ώۂƃ}�b�`���邩����������B�}�b�`����Βu���w�����o���B<br>�}�b�`��������ɓǂݍ��݂��K�v�ȏꍇ�͂����pos�Ŏ���1������ǂݍ��ގw�����o���B|
|***CAnalysisExecutor_Xxxx_replace()***| �u�����s��ɌĂ΂��B<br>�ʏ�͒u���J�n�ʒu��1�o�C�g�i�߂āA�u���J�n�ʒu�������������Ɉڍs������B|
|***CAnalysisExecutor_Xxxx_end()***| birgade�������I������Ƃ��ɌĂ΂��B<br>HTTP�{�f�B�̈�ԍŌ�̈ʒu�ɕ������ǉ����鏈���Ȃǂ��ł���B|

<br>
<br>



