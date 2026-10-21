TARGET := shadowguardian_nx
PORT_NPDM_PROGRAM_ID := 0x055A3A4744DC9101
PORT_NPDM_VERSION := 1.0.0
DCR_GL_MESA := 1
include runtime/runtime.mk

.PHONY: imports
imports:
	python3 runtime/tools/gen_imports.py --check
