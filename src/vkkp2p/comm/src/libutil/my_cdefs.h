

//ECOS_8203 的标准库只能使用C标准
//可以在make file 中用代替： -D__MY_BEGIN_DECLS__='extern "C" {' -D__MY_END_DECLS__='}' 实现 或者-D__MY_BEGIN_DECLS__='' -D__MY_END_DECLS__=''
//也可以在makefile 中 用-include my_cdefs.h
//VC 在 命令行中加以下代替 /D "__MY_BEGIN_DECLS__=" /D "__MY_END_DECLS__=" 或者预处理器定义中加__MY_BEGIN_DECLS__=;__MY_END_DECLS__=

#if defined(_ECOS_8203) && defined(__cplusplus)
# define __MY_BEGIN_DECLS__	extern "C" {
# define __MY_END_DECLS__	}
#else
# define __MY_BEGIN_DECLS__
# define __MY_END_DECLS__
#endif
