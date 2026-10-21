# Independent AArch32 runtime; does not build legacy game-specific sources.
TARGET := mc4_game
BUILD := build/game
SOURCES := source
PORT_NPDM_PROGRAM_ID := 0x0100F53447550002
PORT_NPDM_NAME := moderncombat4
PORT_NPDM_VERSION := 0.1.0
PORT_NPDM_MAIN_STACK := 0x800000
DCR_GL_MESA := 1
PORT_MESA := 20
PORTLIBS ?= $(CURDIR)/portlibs32
include runtime/runtime.mk

# Standard devkitPro elf2nso accepts only AArch64; this runtime is ELF32 ARM.
$(BUILD)/$(TARGET).nso: $(TARGET).elf tools/elf32_to_nso.py | $(BUILD)
	python3 tools/elf32_to_nso.py $< $@
