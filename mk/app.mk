# mk/app.mk -- build rules for a senew app (.sen).  Include it from your app's Makefile:
#
#     APP       = myapp                      # name of the output: build/myapp.sen   (default: the folder name)
#     SENEW_SDK ?= ../senew-sdk              # where the SDK lives (see below)
#     include $(SENEW_SDK)/mk/app.mk
#
# Targets:   make          -> build/$(APP).sen (the file you upload with SENUS: Apps -> "Subir .sen")
#            make size     -> size of the .sen and of its pieces
#            make clean
#
# Variables you may set BEFORE the include (all optional):
#   SRCS          the C files            (default: src/*.c)
#   EXTRA_SRCS    more C files, e.g. a decoder from another folder
#   EXTRA_CFLAGS  more compiler flags, e.g. -O2 -Iother/include -DSOMETHING=1
#   ICON_ART      12-line text icon      (default: icon/arte.txt if it exists; '#' = ink)
#   ICON_MESH     3D menu icon (.bin)    (default: icon/malla.bin if it exists)
#   RES           a resources file       (default: resources.bin if it exists; read with ap_rec_leer)
#   ARMCC         the compiler           (default: arm-none-eabi-gcc)
#   PYTHON        python 3.8+            (default: python3)
#   BUILD         output folder          (default: build)
#
# Compiler: any recent "Arm GNU Toolchain" for bare metal (arm-none-eabi-gcc).  Tested: 15.2 (Arm GNU Toolchain 15.2.Rel1, macOS)
# and 15.2.0 (Fedora gcc-arm-none-eabi).  The target is an ARM926EJ-S (ARMv5TE), soft-float, no libc, no OS: the app talks to the
# system only through the table `w5` (include/cargable.h).

SDK_ROOT := $(abspath $(dir $(lastword $(MAKEFILE_LIST)))..)
APP      ?= $(notdir $(abspath .))
ARMCC    ?= arm-none-eabi-gcc
PYTHON   ?= python3
BUILD    ?= build
SRCS     ?= $(sort $(wildcard src/*.c))
ICON_ART ?= $(wildcard icon/arte.txt)
ICON_MESH?= $(wildcard icon/malla.bin)
RES      ?= $(wildcard resources.bin)

ARMFLAGS = -mcpu=arm926ej-s -mfloat-abi=soft -marm -mthumb-interwork -ffreestanding -fno-builtin -fno-pic -Os \
           -ffunction-sections -fdata-sections -std=gnu11 -Wall -Wextra -Wno-override-init -I$(SDK_ROOT)/include -Isrc
LDFLAGS  = -nostdlib -nostartfiles -static -T $(SDK_ROOT)/lib/w5_app.ld -Wl,--gc-sections -Wl,--emit-relocs \
           -Wl,--build-id=none -Wl,-z,max-page-size=16
SDK_DEPS = $(wildcard $(SDK_ROOT)/include/*.h) $(SDK_ROOT)/lib/w5_app.ld $(SDK_ROOT)/lib/w5_crt0.c $(SDK_ROOT)/tools/sen_empaquetar.py

all: $(BUILD)/$(APP).sen

$(BUILD)/$(APP).elf: $(SRCS) $(EXTRA_SRCS) $(wildcard src/*.h) $(SDK_DEPS)
	@mkdir -p $(BUILD)
	$(ARMCC) $(ARMFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) -Wl,-Map=$(BUILD)/$(APP).map -o $@ $(SRCS) $(EXTRA_SRCS) $(SDK_ROOT)/lib/w5_crt0.c -lgcc

$(BUILD)/$(APP).sen: $(BUILD)/$(APP).elf $(ICON_ART) $(ICON_MESH) $(RES)
	$(PYTHON) $(SDK_ROOT)/tools/sen_empaquetar.py $< $@ $(if $(RES),--rec $(RES)) \
	    $(if $(ICON_ART),--arte $(ICON_ART)) $(if $(ICON_MESH),--malla $(ICON_MESH))

size: $(BUILD)/$(APP).sen
	@ls -l $(BUILD)/$(APP).sen | awk '{print $$5 " bytes  " $$9}'
	@arm-none-eabi-size $(BUILD)/$(APP).elf 2>/dev/null || true

clean:
	rm -rf $(BUILD)

.PRECIOUS: $(BUILD)/$(APP).elf
.PHONY: all size clean
