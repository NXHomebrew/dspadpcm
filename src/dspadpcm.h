/*---------------------------------------------------------------------------*
  Project: Revolution DSP ADPCM encoder
  File:    dspadpcm.h

  Copyright (C)1998-2006 Nintendo  All Rights Reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: dspadpcm.h,v $
  Revision 1.1.1.1  2006/07/24 07:39:09  tfukuda
  initial check in. base0701スタートです。

  Revision 1.2  2006/02/09 07:07:35  aka
  Changed copyright.

  
 *---------------------------------------------------------------------------*/

#ifndef __DSPADPCM_H__
#define __DSPADPCM_H__


typedef struct
{

    u32 channels;
    u32 bitsPerSample;
    u32 sampleRate;
    u32 samples;
    u32 loopStart;
    u32 loopEnd;
    u32 bufferLength;

} SOUNDINFO;

#define SOUND_FILE_SUCCESS      0
#define SOUND_FILE_FORMAT_ERROR 1
#define SOUND_FILE_FOPEN_ERROR  2

typedef struct
{
    // start context
    s16 coef[16];
    u16 gain;
    u16 pred_scale;
    s16 yn1;
    s16 yn2;

    // loop context
    u16 loop_pred_scale;
    s16 loop_yn1;
    s16 loop_yn2;

} ADPCMINFO;


#endif // __DSPADPCM_H__
