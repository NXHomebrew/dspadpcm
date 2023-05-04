/*--------------------------------------------------------------------------*
  Project: Revolution DSP ADPCM encoder
  File:    main.c

  Copyright (C)2001-2006 Nintendo  All Rights Reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: main.c,v $
  Revision 1.1.1.1  2006/07/24 07:39:09  tfukuda
  initial check in. base0701スタートです。

  Revision 1.2  2006/02/09 07:07:35  aka
  Changed copyright.

  
 *--------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include "endian.h"
#include "dspadpcm.h"
#include "dspheader.h"

#define MAX_PATH_SIZE   256

u8 input_path  [MAX_PATH_SIZE]; // path to input file
u8 output_path [MAX_PATH_SIZE]; // path to output file
u8 coef_path   [MAX_PATH_SIZE]; // path to DSPADPCMHEADER text dump file

FILE *input_file;
FILE *output_file;
FILE *coef_file;

BOOL  loop_flag;                // TRUE if loop points were specified on commandline    
BOOL  encode_flag;              // TRUE for encoding
BOOL  decode_flag;              // TRUE for decoding
BOOL  coef_flag;                // TRUE for text dump of DSPADPCMHEADER to file 
BOOL  verbose_flag;             // TRUE is user specified verbose mode
BOOL  ea_flag;                  // TRUE if user specified end address

u32   loopStart;                // user specified loop start sample
u32   loopEnd;                  // user specified loop end sample
u32   sampleEndAddr;    // user specified end address

#define DECODE_WAV  0
#define DECODE_AIFF 1

u32 decodeFormat;               // user specified decode fromat


/*--------------------------------------------------------------------------*
        dsptool DLL interface
 *--------------------------------------------------------------------------*/
HINSTANCE hDllDsptool;

typedef u32 (*lpFunc1)(u32);
typedef u32 (*lpFunc2)(void);
typedef void (*lpFunc3)(s16*, u8*, ADPCMINFO*, u32);
typedef void (*lpFunc4)(u8*, s16*, ADPCMINFO*, u32);
typedef void (*lpFunc5)(u8*, ADPCMINFO*, u32);

lpFunc1 getBytesForAdpcmBuffer;
lpFunc1 getBytesForAdpcmSamples;
lpFunc1 getBytesForPcmBuffer;
lpFunc1 getBytesForPcmSamples;
lpFunc1 getNibbleAddress;
lpFunc1 getNibblesForNSamples;
lpFunc1 getSampleForAdpcmNibble;
lpFunc2 getBytesForAdpcmInfo;
lpFunc3 encode;
lpFunc4 decode;
lpFunc5 getLoopContext;


/*--------------------------------------------------------------------------*
        soundfile DLL interface
 *--------------------------------------------------------------------------*/
HINSTANCE hDllSoundfile;

typedef int (*lpFunc6)(u8 *path, SOUNDINFO *soundinfo);
typedef int (*lpFunc7)(u8 *path, SOUNDINFO *soundinfo, void *dest);
typedef int (*lpFunc8)(char *path, SOUNDINFO *info, void *samples);
lpFunc6 getSoundInfo;
lpFunc7 getSoundSamples;
lpFunc8 writeWaveFile;
lpFunc8 writeAiffFile;


/*--------------------------------------------------------------------------*/
void print_banner(void)
{

    printf("\n");
    printf("DSPADPCM v2.3 - DSP-ADPCM encoder\n");
    printf("Copyright 2001 Nintendo. All rights reserved.\n\n");

} // end print_banner


/*--------------------------------------------------------------------------*/
void print_usage(void)
{

    printf("\n");
    printf("Usage:\n\n");
    printf("DSPADPCM -<mode> <inputfile> [<outputfile>] [-opt<argument>]\n\n"); 
    printf("Where:\n");
    printf("   <mode>.............E for Encode, D for Decode (required)\n");
    printf("   <inputfile>........Input file (required)\n");
    printf("   [<outputfile>].....Output file (optional)\n\n");

    printf("Options are:\n");
    printf("   -c<coeffile>.......Text dump of coefficients and other data\n");
    printf("   -l<start-end>......Sound effect is looped; 'start' is first sample\n");
    printf("                      in loop, counting from zero. 'end' is the last\n");
    printf("                      sample in the loop, also counting from zero.\n");
    printf("   -a<end addr>.......For non-looped sound effects; last sample in\n");
    printf("                      the sound effect, counting from zero.\n");
    printf("   -h.................This help text.\n");
    printf("   -v.................Verbose mode.\n");
    printf("   -f.................Decode to AIFF.\n");
    printf("   -w.................Decode to WAV (default).\n");
    printf("\n\n");
    printf("This tool generates DSPADPCM data from MONO, 16-bit PCM WAV or AIFF\n");
    printf("files. The DSPADPCM output file (*.dsp) has a %d byte data header \n", sizeof(DSPADPCMHEADER));
    printf("which contains the ADPCM coefficients, loop context (if any), \n");
    printf("and other sample info. The format of this header is described \n");
    printf("in the DSPADPCM documentation.\n\n");
        
} // end print_usage()


/*--------------------------------------------------------------------------*/

void init(void)
{

    input_file          = NULL;
    output_file         = NULL;
    coef_file           = NULL;
        decodeFormat    = DECODE_WAV;

    hDllDsptool     = NULL;
    hDllSoundfile   = NULL;

} // end init()


/*--------------------------------------------------------------------------*/
void clean_up(void)
{

    if (input_file)             fclose(input_file);
    if (output_file)    fclose(output_file);
    if (coef_file)              fclose(coef_file);
    if (hDllDsptool)    FreeLibrary(hDllDsptool);    
    if (hDllSoundfile)  FreeLibrary(hDllSoundfile);    

} // end clean_up()


/*--------------------------------------------------------------------------*/

void coefficients(u16 *p)
{
    int i;

    for (i = 0; i < 16; i++)
        printf("[%d]: 0x%04X\n", i, *p++ & 0xffff);
}


/*--------------------------------------------------------------------------
 * findbase() - returns pointer to first character of file basename
 *--------------------------------------------------------------------------*/
BYTE *findbase(BYTE *string)
{

    BYTE *ptr = string; 

        while (*ptr++); // find end of string

    while ((--ptr >= string) && (*ptr != '\\') && (*ptr != '/') && (*ptr != ':'));

    return(++ptr);

} // end findbase()


/*--------------------------------------------------------------------------
 * findext() - returns pointer to start of file extension (including '.')
 *--------------------------------------------------------------------------*/
BYTE *findext(BYTE *string)
{

    BYTE *ptr;
    BYTE *end;

    ptr = string;

    while (*ptr++); // find end of string

    end = ptr;

        while ((--ptr >= string) && (*ptr != '.'));

    if (ptr <= string)
                return(end);
    else
        return(ptr);
        
} // end findext()


/*--------------------------------------------------------------------------
 * parse_args() - parse the command line arguments
 *--------------------------------------------------------------------------*/
BOOL parse_args(int argc, char *argv[])
{
    BOOL rval;
    u16  i;

    if (argc < 2)
    {
        // must specify input file at least
        printf("\nERROR: Missing parameter\n");

        print_usage();

        return(FALSE);
    }

    rval         = TRUE;
    loop_flag    = FALSE;
    encode_flag  = FALSE;
    decode_flag  = FALSE;
    coef_flag    = FALSE;
    verbose_flag = FALSE;
    ea_flag      = FALSE;

    memset(input_path,   0x00, MAX_PATH_SIZE);
    memset(output_path,  0x00, MAX_PATH_SIZE);
    memset(coef_path,    0x00, MAX_PATH_SIZE);

    loopStart     = 0xFFFFFFFF;
    loopEnd       = 0xFFFFFFFF;
    sampleEndAddr = 0x00000000;

    for (i = 1; (i < argc) && (rval == TRUE); i++)
    {
        switch (argv[i][0])
        {
            case '?':
                // user is confused
                return(FALSE);
                
                break;

            case '-':
            case '/':
//            case '\\':

                switch(argv[i][1])
                {
                case 'f':
                case 'F':

                    decodeFormat = DECODE_AIFF;

                    break;
                
                case 'w':
                case 'W':

                    decodeFormat = DECODE_WAV;

                    break;

                    case 'e':
                    case 'E':

                        if (TRUE == decode_flag)
                        {
                            // user already asserted decode mode
                            printf("\nERROR: Decode flag already asserted!\n");
                            rval = FALSE;
                        }
                        else
                        {
                            encode_flag = TRUE;
                        }
                        
                        break;

                    case 'D':
                    case 'd':

                        if (TRUE == encode_flag)
                        {
                            // user already asserted encode mode
                            printf("\nERROR: Encode flag already asserted!\n");
                            rval = FALSE;
                        }
                        else
                        {
                            decode_flag = TRUE;
                        }
                        
                        break;

                    case 'l':
                    case 'L':

                        loop_flag = TRUE;
                        if (EOF == sscanf(&argv[i][2], "%d-%d", &loopStart, &loopEnd))
                        {
                            printf("\nERROR: Unable to parse loop points '%s'\n", &argv[i][2]);
                            rval = FALSE;
                        }

                        break;

                    case '?':
                    case 'H':
                    case 'h':

                        return(FALSE);
                        
                        break;

                    case 'a':
                    case 'A':

                        if (EOF == sscanf(&argv[i][2], "%d", &sampleEndAddr))
                        {
                            printf("\nERROR: Invalid sample end address.\n");
                            rval = FALSE;
                        }
                        ea_flag = TRUE;

                        break;

                    case 'c':
                    case 'C':

                        // specify coefficient output file
                        if (argv[i][2])
                        {
                            strcpy(coef_path, &argv[i][2]);
                        }
                        coef_flag = TRUE;

                        break;
        
                    case 'v':
                    case 'V':

                        verbose_flag = TRUE;
                        
                        break;
                        
                    default:

                        // unknown switch
                        printf("\nERROR: Unknown switch '%c'\n", argv[i][1]);
                        rval = FALSE;
                        
                        break;

                } // end switch

                
                break;

            default:

                if (0 == input_path[0])
                {
                    // input path not specified yet, so argument must be the input path
                    strcpy(input_path, &argv[i][0]);
                }
                else if (0 == output_path[0])
                {
                    // an input_path has been specified, so this must be the output
                    strcpy(output_path, &argv[i][0]);
                }
                else
                {
                    // unknown unswitched argument
                    printf("\nERROR: Extraneous argument '%s'\n", argv[i]);
                    rval = FALSE;
                }                            

                break;

        } // end switch()
    } // for...i

    // now perform sanity check
    if (0 == input_path[0])
    {
        // no input file specified!
        printf("\nERROR: No input file specified!\n");
        rval = FALSE;
    }

    if (0 == output_path[0])
    {
        // output path not specified yet, use default
        strcpy(output_path, findbase(input_path));

        if (TRUE == decode_flag)
        {
                        if (decodeFormat != DECODE_AIFF)
                        {
                    // add default extension of '.wav'    
                        strcpy(findext(output_path), ".wav");
                        }
                        else
                        {
                    // add default extension of '.aif'    
                        strcpy(findext(output_path), ".aif");
                        }
        }
        else if (TRUE == encode_flag)
        {
            // add default extension of '.dsp'    
            strcpy(findext(output_path), ".dsp");
        }
        else
        {
            printf("\nERROR: MODE directive not specified!\n");
            rval = FALSE;
        }
    }

    if ((0 == coef_path[0]) && (coef_flag = TRUE))
    {
        // coefficient output path not specified, use default
        strcpy(coef_path, findbase(input_path));

        // add default extension of '.txt'
        strcpy(findext(coef_path), ".txt");
    }

#define ARAM_MAX_RANGE  0x4000000   // 64MB of ARAM - in my dreams

    if (TRUE == loop_flag)
    {
        if (loopStart > loopEnd)
        {
            printf("\nWARNING: loopStart is greater than loopEnd\n");
        }

        if (loopStart > ARAM_MAX_RANGE)
        {
            printf("\nWARNING: loop-start is beyond valid ARAM range! (0x%08X)\n", loopStart);
        }

        if (loopEnd > ARAM_MAX_RANGE)
        {
            printf("\nWARNING: loop=end is beyond valid ARAM range! (0x%08X)\n", loopEnd);
        }

        if (TRUE == ea_flag)
        {
            printf("\nWARNING: '-a' argument ignored for looped sample.\n");
        }
    }

    if (sampleEndAddr > ARAM_MAX_RANGE)
    {
        printf("\nWARNING: sample-end address is beyond valid ARAM range! (0x%08X)\n", sampleEndAddr);
    }

        if (verbose_flag)
        {
                printf("\tinput file : '%s'\n", input_path);
                printf("\toutput file: '%s'\n", output_path);
                printf("\tcoef file  : '%s'\n", coef_path);
                printf("\n");
        }

    // debug
/*
        printf("\n****************************\n");

        printf("input_path : '%s'\n", input_path);
        printf("output_path: '%s'\n", output_path);
        printf("coef_path  : '%s'\n", coef_path);
        printf("decode_path: '%s'\n", decode_path);

        printf("\n");

        printf("loopStart: %d\n", loopStart);
        printf("loopEnd  : %d\n", loopEnd);

        printf("\n");

        printf("rval: %d\n", rval);

        //exit(0);

  */

    return(rval);

} // end parse args


/*--------------------------------------------------------------------------
 *      dump DSPHEADER to specified file  
 *--------------------------------------------------------------------------*/
void dump_header(DSPADPCMHEADER *h, FILE *handle)
{

    u16 i;
    u16 j;

    fprintf(handle, "\n"); 

    fprintf(handle, "Header size: %d bytes\n\n", sizeof(DSPADPCMHEADER));

    fprintf(handle, "Sample     : '%s'\n", input_path);
    fprintf(handle, "Length     : %d samples\n", reverse_endian_32(h->num_samples));
    fprintf(handle, "Num nibbles: %d ADPCM nibbles\n", reverse_endian_32(h->num_adpcm_nibbles));

    fprintf(handle, "Sample Rate: %d Hz\n", reverse_endian_32(h->sample_rate));
    fprintf(handle, "Loop Flag  : %s\n", VOICE_TYPE_LOOPED == reverse_endian_16(h->loop_flag) ? "LOOPED" : "NOT LOOPED");

    fprintf(handle, "\n"); 

    fprintf(handle, "Start Addr : 0x%08X + ARAM_offset (ADPCM nibble mode)\n", reverse_endian_32(h->sa));
    fprintf(handle, "End Addr   : 0x%08X + ARAM_offset (ADPCM nibble mode)\n", reverse_endian_32(h->ea));
    fprintf(handle, "Curr Addr  : 0x%08X + ARAM_offset (ADPCM nibble mode)\n", reverse_endian_32(h->ca));

    fprintf(handle, "\n"); 

    j = 0;
    for (i=0; i<16; i+=2)
    {
        fprintf(handle, "a1[%d]: 0x%04X a2[%d]: 0x%04X \n", j, reverse_endian_16(h->coef[i]), j, reverse_endian_16(h->coef[i+1]));
        j++;
    }

    fprintf(handle, "\n"); 

    fprintf(handle, "Gain      : 0x%04X\n", reverse_endian_16(h->gain));
    fprintf(handle, "Pred/Scale: 0x%04X\n", reverse_endian_16(h->ps));
    fprintf(handle, "y[n-1]    : 0x%04X\n", reverse_endian_16(h->yn1));
    fprintf(handle, "y[n-2]    : 0x%04X\n", reverse_endian_16(h->yn2));

    fprintf(handle, "\n"); 

    fprintf(handle, "Loop Pred/Scale: 0x%04X\n", reverse_endian_16(h->lps));
    fprintf(handle, "Loop y[n-1]    : 0x%04X\n", reverse_endian_16(h->lyn1));
    fprintf(handle, "Loop y[n-2]    : 0x%04X\n", reverse_endian_16(h->lyn2));

} // end dump_header()


/*--------------------------------------------------------------------------*
 *  
 *--------------------------------------------------------------------------*/
void encode_soundfile(char *path, SOUNDINFO *soundinfo)
{
    DSPADPCMHEADER dspadpcmheader;
    ADPCMINFO adpcminfo;
    void *soundbuffer, *outputbuffer;

    if (verbose_flag) printf(" Done.\nGetting sound samples...");

    soundbuffer     = malloc(soundinfo->bufferLength);
    outputbuffer    = malloc(getBytesForAdpcmBuffer(soundinfo->samples));

    if (!soundbuffer || !outputbuffer)
    {
        if (soundbuffer)    free(soundbuffer);
        if (outputbuffer)   free(outputbuffer);

        printf("Cannot allocate buffers for encode!\n");
        return;
    }

    memset(&dspadpcmheader, 0, sizeof(DSPADPCMHEADER));

    dspadpcmheader.num_samples          = reverse_endian_32(soundinfo->samples);
    dspadpcmheader.num_adpcm_nibbles    = reverse_endian_32(getNibblesForNSamples(soundinfo->samples));
    dspadpcmheader.sample_rate          = reverse_endian_32(soundinfo->sampleRate);

    // if the user specified loop points on the commandline use them
        // or else look for loop points in the adpcminfo
    if (loop_flag)
    {
            u32 nibbleLoopStart, nibbleLoopEnd, nibbleCurrent;

                nibbleLoopStart = getNibbleAddress(loopStart);
                nibbleLoopEnd   = getNibbleAddress(loopEnd);
                nibbleCurrent   = getNibbleAddress(0);

                dspadpcmheader.loop_flag                = reverse_endian_16(VOICE_TYPE_LOOPED);
                dspadpcmheader.format                   = reverse_endian_16(DEC_MODE_ADPCM);
        dspadpcmheader.sa                               = reverse_endian_32(nibbleLoopStart);
        dspadpcmheader.ea                               = reverse_endian_32(nibbleLoopEnd);
        dspadpcmheader.ca                               = reverse_endian_32(nibbleCurrent);
    }
        else if (soundinfo->loopEnd) // the sound info has loops form AIFF
        {
            u32 nibbleLoopStart, nibbleLoopEnd, nibbleCurrent;

                nibbleLoopStart = getNibbleAddress(soundinfo->loopStart);
        nibbleLoopEnd   = getNibbleAddress(soundinfo->loopEnd - 1); // AIFF loop end is 1 based
                nibbleCurrent   = getNibbleAddress(0);

                dspadpcmheader.loop_flag                = reverse_endian_16(VOICE_TYPE_LOOPED);
                dspadpcmheader.format                   = reverse_endian_16(DEC_MODE_ADPCM);
        dspadpcmheader.sa                               = reverse_endian_32(nibbleLoopStart);
        dspadpcmheader.ea                               = reverse_endian_32(nibbleLoopEnd);
        dspadpcmheader.ca                               = reverse_endian_32(nibbleCurrent);
        }
    else // not looped
    {
            u32 nibbleLoopStart, nibbleLoopEnd, nibbleCurrent;

                nibbleLoopStart = getNibbleAddress(0);

                // if the user specified end address use it
                if (ea_flag)
                        nibbleLoopEnd   = getNibbleAddress(sampleEndAddr);
                else
                        nibbleLoopEnd   = getNibbleAddress(soundinfo->samples - 1);

                nibbleCurrent   = getNibbleAddress(0);

                dspadpcmheader.loop_flag                = reverse_endian_16(VOICE_TYPE_NOTLOOPED);
                dspadpcmheader.format                   = reverse_endian_16(DEC_MODE_ADPCM);
        dspadpcmheader.sa                               = reverse_endian_32(nibbleLoopStart);
        dspadpcmheader.ea                               = reverse_endian_32(nibbleLoopEnd);
        dspadpcmheader.ca                               = reverse_endian_32(nibbleCurrent);
    }

    getSoundSamples(path, soundinfo, soundbuffer);    

        if (verbose_flag) printf(" Done.\nEncoding samples...");
    
        encode(soundbuffer, outputbuffer, &adpcminfo, soundinfo->samples);

    // if the user specified loops get loop context
        if (loop_flag)
                getLoopContext(outputbuffer, &adpcminfo, loopStart);
        // see if the aiff file has loop points
        else if (soundinfo->loopEnd)
                getLoopContext(outputbuffer, &adpcminfo, soundinfo->loopStart);
        // no loop make sure loop context is 0
        else
                adpcminfo.loop_pred_scale = adpcminfo.loop_yn1 = adpcminfo.loop_yn2 = 0;

    // put the adpcm info on the dsp_header
    dspadpcmheader.coef[0]  = reverse_endian_16(adpcminfo.coef[0]);
    dspadpcmheader.coef[1]  = reverse_endian_16(adpcminfo.coef[1]);
    dspadpcmheader.coef[2]  = reverse_endian_16(adpcminfo.coef[2]);
    dspadpcmheader.coef[3]  = reverse_endian_16(adpcminfo.coef[3]);
    dspadpcmheader.coef[4]  = reverse_endian_16(adpcminfo.coef[4]);
    dspadpcmheader.coef[5]  = reverse_endian_16(adpcminfo.coef[5]);
    dspadpcmheader.coef[6]  = reverse_endian_16(adpcminfo.coef[6]);
    dspadpcmheader.coef[7]  = reverse_endian_16(adpcminfo.coef[7]);
    dspadpcmheader.coef[8]  = reverse_endian_16(adpcminfo.coef[8]);
    dspadpcmheader.coef[9]  = reverse_endian_16(adpcminfo.coef[9]);
    dspadpcmheader.coef[10] = reverse_endian_16(adpcminfo.coef[10]);
    dspadpcmheader.coef[11] = reverse_endian_16(adpcminfo.coef[11]);
    dspadpcmheader.coef[12] = reverse_endian_16(adpcminfo.coef[12]);
    dspadpcmheader.coef[13] = reverse_endian_16(adpcminfo.coef[13]);
    dspadpcmheader.coef[14] = reverse_endian_16(adpcminfo.coef[14]);
    dspadpcmheader.coef[15] = reverse_endian_16(adpcminfo.coef[15]);
    dspadpcmheader.gain     = reverse_endian_16(adpcminfo.gain);
    dspadpcmheader.ps       = reverse_endian_16(adpcminfo.pred_scale);
    dspadpcmheader.yn1      = reverse_endian_16(adpcminfo.yn1);
    dspadpcmheader.yn2      = reverse_endian_16(adpcminfo.yn2);
    dspadpcmheader.lps      = reverse_endian_16(adpcminfo.loop_pred_scale);
    dspadpcmheader.lyn1     = reverse_endian_16(adpcminfo.loop_yn1);
    dspadpcmheader.lyn2     = reverse_endian_16(adpcminfo.loop_yn2);

        if (verbose_flag) printf(" Done.\nWriting DSPADPCM file...");

    // write the output file
    fwrite(&dspadpcmheader, 1, sizeof(DSPADPCMHEADER), output_file);
    fwrite(outputbuffer, getBytesForAdpcmSamples(soundinfo->samples), sizeof(u8), output_file);

        if (coef_flag)
        {
                if (verbose_flag) printf(" Done.\nWriting text dump file ...");
                dump_header(&dspadpcmheader, coef_file);
        }

        if (verbose_flag) printf(" Done.\n");
}


/*--------------------------------------------------------------------------*
 *  
 *--------------------------------------------------------------------------*/
void encode_input_file(void)
{
    SOUNDINFO       soundinfo;
    
        if (verbose_flag) printf("Getting sound header...");

    switch (getSoundInfo(input_path, &soundinfo))
    {
    case SOUND_FILE_SUCCESS:

        if (soundinfo.bitsPerSample != 16)
        {
            printf("Sound file buffer not 16bit samples!\n");
            return;
        }

        if (soundinfo.channels != 1)
        {
            printf("Soundfile buffer not mono!\n");
            return;
        }

        encode_soundfile(input_path, &soundinfo);

        break;

    case SOUND_FILE_FORMAT_ERROR:

        printf("Sound file format not supported!\n");

        return;

    case SOUND_FILE_FOPEN_ERROR:

        printf("fopen error for sound file!\n");

        return;
    }
}


/*--------------------------------------------------------------------------*
 *  
 *--------------------------------------------------------------------------*/
void decode_input_file(void)
{
        DSPADPCMHEADER dspadpcmheader;
        u32     samples, sampleRate;
        u8  *inputBuffer;
    u16 *outputBuffer;

        if (verbose_flag) printf("Getting DSPADPCM header...");

        // get DSPADPCMHEADER
        fread(&dspadpcmheader, 1, sizeof(DSPADPCMHEADER), input_file);
        
        samples         = reverse_endian_32(dspadpcmheader.num_samples);
        sampleRate      = reverse_endian_32(dspadpcmheader.sample_rate);

        // allocate buffers
        if (inputBuffer = malloc(getBytesForAdpcmSamples(samples)))
        {
                if (verbose_flag) printf(" Done.\nGetting ADPCM samples...");

                if (outputBuffer = malloc(samples * 2))
                {
            SOUNDINFO soundinfo;
            ADPCMINFO adpcminfo;

                        // prepare adpcminfo for decoding
                        adpcminfo.coef[0]       = (s16)reverse_endian_16(dspadpcmheader.coef[0]);
                        adpcminfo.coef[1]       = (s16)reverse_endian_16(dspadpcmheader.coef[1]);
                        adpcminfo.coef[2]       = (s16)reverse_endian_16(dspadpcmheader.coef[2]);
                        adpcminfo.coef[3]       = (s16)reverse_endian_16(dspadpcmheader.coef[3]);
                        adpcminfo.coef[4]       = (s16)reverse_endian_16(dspadpcmheader.coef[4]);
                        adpcminfo.coef[5]       = (s16)reverse_endian_16(dspadpcmheader.coef[5]);
                        adpcminfo.coef[6]       = (s16)reverse_endian_16(dspadpcmheader.coef[6]);
                        adpcminfo.coef[7]       = (s16)reverse_endian_16(dspadpcmheader.coef[7]);
                        adpcminfo.coef[8]       = (s16)reverse_endian_16(dspadpcmheader.coef[8]);
                        adpcminfo.coef[9]       = (s16)reverse_endian_16(dspadpcmheader.coef[9]);
                        adpcminfo.coef[10]      = (s16)reverse_endian_16(dspadpcmheader.coef[10]);
                        adpcminfo.coef[11]      = (s16)reverse_endian_16(dspadpcmheader.coef[11]);
                        adpcminfo.coef[12]      = (s16)reverse_endian_16(dspadpcmheader.coef[12]);
                        adpcminfo.coef[13]      = (s16)reverse_endian_16(dspadpcmheader.coef[13]);
                        adpcminfo.coef[14]      = (s16)reverse_endian_16(dspadpcmheader.coef[14]);
                        adpcminfo.coef[15]      = (s16)reverse_endian_16(dspadpcmheader.coef[15]);
                        adpcminfo.gain          = reverse_endian_16(dspadpcmheader.gain);
                        adpcminfo.pred_scale= 0;
                        adpcminfo.yn1           = 0;
                        adpcminfo.yn2           = 0;

                        // read adpcm samples into input buffer
                        fread(inputBuffer, getBytesForAdpcmSamples(samples), sizeof(u8), input_file);

                        if (verbose_flag) printf(" Done.\nDecoding samples...");

                        // decode samples to buffer
                        decode(
                                inputBuffer,
                                outputBuffer,
                                &adpcminfo,
                                samples
                                );

            soundinfo.bitsPerSample = 16;
            soundinfo.bufferLength  = samples * 2;
            soundinfo.channels      = 1;
            soundinfo.sampleRate    = sampleRate;
            soundinfo.samples       = samples;

            if (dspadpcmheader.loop_flag)
            {
                soundinfo.loopStart = getSampleForAdpcmNibble(reverse_endian_32(dspadpcmheader.sa));
                soundinfo.loopEnd   = getSampleForAdpcmNibble(reverse_endian_32(dspadpcmheader.ea)) + 1;
            }
            else
            {
                soundinfo.loopStart = 0; 
                soundinfo.loopEnd   = 0;
            }

                        if (verbose_flag) printf(" Done.\nWriting sound file...");

                switch (decodeFormat)
                        {
                        case DECODE_WAV:

                writeWaveFile(output_path, &soundinfo, outputBuffer); 

                                break;

                        case DECODE_AIFF:
                                
                writeAiffFile(output_path, &soundinfo, outputBuffer); 

                break;
                        }
                }
                else
                {
                        printf("\nERROR: Cannot allocate output buffer!\n");
                        clean_up();
                        exit(1);
                }
        }
        else
        {
                printf("\nERROR: Cannot allocate input buffer!\n");
                clean_up();
                exit(1);
        }

        // free buffers
        if (inputBuffer)        free(inputBuffer);
        if (outputBuffer)       free(outputBuffer);

        // see if we should write a coefficient file
        if (coef_flag)
        {
                if (verbose_flag) printf(" Done.\nWriting text dump file...");
        dump_header(&dspadpcmheader, coef_file);
        }
        if (verbose_flag) printf(" Done.\n");
}


/*--------------------------------------------------------------------------*/
int get_dll(void)
{
    hDllDsptool = LoadLibrary("dsptool.dll");

    if (hDllDsptool)
    {
        getBytesForAdpcmBuffer  = (lpFunc1)GetProcAddress(hDllDsptool, "getBytesForAdpcmBuffer");
        getBytesForAdpcmSamples = (lpFunc1)GetProcAddress(hDllDsptool, "getBytesForAdpcmSamples");
        getBytesForPcmBuffer    = (lpFunc1)GetProcAddress(hDllDsptool, "getBytesForPcmBuffer");
        getBytesForPcmSamples   = (lpFunc1)GetProcAddress(hDllDsptool, "getBytesForPcmSamples");
        getNibbleAddress        = (lpFunc1)GetProcAddress(hDllDsptool, "getNibbleAddress");
        getNibblesForNSamples   = (lpFunc1)GetProcAddress(hDllDsptool, "getNibblesForNSamples");
        getSampleForAdpcmNibble = (lpFunc1)GetProcAddress(hDllDsptool, "getSampleForAdpcmNibble");
        getBytesForAdpcmInfo    = (lpFunc2)GetProcAddress(hDllDsptool, "getBytesForAdpcmInfo");
        encode                  = (lpFunc3)GetProcAddress(hDllDsptool, "encode");
        decode                  = (lpFunc4)GetProcAddress(hDllDsptool, "decode");
        getLoopContext          = (lpFunc5)GetProcAddress(hDllDsptool, "getLoopContext");

        if (
            !getBytesForAdpcmBuffer     ||
            !getBytesForAdpcmSamples    ||
            !getBytesForPcmBuffer       ||
            !getBytesForPcmSamples      ||
            !getNibbleAddress           ||
            !getNibblesForNSamples      ||
            !getSampleForAdpcmNibble    ||
            !getBytesForAdpcmInfo       ||
            !encode                     ||
            !decode                     ||
            !getLoopContext          
            ) return FALSE;
    }

    hDllSoundfile = LoadLibrary("soundfile.dll");

    if (hDllSoundfile)
    {
        getSoundInfo            = (lpFunc6)GetProcAddress(hDllSoundfile, "getSoundInfo");
        getSoundSamples         = (lpFunc7)GetProcAddress(hDllSoundfile, "getSoundSamples");
        writeWaveFile           = (lpFunc8)GetProcAddress(hDllSoundfile, "writeWaveFile");
        writeAiffFile           = (lpFunc8)GetProcAddress(hDllSoundfile, "writeAiffFile");

        if (
            !getSoundInfo       ||
            !getSoundSamples    ||
            !writeWaveFile      ||
            !writeAiffFile
            ) return FALSE;
    }

    if (hDllDsptool && hDllSoundfile)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


/*--------------------------------------------------------------------------*
 * main() 
 *--------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    init();
    print_banner();

    if (FALSE == parse_args(argc, argv))
    {
        // don't print usage help, it just adds more junk to 
        // log files. print usage only for missing parameters or explicit 
        // requests for help.
        exit(1);
    }

    // load the dspadpcm.dll
    if (FALSE == get_dll())
    {
        printf("\nERROR: Failed to load dll\n");
        exit(1);
    }

        // open files
        if ((input_file = fopen(input_path, "rb")) == NULL)
        {
                printf("\nERROR: Cannot open %s for reading!\n", input_path);
                clean_up();
                exit(1);
        }

        if ((output_file = fopen(output_path, "wb")) == NULL)
        {
                printf("\nERROR: Cannot open %s for writing!\n", output_path);
                clean_up();
                exit(1);
        }

        if (coef_path)
        {
                if ((coef_file = fopen(coef_path, "w")) == NULL)
                {
                        printf("\nERROR: Cannot open %s for writing!\n", coef_path);
                        clean_up();
                        exit(1);
                }
        }

    // encode or decode
    if (encode_flag)
        encode_input_file();

    if (decode_flag)
        decode_input_file();

    // clean up
    clean_up();

    // exit with a clean bill of health
    exit(0);

} // end main()
