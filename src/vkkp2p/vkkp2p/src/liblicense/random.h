#ifndef __MINIUTIL_UTIL_RANDOM_H__
#define __MINIUTIL_UTIL_RANDOM_H__


namespace miniutil
{
	class Random
	{
		public:
            static void rand_seed(unsigned int);
            static unsigned int rand_value(unsigned int minvalue, unsigned int maxvalue);
			static unsigned int rand_value(unsigned int maxvalue);
			static int rand_stringbuf16(int seed_in, char *buf_out);
			static int rand_stringbuf(int len, char *buf_out, char* excludechars,int seed_in = 0);
	};
}

#endif
