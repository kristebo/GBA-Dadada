.SUFFIXES:

ifeq ($(strip $(DEVKITARM)),)
$(error DEVKITARM is not set; build through ./build.sh or the pinned devkitPro image)
endif

include $(DEVKITARM)/gba_rules

PROJECT_ROOT := $(abspath $(dir $(firstword $(MAKEFILE_LIST))))
TARGET   := tg2027
BUILD    := build
SOURCES  := source/demo source/effects source/engine source/scenes
INCLUDES := include
DATA     :=
GRAPHICS := assets/graphics
MUSIC    :=
LOGO_PNG := $(PROJECT_ROOT)/assets/graphics/tg_logo.png
LOGO_ASM := $(PROJECT_ROOT)/$(BUILD)/tg_logo.s
LOGO_HDR := $(PROJECT_ROOT)/$(BUILD)/tg_logo.h

ARCH     := -mthumb -mthumb-interwork
CFLAGS   := -g -Wall -Wextra -O2 -std=gnu11 -mcpu=arm7tdmi -mtune=arm7tdmi $(ARCH)
CFLAGS   += -iquote $(PROJECT_ROOT)/include -I$(LIBGBA)/include -I$(PROJECT_ROOT)/$(BUILD)
ASFLAGS  := -g $(ARCH)
LDFLAGS  := -g $(ARCH) -Wl,-Map,$(PROJECT_ROOT)/rom/$(TARGET).map
LIBS     := -lgba -lm
LIBDIRS  := $(LIBGBA)
SIZE     := arm-none-eabi-size

ifneq ($(BUILD),$(notdir $(CURDIR)))

export OUTPUT := $(PROJECT_ROOT)/rom/$(TARGET)
export VPATH  := $(foreach dir,$(SOURCES),$(PROJECT_ROOT)/$(dir)) \
                 $(foreach dir,$(DATA),$(PROJECT_ROOT)/$(dir)) \
                 $(foreach dir,$(GRAPHICS),$(PROJECT_ROOT)/$(dir)) \
                 $(PROJECT_ROOT)/$(BUILD)
export DEPSDIR := $(PROJECT_ROOT)/$(BUILD)

CFILES   := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
SFILES   := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s))) \
            tg_logo.s
BINFILES := $(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

export OFILES_BIN     := $(addsuffix .o,$(BINFILES))
export OFILES_SOURCES := $(CFILES:.c=.o) $(SFILES:.s=.o)
export OFILES         := $(OFILES_BIN) $(OFILES_SOURCES)
export LD             := $(CC)
export INCLUDE        := $(foreach dir,$(INCLUDES),-iquote $(CURDIR)/$(dir)) \
                         $(foreach dir,$(LIBDIRS),-I$(dir)/include) \
                         -I$(PROJECT_ROOT)/$(BUILD)
export LIBPATHS       := $(foreach dir,$(LIBDIRS),-L$(dir)/lib)

.PHONY: all clean rom-size $(BUILD)

all: $(BUILD)

rom:
	@mkdir -p $@

$(LOGO_ASM): $(LOGO_PNG) | rom
	@mkdir -p "$(PROJECT_ROOT)/$(BUILD)"
	@grit "$<" -gT000000 -gt -gB4 -p -pn16 -pT0 -fts \
	    -slogo_gfx -o"$(PROJECT_ROOT)/$(BUILD)/tg_logo" -W1

$(LOGO_HDR): $(LOGO_ASM)

$(BUILD): $(LOGO_ASM) | rom
	@mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

rom-size: all
	@$(SIZE) -A $(OUTPUT).elf
	@bytes=$$(wc -c < "$(OUTPUT).gba"); budget=33554432; \
	  printf "ROM image: %s bytes\nROM budget: %s bytes\nFree: %s bytes\n" \
	    "$$bytes" "$$budget" "$$((budget - bytes))"

clean:
	@rm -rf $(BUILD)
	@rm -f rom/$(TARGET).elf rom/$(TARGET).gba rom/$(TARGET).map

else

$(OUTPUT).gba: $(OUTPUT).elf
$(OUTPUT).elf: $(OFILES)
$(OFILES_SOURCES): $(HFILES)

%.bin.o %_bin.h: %.bin
	@$(bin2o)

-include $(DEPSDIR)/*.d

endif
