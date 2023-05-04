/*---------------------------------------------------------------------------*
  Project: Revolution DSP ADPCM encoder
  File:    Types.h

  Copyright (C)2001-2006 Nintendo  All Rights Reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: Types.h,v $
  Revision 1.1.1.1  2006/07/24 07:39:09  tfukuda
  initial check in. base0701スタートです。

  Revision 1.2  2006/02/09 07:07:35  aka
  Changed copyright.

  Revision 1.1  2005/11/04 06:11:37  aka
  Imported from Dolphin tree.
    
    1     5/20/01 11:51p Eugene
    DSPADPCM converter tool, command-line. Converts WAV or AIFF files into
    DSP-ADPCM-formatted files, with a 96-byte DSP paramblock header (see
    DSPADPCM document). 
    
    Illustrates use of DSPTOOL DLL, which provides the DSP-ADPCM codec
    functions.
    
    Source code is provided to developers (eventually). This is an MSVC++
    project (bleah)
    
    2     3/04/99 2:18p Tianli01
    testing
    
    1     3/04/99 2:18p Tianli01
    initial checkin for testing
    
    1     12/15/98 10:05p Shiki

  Change History:
    12/10/1998  Shiki Okasaka   Revised to reflect the coding guidelines
    12/04/1998  Shiki Okasaka   Created

  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#ifndef __TYPES_H__
#define __TYPES_H__

#ifdef  __MWERKS__
// for metrowerks compiler
typedef char                s8;
typedef short               s16;
typedef long                s32;
typedef long long           s64;
typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned long       u32;
typedef unsigned long long  u64;
typedef float               f32;
#else   // __MWERKS__
#ifndef WIN32
// for GNU compiler, etc.
typedef char                s8;
typedef short               s16;
typedef long                s32;
typedef long long           s64;
typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned long       u32;
typedef unsigned long long  u64;
#else   // WIN32
// for Microsoft compiler
typedef __int8              s8;
typedef __int16             s16;
typedef __int32             s32;
typedef __int64             s64;
typedef unsigned __int8     u8;
typedef unsigned __int16    u16;
typedef unsigned __int32    u32;
typedef unsigned __int64    u64;
#endif  // WIN32
#endif  // __MWERKS__

typedef void*               Ptr;
//typedef u32                 BOOL;
//typedef u8                  BYTE;

#define TRUE                1
#define FALSE               0

#endif  // __TYPES_H__
