## SPDX-License-Identifier: MIT
## Copyright 2020 NXP

CSRCS += $(notdir $(wildcard $(PRJ_DIR)/custom/*.c))

DEPPATH += --dep-path $(PRJ_DIR)/custom
VPATH += :$(PRJ_DIR)/custom

CFLAGS += "-I$(PRJ_DIR)/custom"
