#
# Makefile
#
#CC 				?= gcc
makefile_name := $(abspath $(lastword $(MAKEFILE_LIST)))
makefile_path := $(strip $(patsubst %/,% , $(dir $(makefile_name))))

CC				?= gcc
PRJ_DIR  := $(makefile_path)
LVGL_DIR_NAME 	?= lvgl
LVGL_DIR := $(PRJ_DIR)

WARNINGS		:= -Wall -Wshadow -Wundef -Wmissing-prototypes -Wno-discarded-qualifiers -Wextra -Wno-unused-function -Wno-error=strict-prototypes -Wpointer-arith \
					-fno-strict-aliasing -Wno-error=cpp -Wuninitialized -Wmaybe-uninitialized -Wno-unused-parameter -Wno-missing-field-initializers -Wtype-limits \
					-Wsizeof-pointer-memaccess -Wno-format-nonliteral -Wno-cast-qual -Wunreachable-code -Wno-switch-default -Wreturn-type -Wmultichar -Wformat-security \
					-Wno-ignored-qualifiers -Wno-error=pedantic -Wno-sign-compare -Wno-error=missing-prototypes -Wdouble-promotion -Wclobbered -Wdeprecated -Wempty-body \
					-Wstack-usage=2048 -Wno-unused-value -std=gnu99
CFLAGS 			?= -O3 -g0 -I$(LVGL_DIR)/ $(WARNINGS)
CFLAGS 			+= -I$(makefile_path)/ -c -fmessage-length=0
CFLAGS 			+= $(shell sdl2-config --cflags)
CFLAGS 			+= $(shell pkg-config --cflags libavcodec libavformat libavutil libswscale libswresample)

LDFLAGS 		?= -lm -lstdc++ -lcurl -lavcodec -lavformat -lavutil -lswscale -lswresample
LDFLAGS 		+= $(shell sdl2-config --libs) -lSDL2_mixer 

BIN 			= lvgl_demo

BUILD_DIR 		= ./build
BUILD_OBJ_DIR 	= $(BUILD_DIR)/obj
BUILD_BIN_DIR 	= $(BUILD_DIR)/bin

prefix 			?= /usr
bindir 			?= $(prefix)/bin

#Collect the files to compile
MAINSRC          = ./main.c

include $(LVGL_DIR)/lvgl/lvgl.mk
include $(LVGL_DIR)/lv_drivers/lv_drivers.mk
include $(LVGL_DIR)/generated/generated.mk
include $(LVGL_DIR)/custom/custom.mk
#CSRCS 			+=$(LVGL_DIR)/mouse_cursor_icon.c 

OBJEXT 			?= .o

AOBJS 			= $(ASRCS:.S=$(OBJEXT))
COBJS 			= $(CSRCS:.c=$(OBJEXT))
CXXOBJS 		= $(CXXSRCS:.cpp=$(OBJEXT))

DEPS += $(addprefix $(GEN_OBJ_DIR),$(CSRCS:.c=.d))

MAINOBJ 		= $(MAINSRC:.c=$(OBJEXT))

SRCS 			= $(ASRCS) $(CSRCS) $(CXXSRCS) $(MAINSRC)
OBJS 			= $(AOBJS) $(COBJS) $(CXXOBJS) $(MAINOBJ)
TARGET 			= $(addprefix $(BUILD_OBJ_DIR)/, $(patsubst ./%, %, $(OBJS)))

## MAINOBJ -> OBJFILES

all: default

$(BUILD_OBJ_DIR)/%.o: %.c lv_conf.h
	@mkdir -p $(dir $@)
	@$(CC)  $(CFLAGS) -c $< -o $@
	@echo "CC $<"

$(BUILD_OBJ_DIR)/%.o: %.S lv_conf.h
	@mkdir -p $(dir $@)
	@$(CC)  $(CFLAGS) -c $< -o $@
	@echo "CC $<"

$(BUILD_OBJ_DIR)/%.o: %.cpp lv_conf.h
	@mkdir -p $(dir $@)
	@$(CXX)  $(CFLAGS) -c $< -o $@
	@echo "CXX $<"

default: $(TARGET) $(GEN_OBJS)
	@mkdir -p $(dir $(BUILD_BIN_DIR)/)
	$(CC) -o $(BUILD_BIN_DIR)/$(BIN) $(TARGET) $(GEN_OBJS) $(LDFLAGS)

clean: 
	rm -rf $(BUILD_DIR)

install:
	install -d $(DESTDIR)$(bindir)
	install $(BUILD_BIN_DIR)/$(BIN) $(DESTDIR)$(bindir)

uninstall:
	$(RM) -r $(addprefix $(DESTDIR)$(bindir)/,$(BIN))
