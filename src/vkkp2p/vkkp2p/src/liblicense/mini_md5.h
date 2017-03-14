
/****************************************************************************
**
** Copyright (C) 2004-2008 VATATA.com . All rights reserved.
**
** This file is the part of the Miniutil Poject of VATATA.com.
** Vatata's miniutil library is lightweight and portable (for linux
** and Windows). The C++ Class Library included Threadpool, Dll danymic loading, 
** General socket application model and tcp server, httpserver/servlet, etc. 
** It is a pratical utility disigned for who don't want learn the Big C++ 
** Developing Platform, such as  Boost or ACE.
**
** This file may be used under the terms of the GNU General Public
** License versions 2.0 or 3.0 as published by the Free Software
** Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file.  Alternatively you may (at
** your option) use any later version of the GNU General Public
** License if such license has been publicly approved by VATATA.com.
** (or its successors, if any) and the Miniutil Project Aministrators. 
**
** Please review the following information to ensure GNU General
** Public Licensing requirements will be met:
** http://www.gnu.org/licenses/lgpl.html. If you are unsure which 
** license is appropriate for your use, please review the following 
** information: http://code.google.com/p/miniutil/ or contact 
** http://www.vatata.com.
**
** In addition, as a special exception, VATATA.com, as the sole
** copyright holder for Miniutil Project, grants users of VATATA
** P2P Platform the right to release and package with the related 
** libraries without the source code.
**
** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE. vatata reserves all rights not expressly
** granted herein.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/


//
//  md5.h change namespace to miniutil.
//                           wang bo
//

// MD5.CC - source code for the C++/object oriented translation and 
//          modification of MD5.

// Translation and modification (c) 1995 by Mordechai T. Abzug 

// This translation/ modification is provided "as is," without express or 
// implied warranty of any kind.

// The translator/ modifier does not claim (1) that MD5 will do what you think 
// it does; (2) that this translation/ modification is accurate; or (3) that 
// this software is "merchantible."  (Language for this disclaimer partially 
// copied from the disclaimer below).

/* based on:

   MD5.H - header file for MD5C.C
   MDDRIVER.C - test driver for MD2, MD4 and MD5

   Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.

License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD5 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.

License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD5 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.

*/

#ifndef ___MINIUTIL__MD5____H__
#define ___MINIUTIL__MD5____H__


#include <stdio.h>
//#include <fstream>
//#include <iostream>
//
//using namespace std;

namespace miniutil
{

    namespace util
    {
	    class MD5 {

	    public:
	    // methods for controlled operation:
	      MD5              ();  // simple initializer
	      void  update     (unsigned char *input, unsigned int input_length);
//	      void  update     (istream& stream);
	      void  update     (FILE *file);
//	      void  update     (ifstream& stream);
	      void  finalize   ();

	    // constructors for special circumstances.  All these constructors finalize
	    // the MD5 context.
	      MD5              (unsigned char *string ,int len = -1 ); // digest string, finalize
//	      MD5              (istream& stream);       // digest stream, finalize
	      MD5              (FILE *file);            // digest file, close, finalize
//	      MD5              (ifstream& stream);      // digest stream, close, finalize

	    // methods to acquire finalized result
	      unsigned char    *raw_digest ();  // digest as a 16-byte binary array
	      char *            hex_digest ();  // digest as a 33-byte ascii-hex string
//	      friend ostream&   operator<< (ostream&, MD5 context);

            //static inline string raw_digest(const char * data, int len = -1 )
            //{
            //    MD5 md5( (unsigned char *) data, len );
            //    return string((char *)md5.raw_digest() , 16);
            //}
            //static inline string raw_digest( const string & data )
            //{
            //    return raw_digest( data.data(),(int) data.length() );
            //}

            //static inline string hex_digest(const char * data, int len = -1 )
            //{
            //    MD5 md5( (unsigned char *) data, len );
            //    return string( md5.hex_digest() );
            //}
            //static inline string hex_digest( const string & data )
            //{
            //    return hex_digest( data.data(),(int) data.length() );
            //}

	    private:

	    // first, some types:
	      typedef unsigned       int uint4; // assumes integer is 4 words long
	      typedef unsigned short int uint2; // assumes short integer is 2 words long
	      typedef unsigned      char uint1; // assumes char is 1 word long

	    // next, the private data:
	      uint4 state[4];
	      uint4 count[2];     // number of *bits*, mod 2^64
	      uint1 buffer[64];   // input buffer
	      uint1 digest[16];
	      uint1 finalized;

	    // last, the private methods, mostly static:
	      void init             ();               // called by all constructors
	      void transform        (uint1 *buffer);  // does the real update work.  Note 
											      // that length is implied to be 64.

	      static void encode    (uint1 *dest, uint4 *src, uint4 length);
	      static void decode    (uint4 *dest, uint1 *src, uint4 length);
	      static void memcpy    (uint1 *dest, uint1 *src, uint4 length);
	      static void memset    (uint1 *start, uint1 val, uint4 length);

	      static inline uint4  rotate_left (uint4 x, uint4 n);
	      static inline uint4  F           (uint4 x, uint4 y, uint4 z);
	      static inline uint4  G           (uint4 x, uint4 y, uint4 z);
	      static inline uint4  H           (uint4 x, uint4 y, uint4 z);
	      static inline uint4  I           (uint4 x, uint4 y, uint4 z);
	      static inline void   FF  (uint4& a, uint4 b, uint4 c, uint4 d, uint4 x, 
					    uint4 s, uint4 ac);
	      static inline void   GG  (uint4& a, uint4 b, uint4 c, uint4 d, uint4 x, 
					    uint4 s, uint4 ac);
	      static inline void   HH  (uint4& a, uint4 b, uint4 c, uint4 d, uint4 x, 
					    uint4 s, uint4 ac);
	      static inline void   II  (uint4& a, uint4 b, uint4 c, uint4 d, uint4 x, 
					    uint4 s, uint4 ac);

	    };
    }

    typedef miniutil::util::MD5 MD5; // for compatible
}

#endif
