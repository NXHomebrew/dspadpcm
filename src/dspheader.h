/*--------------------------------------------------------------------------*
  Project: Revolution DSP ADPCM encoder
  File:    dspheader.h

  Copyright (C)2001-2006 Nintendo  All Rights Reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
  in whole or in part, without the prior written consent of Nintendo.

  $Log: dspheader.h,v $
  Revision 1.1.1.1  2006/07/24 07:39:09  tfukuda
  initial check in. base0701スタートです。

  Revision 1.2  2006/02/09 07:07:35  aka
  Changed copyright.

  
 *--------------------------------------------------------------------------*/

#ifndef __DSPHEADER_H__
#define __DSPHEADER_H__

#define VOICE_TYPE_NOTLOOPED    0x0000     // sample is not looped        
#define VOICE_TYPE_LOOPED       0x0001     // sample is indeed looped

#define DEC_MODE_ADPCM          0x0000     // ADPCM mode
#define DEC_MODE_PCM16          0x000A     // 16-bit PCM mode
#define DEC_MODE_PCM8           0x0009     // 8-bit PCM mode (UNSIGNED)


typedef struct
{
    u32 num_samples;
    u32 num_adpcm_nibbles;
    u32 sample_rate;

    u16 loop_flag;
    u16 format;
    u32 sa;     // loop start address
    u32 ea;     // loop end address
    u32 ca;     // current address

    u16 coef[16];

    // start context
    u16 gain;   
    u16 ps;
    u16 yn1;
    u16 yn2;

    // loop context
    u16 lps;    
    u16 lyn1;
    u16 lyn2;

    u16 pad[11];

} DSPADPCMHEADER;

#endif // __DSPHEADER_H__
