###############################################################################
# Makefile for DSPADPCM
#
# Copyright (C)2005-2006 Nintendo  All Rights Reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
#  $Log: makefile,v $
#  Revision 1.1.1.1  2006/07/24 07:39:09  tfukuda
#  initial check in. base0701スタートです。
#
#  Revision 1.6  2006/02/10 02:34:19  aka
#  Modified build target.
#
#  Revision 1.5  2006/02/09 07:52:41  yasuh-to
#  Added permission setting.
#
#  Revision 1.4  2006/02/09 06:59:06  aka
#  Changed copyright.
#
#  Revision 1.3  2006/02/07 12:23:26  mitu
#  changed for VC.NET 2003
#
#  Revision 1.2  2005/11/04 06:41:34  aka
#  Modified for Revolution.
#
#  Revision 1.1  2005/11/04 06:11:28  aka
#  Created.
#
###############################################################################

all:	builddspadpcm

builddspadpcm:
		@chmod u+x buildscript
		@echo ===========Building DSPADPCM===============
		@./buildscript
		@echo ===========================================

clean:
		@chmod u+x buildscript
		@echo ===========Cleaning DSPADPCM===============
		@./buildscript /CLEAN
		@echo ===========================================


MODULENAME	= dspadpcm
X86		= TRUE

include $(REVOLUTION_SDK_ROOT)/build/buildtools/commondefs

include $(REVOLUTION_SDK_ROOT)/build/buildtools/modulerules
