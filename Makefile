#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

TOPDIR ?= $(CURDIR)
include $(DEVKITARM)/3ds_rules

# Your values.
APP_TITLE           :=  MultiUpdater
APP_DESCRIPTION     :=  Updater for FIRM payloads, CIAs, and other files
APP_AUTHOR          :=  LiquidFenrir


TARGET              :=  $(subst $e ,_,$(notdir $(APP_TITLE)))
OUTDIR              :=  out
BUILD               :=  build
SOURCES             :=  source
INCLUDES            :=  include
ROMFS               :=  romfs
# DATA                :=  data


# Path to the files
# If left blank, will try to use "icon.png", "$(TARGET).png", or the default ctrulib icon, in that order
ICON                :=  resources/icon.png

BANNER_AUDIO        :=  resources/audio.wav
BANNER_IMAGE        :=  resources/banner.png

RSF_PATH            :=  resources/rominfo.rsf

# If left blank, makerom will use the default Homebrew logo
LOGO                :=  resources/logo.bcma.lz


# If left blank, makerom will use default values (0xff3ff and CTR-P-CTAP, respectively)
# Be careful if UNIQUE_ID is the same as other apps: it will overwrite the previously installed one
UNIQUE_ID           :=  0xd5c49
PRODUCT_CODE        :=  CTR-P-ULTI

# VERSION             :=  $(shell git describe --tags)
VERSION             :=  v0.0r

# Don't really need to change this
ICON_FLAGS          :=	nosavebackups,visible

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
ARCH	:=	-march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft

CFLAGS	:=	-g -Wall -Wextra -O2 -mword-relocations \
      	  	-ffunction-sections \
      	  	$(ARCH)

CFLAGS	+=	$(INCLUDE) -DARM11 -D_3DS -D_GNU_SOURCE \
      	  	-DAPP_TITLE="\"$(APP_TITLE)\"" \
      	  	-DAPP_AUTHOR="\"$(APP_AUTHOR)\"" \
      	  	-DVERSION_STRING="\"$(VERSION)\""

CXXFLAGS	:= $(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++11

ASFLAGS	:=	-g $(ARCH)
LDFLAGS	=	-specs=3dsx.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map)

LIBS	:= -larchive -lbz2 -llzma -lz -lctru -lm

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:= $(CTRULIB) $(PORTLIBS)


#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export OUTPUT	:=	$(CURDIR)/$(OUTDIR)/$(TARGET)
export TOPDIR	:=	$(CURDIR)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
			$(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
PICAFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.v.pica)))
SHLISTFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.shlist)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
#---------------------------------------------------------------------------------
	export LD	:=	$(CC)
#---------------------------------------------------------------------------------
else
#---------------------------------------------------------------------------------
	export LD	:=	$(CXX)
#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------

export OFILES	:=	$(addsuffix .o,$(BINFILES)) \
			$(PICAFILES:.v.pica=.shbin.o) $(SHLISTFILES:.shlist=.shbin.o) \
			$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
			-I$(CURDIR)/$(BUILD)

export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)

ifeq ($(strip $(ICON)),)
	icons := $(wildcard *.png)
	ifneq (,$(findstring $(TARGET).png,$(icons)))
		export APP_ICON := $(TOPDIR)/$(TARGET).png
	else
		ifneq (,$(findstring icon.png,$(icons)))
			export APP_ICON := $(TOPDIR)/icon.png
		endif
	endif
else
	export APP_ICON := $(TOPDIR)/$(ICON)
endif

ifeq ($(strip $(NO_SMDH)),)
	export _3DSXFLAGS += --smdh=$(OUTPUT).smdh
endif

ifneq ($(ROMFS),)
	export _3DSXFLAGS += --romfs=$(CURDIR)/$(ROMFS)
endif

.PHONY: $(BUILD) clean all release

#---------------------------------------------------------------------------------
3dsx: $(BUILD) $(OUTPUT).3dsx

cia  :$(BUILD) $(OUTPUT).cia

all: 3dsx cia

release: clean all $(OUTPUT)-$(VERSION).zip

$(BUILD):
	@mkdir -p $(OUTDIR)
	@[ -d "$@" ] || mkdir -p "$@"
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr "$(BUILD)" "$(OUTDIR)"

#---------------------------------------------------------------------------------
%.zip:
	@7z a -bso0 "$@" "$(OUTPUT).cia"  "$(OUTPUT).3dsx"
	@7z rn -bso0 "$@" "$(TARGET).3dsx"  "3ds/$(TARGET)/$(TARGET).3dsx"

#---------------------------------------------------------------------------------
ifeq ($(strip $(NO_SMDH)),)
$(OUTPUT).3dsx	:	$(OUTPUT).elf $(OUTPUT).smdh
else
$(OUTPUT).3dsx	:	$(OUTPUT).elf
endif

#---------------------------------------------------------------------------------
MAKEROM     ?=  makerom

MAKEROM_ARGS	  :=  -elf "$(OUTPUT).elf" \
	                  -rsf "$(RSF_PATH)" \
	                  -banner "$(BUILD)/banner.bnr" \
	                  -icon "$(BUILD)/icon.icn" \
	                  -DAPP_TITLE="$(APP_TITLE)" \
	                  -DAPP_PRODUCT_CODE="$(PRODUCT_CODE)" \
	                  -DAPP_UNIQUE_ID="$(UNIQUE_ID)"

ifneq ($(strip $(ROMFS)),)
	MAKEROM_ARGS  +=  -DAPP_ROMFS="$(ROMFS)"
endif
ifneq ($(strip $(LOGO)),)
	MAKEROM_ARGS  +=  -logo "$(LOGO)"
endif

$(OUTPUT).cia:  $(OUTPUT).elf $(BUILD)/banner.bnr $(BUILD)/icon.icn
	$(MAKEROM) -f cia -o "$@" -target t -exefslogo $(MAKEROM_ARGS)


BANNERTOOL  ?=  bannertool

ifeq ($(suffix $(BANNER_IMAGE)),.cgfx)
	BANNER_IMAGE_ARG := -ci
else
	BANNER_IMAGE_ARG := -i
endif

ifeq ($(suffix $(BANNER_AUDIO)),.cwav)
	BANNER_AUDIO_ARG := -ca
else
	BANNER_AUDIO_ARG := -a
endif

$(BUILD)/banner.bnr :  $(BANNER_IMAGE) $(BANNER_AUDIO)
	$(BANNERTOOL) makebanner $(BANNER_IMAGE_ARG) "$(BANNER_IMAGE)" $(BANNER_AUDIO_ARG) "$(BANNER_AUDIO)" -o "$@"

$(BUILD)/icon.icn   :  $(APP_ICON)
	$(BANNERTOOL) makesmdh -s "$(APP_TITLE)" -l "$(APP_DESCRIPTION)" -p "$(APP_AUTHOR)" -i "$(APP_ICON)" -f "$(ICON_FLAGS)" -o "$@"


#---------------------------------------------------------------------------------
else

DEPENDS	:=	$(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------

$(OUTPUT).elf	:	$(OFILES)

#---------------------------------------------------------------------------------
# you need a rule like this for each extension you use as binary data
#---------------------------------------------------------------------------------
%.bin.o	:	%.bin
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)

#---------------------------------------------------------------------------------
# rules for assembling GPU shaders
#---------------------------------------------------------------------------------
define shader-as
	$(eval CURBIN := $(patsubst %.shbin.o,%.shbin,$(notdir $@)))
	picasso -o $(CURBIN) $1
	bin2s $(CURBIN) | $(AS) -o $@
	echo "extern const u8" `(echo $(CURBIN) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`"_end[];" > `(echo $(CURBIN) | tr . _)`.h
	echo "extern const u8" `(echo $(CURBIN) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`"[];" >> `(echo $(CURBIN) | tr . _)`.h
	echo "extern const u32" `(echo $(CURBIN) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`_size";" >> `(echo $(CURBIN) | tr . _)`.h
endef

%.shbin.o : %.v.pica %.g.pica
	@echo $(notdir $^)
	@$(call shader-as,$^)

%.shbin.o : %.v.pica
	@echo $(notdir $<)
	@$(call shader-as,$<)

%.shbin.o : %.shlist
	@echo $(notdir $<)
	@$(call shader-as,$(foreach file,$(shell cat $<),$(dir $<)/$(file)))

-include $(DEPENDS)

#---------------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------------
