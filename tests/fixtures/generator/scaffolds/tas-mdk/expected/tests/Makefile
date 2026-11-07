COMPONENT_NAME=My-Proj.1--tests

ifndef COMPILE_CHAIN
$(error Variable COMPILE_CHAIN is not defined)
endif

-include $(COMPILE_CHAIN)/component_declaration.mk
include $(COMPILE_CHAIN)/rules_variants.mk

$(COMPONENT_NAME)_tests: $(COMPONENT_NAME)_tests_python
$(COMPONENT_NAME)_tests_python:
	COMPONENT_NAME=$(COMPONENT_NAME) \
	VT_STDOUT=$(VT_STDOUT) \
	PATH=${ROOT_OBJ}/gram_addons--simulator_launcher/BIN:${PATH} \
	PYTHONPATH=${PYTHONPATH} \
	LD_LIBRARY_PATH=${LD_LIBRARY_PATH} \
	python3 -m gram_addons__python_test_suite.runtests $(TEST_ARGS)
