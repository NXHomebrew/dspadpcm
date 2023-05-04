/*--------------------------------------------------------------------------*
  Project: Revolution DSP ADPCM encoder
  File:    endian.h

  Copyright (C)2001-2006 Nintendo  All Rights Reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: endian.h,v $
  Revision 1.1.1.1  2006/07/24 07:39:09  tfukuda
  initial check in. base0701スタートです。

  Revision 1.2  2006/02/09 07:07:35  aka
  Changed copyright.

  
 *--------------------------------------------------------------------------*/

#ifndef __ENDIAN_H__
#define __ENDIAN_H__

#include "Types.h"

u32  reverse_endian_32(u32 i);
u16  reverse_endian_16(u16 i);
void reverse_buffer_16(u16 *p, int samples);

#endif // __ENDIAN_H__
