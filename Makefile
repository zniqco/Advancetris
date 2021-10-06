.SUFFIXES:

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/gba_rules

TARGET := $(notdir $(CURDIR))
BUILD := build
SOURCES := source
INCLUDES := include
DATA := data

# options for code generation
ARCH := -mthumb -mthumb-interwork

ifeq ($(DEBUG),1)
	CFLAGS := -gdwarf-2 -Wall -O1 -mcpu=arm7tdmi -mtune=arm7tdmi $(ARCH)
else
	CFLAGS := -g -Wall -O3 -mcpu=arm7tdmi -mtune=arm7tdmi -fomit-frame-pointer -ffast-math $(ARCH)
endif

CFLAGS	+= $(INCLUDE)

CXXFLAGS := $(CFLAGS) -fno-rtti -fno-exceptions

ASFLAGS := -g $(ARCH)
LDFLAGS	= -g $(ARCH) -Wl,-Map,$(notdir $*.map)
LIBS := -lgba
LIBDIRS := $(LIBGBA)

ifneq ($(BUILDDIR), $(CURDIR))

export OUTPUT := $(CURDIR)/$(TARGET)

export VPATH := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
                $(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR := $(CURDIR)/$(BUILD)

CFILES := $(foreach dir,$(SOURCES),$(patsubst $(dir)/%,%,$(wildcard $(dir)/*.c) $(wildcard $(dir)/**/*.c)))
CPPFILES := $(foreach dir,$(SOURCES),$(patsubst $(dir)/%,%,$(wildcard $(dir)/*.cpp) $(wildcard $(dir)/**/*.cpp)))
PNGFILES := $(foreach dir,$(SOURCES),$(patsubst $(dir)/%,%,$(wildcard $(dir)/*.png) $(wildcard $(dir)/**/*.png)))
BINFILES := $(foreach dir,$(DATA),$(patsubst $(dir)/%,%,$(wildcard $(dir)/*.*) $(wildcard $(dir)/**/*.*)))

# use CXX for linking C++ projects, CC for standard C
ifeq ($(strip $(CPPFILES)),)
	export LD := $(CC)
else
	export LD := $(CXX)
endif

export OFILES_BIN := $(addsuffix .o,$(BINFILES))

export OFILES_SOURCES := $(PNGFILES:.png=.o) $(CPPFILES:.cpp=.o) $(CFILES:.c=.o)
 
export OFILES := $(OFILES_BIN) $(OFILES_SOURCES)

export HFILES := $(addsuffix .h,$(subst .,_,$(BINFILES)))

export INCLUDE := $(foreach dir,$(INCLUDES),-iquote $(CURDIR)/$(dir)) \
                  $(foreach dir,$(LIBDIRS),-I$(dir)/include) \
                  -I$(CURDIR)/$(BUILD)
 
export LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib)

.PHONY: $(BUILD) clean

$(BUILD):
	@for i in $(foreach files,$(CFILES) $(CPPFILES) $(PNGFILES) $(BINFILES),$(dir $(BUILD)/$(files))); do \
		[ -d $$i ] || mkdir -p $$i; \
	done

	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) BUILDDIR=`cd $(BUILD) && pwd` --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

clean:
	@echo clean ...
	@rm -fr $(BUILD) $(TARGET).elf $(TARGET).gba 

else

$(OUTPUT).gba: $(OUTPUT).elf

$(OUTPUT).elf: $(OFILES)

$(OFILES_SOURCES): $(HFILES)

# custom grit output
%.s %.h: %.png %.grit
	@grit $< -fts -o$@
	@for i in $*.s $*.h; do \
		sed -E -i \
		-e 's/($(notdir $*))TilesLen/\U\1_TILES_LENGTH/g' \
		-e 's/($(notdir $*))Tiles/\U\1_TILES/g' \
		-e 's/($(notdir $*))MapLen/\U\1_MAP_LENGTH/g' \
		-e 's/($(notdir $*))Map/\U\1_MAP/g' \
		-e 's/($(notdir $*))PalLen/\U\1_PALETTE_LENGTH/g' \
		-e 's/($(notdir $*))Pal/\U\1_PALETTE/g' \
		$$i; \
	done

-include $(DEPSDIR)/*.d

endif
