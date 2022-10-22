# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

IRXS = ps2dev9 netman smap
IRX_SRCS = $(shell for i in $(IRXS); do echo irx_$${i}.c; done)
IRX_OBJS = $(shell for i in $(IRXS); do echo irx_$${i}.o; done)
EE_BIN_PKD = JOYSRV.ELF
EE_BIN_STR = joysrv-str.elf
EE_BIN = joysrv-unc.elf
EE_OBJS = joysrv.o pad.o moduleloader.o myoplnetparser.o ps2ipcfg.o ps2ipcfglib.o $(IRX_OBJS)
EE_LIBS = -lmc -lpad -lpadx -lnetman -ldebug -lps2ip -ldebug -lpatches
BIN2C = $(PS2SDK)/bin/bin2c
PS2IP ?= 192.168.83.74

ifndef PS2SDK
ps2sdk-not-setup:
	@echo "PS2SDK is not setup. Please setup PS2SDK before building this project"
endif

all: $(EE_BIN_PKD)

$(EE_BIN_PKD): $(EE_BIN_STR)
	ps2-packer $< $@

$(EE_BIN_STR): $(EE_BIN)
	$(EE_STRIP) --strip-all $< -o $@

clean:
	rm -f $(EE_BIN_PKD) $(EE_BIN_STR) $(EE_BIN) $(EE_OBJS) $(IRX_SRCS)

irx_%.c: $(PS2SDK)/iop/irx/%.irx
	$(BIN2C) $< irx_$*.c $*_irx

DEV9_irx.c: $(PS2SDK)/iop/irx/ps2dev9.irx
	$(BIN2C) $< DEV9_irx.c DEV9_irx

NETMAN_irx.c: $(PS2SDK)/iop/irx/netman.irx
	$(BIN2C) $< NETMAN_irx.c NETMAN_irx

SMAP_irx.c: $(PS2SDK)/iop/irx/smap.irx
	$(BIN2C) $< SMAP_irx.c SMAP_irx

run: $(EE_BIN_PKD)
	ps2client -h $(PS2IP) execee host:$(EE_BIN_PKD)

reset:
	ps2client -h $(PS2IP) reset

ifdef PS2SDK
include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
endif
