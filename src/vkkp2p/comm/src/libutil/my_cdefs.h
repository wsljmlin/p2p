

//ECOS_8203 �ı�׼��ֻ��ʹ��C��׼
//������make file ���ô��棺 -D__MY_BEGIN_DECLS__='extern "C" {' -D__MY_END_DECLS__='}' ʵ�� ����-D__MY_BEGIN_DECLS__='' -D__MY_END_DECLS__=''
//Ҳ������makefile �� ��-include my_cdefs.h
//VC �� �������м����´��� /D "__MY_BEGIN_DECLS__=" /D "__MY_END_DECLS__=" ����Ԥ�����������м�__MY_BEGIN_DECLS__=;__MY_END_DECLS__=

#if defined(_ECOS_8203) && defined(__cplusplus)
# define __MY_BEGIN_DECLS__	extern "C" {
# define __MY_END_DECLS__	}
#else
# define __MY_BEGIN_DECLS__
# define __MY_END_DECLS__
#endif
