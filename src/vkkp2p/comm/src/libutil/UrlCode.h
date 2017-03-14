#pragma once

#include "basetypes.h"

inline char Char2Int(char ch){
        if(ch>='0' && ch<='9')return (char)(ch-'0');
        if(ch>='a' && ch<='f')return (char)(ch-'a'+10);
        if(ch>='A' && ch<='F')return (char)(ch-'A'+10);
        return -1;
}

inline char Str2Bin(char *str){
        char tempWord[2];
        char chn;

        tempWord[0] = Char2Int(str[0]);                                //make the B to 11 -- 00001011
        tempWord[1] = Char2Int(str[1]);                                //make the 0 to 0  -- 00000000

        chn = (tempWord[0] << 4) | tempWord[1];                //to change the BO to 10110000

        return chn;
}
inline string UrlDecode(const string& str){
        string output="";
        char tmp[2];
        int i=0,len=(int)str.length();
		char *sOut = new char[len+2];
		int j=0;
        while(i<len){
                if(str[i]=='%'){
                        tmp[0]=str[i+1];
                        tmp[1]=str[i+2];
						sOut[j++] = Str2Bin(tmp);
                        i=i+3;
                }
                else if(str[i]=='+'){
						sOut[j++] = ' ';
                        i++;
                }
                else{
                        sOut[j++] = str[i];
                        i++;
                }
        }
		sOut[j] = '\0';
		output = sOut;
        delete[] sOut;
        return output;
}


 
    inline unsigned char toHex(const unsigned char &x)
    {
        return x > 9 ? x + 55: x + 48; 
    }
 
    inline string UrlEncode(const string &sIn)
    {
    // cout << "size: " << sIn.size() << endl;
        string sOut;
		size_t slen = sIn.length();
        unsigned char buf[4]; 
		for( size_t ix = 0; ix < slen; ix++ )
        {       
            memset( buf, 0, 4 ); 
            if( isalnum( (unsigned char)sIn[ix] ) )
            {       
                buf[0] = sIn[ix];
            }
            else if ( isspace( (unsigned char)sIn[ix] ) )
            {
                buf[0] = '+';
            }
            else
            {
                buf[0] = '%';
                buf[1] = toHex( (unsigned char)sIn[ix] >> 4 );
                buf[2] = toHex( (unsigned char)sIn[ix] % 16);
            }
            sOut += (char *)buf;
        }
        return sOut;
    };

//
//// 代码转换操作类 用于将utf-8 格式转成 gb2312
//class CodeConverter {
//        private:
//                        iconv_t cd;
//        public:
//                        CodeConverter(const char *from_charset,const char *to_charset) {// 构造
//                                cd = iconv_open(to_charset,from_charset);
//                        }
//                
//                        ~CodeConverter() {// 析构
//                                iconv_close(cd);
//                        }
//                
//                        int convert(char *inbuf,int inlen,char *outbuf,int outlen) {// 转换输出
//                                char **pin = &inbuf;
//                                char **pout = &outbuf;
//
//                                memset(outbuf,0,outlen);
//                                return iconv(cd,pin,(size_t *)&inlen,pout,(size_t *)&outlen);
//                        }
//};
//
////输入url_Utf-8 ,输出 gb2312
//string Url2Str_Utf8(string instr){
//        string input;
//        input=UrlDecode(instr);
//
//        const int        outlen=instr.length();
//        char output[outlen];
//
//        CodeConverter cc = CodeConverter("utf-8","gb2312");
//        cc.convert((char *)input.c_str(),strlen(input.c_str()),output,outlen);
//
//        return output;
//}

//输入url_gb2312 ,输出 gb2312 实际上是直接调用 UrlDecode()
inline string Url2Str_gb2312(string str){
        return UrlDecode(str);
}
