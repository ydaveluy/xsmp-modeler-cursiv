COMPONENT_NAME=My-Proj.1

ifndef COMPILE_CHAIN
$(error Variable COMPILE_CHAIN is not defined)
endif

-include $(COMPILE_CHAIN)/component_declaration.mk

CXX_STANDARD=11

LIBRARY_NAME=libmy_proj_1.so

include $(COMPILE_CHAIN)/rules_libs.mk

$(COMPONENT_NAME)_tests:
	$(MAKE) -C tests
	$(MAKE) -C tests tu
