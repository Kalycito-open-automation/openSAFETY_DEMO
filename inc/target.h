/**
********************************************************************************
\file   target.h

\brief  Target specific header file of the application interface module

This file contains definitions for the platform of the application interface
user library.

*******************************************************************************/

/*------------------------------------------------------------------------------
Copyright (c) 2013, Bernecker+Rainer Industrie-Elektronik Ges.m.b.H. (B&R)
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the copyright holders nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
------------------------------------------------------------------------------*/

#ifndef _INC_TARGET_H_
#define _INC_TARGET_H_

//------------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// const defines
//------------------------------------------------------------------------------

#if (defined(__NIOS2__) || defined(__MICROBLAZE__)) && defined(__GNUC__)

#ifndef CHAR
    #define CHAR char
#endif

#ifndef INT8
    #define INT8 char
#endif

#ifndef UINT8
    #define UINT8 unsigned char
#endif

#ifndef BOOL
    #define BOOL UINT8
#endif

#ifndef INT16
    #define INT16 short int
#endif

#ifndef UINT16
    #define UINT16 unsigned short int
#endif

#ifndef INT
    #define INT int
#endif

#ifndef INT32
    #define INT32 int
#endif

#ifndef UINT
    #define UINT unsigned int
#endif

#ifndef UINT32
    #define UINT32 unsigned int
#endif

#ifndef INT64
    #define INT64 long long int
#endif

#ifndef UINT64
    #define UINT64 unsigned long long int
#endif

#ifndef NULL
    #define NULL ((void *) 0)
#endif

#ifndef _TIME_OF_DAY_DEFINED_

    typedef struct
    {
        unsigned long  int  m_dwMs;
        unsigned short int  m_wDays;

    } tTimeOfDay;

#define _TIME_OF_DAY_DEFINED_

#endif  //_TIME_OF_DAY_DEFINED_

#elif defined(__arm__) && ( defined(__CC_ARM) || defined( __GNUC__ ) )

#ifndef CHAR
    #define CHAR char
#endif

#ifndef INT8
    #define INT8 char
#endif

#ifndef UINT8
    #define UINT8 unsigned char
#endif

#ifndef BOOL
    #define BOOL UINT8
#endif

#ifndef INT16
    #define INT16 short int
#endif

#ifndef UINT16
    #define UINT16 unsigned short int
#endif

#ifndef INT
    #define INT int
#endif

#ifndef INT32
    #define INT32 int
#endif

#ifndef UINT
    #define UINT unsigned int
#endif

#ifndef UINT32
    #define UINT32 unsigned int
#endif

#ifndef INT64
    #define INT64 long int
#endif

#ifndef UINT64
    #define UINT64 unsigned long int
#endif

#ifndef NULL
    #define NULL ((void *) 0)
#endif

#ifndef _TIME_OF_DAY_DEFINED_

    typedef struct
    {
        unsigned long  int  m_dwMs;
        unsigned short int  m_wDays;

    } tTimeOfDay;

#define _TIME_OF_DAY_DEFINED_

#endif  //_TIME_OF_DAY_DEFINED_

#elif defined(_WIN32_) || defined( _MSC_VER )
  // types defined in WINDEF.H, included by <windows.h>
  #include <windows.h>
  
  
#ifndef _TIME_OF_DAY_DEFINED_

    typedef struct
    {
        unsigned long  int  m_dwMs;
        unsigned short int  m_wDays;

    } tTimeOfDay;

#define _TIME_OF_DAY_DEFINED_

#endif  //_TIME_OF_DAY_DEFINED_

#else
    #error "Please define the datatypes for your target and compiler here!"
#endif

//------------------------------------------------------------------------------
// typedef
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------


#endif /* _INC_TARGET_H_ */


