#pragma once

//#ifdef __cplusplus
//extern "C" {
//#endif

//#define _PKCS_
//应用于加密少量字符串等

int imp_dc_des(const char* userkey,const char*des,int des_len,char* buf,int len);
int imp_en_des(const char* userkey,const char*buf,int buf_len,char* des,int des_len);

int dc_des(const char*des,int des_len,char* buf,int len);
int en_des(const char*buf,int buf_len,char* des,int des_len);

//int str2hex(const char str[],unsigned char hex[]);

//add hcl
int des_buf2str(const char* inbuf,int inlen, char* outstr,int outlen);
int des_str2buf(const char* instr,int inlen, char* outbuf,int outlen);
int des_imp_dc_des_str(const char* userkey,const char*des,int des_len,char* buf,int len);
int des_imp_en_des_str(const char* userkey,const char*buf,int buf_len,char* des,int des_len);

//#ifdef __cplusplus
//};
//#endif

