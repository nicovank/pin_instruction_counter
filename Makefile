ifndef PIN_ROOT
	$(error Please specify the PIN_ROOT environment variable)
endif

CONFIG_ROOT := $(PIN_ROOT)/source/tools/Config
include $(CONFIG_ROOT)/makefile.config
include $(CONFIG_ROOT)/makefile.default.rules

TOOL_NAME = instruction_counter

$(OBJDIR)$(TOOL_NAME)$(OBJ_SUFFIX): src/$(TOOL_NAME).cpp
	mkdir -p $(OBJDIR)
	$(CXX) $(TOOL_CXXFLAGS) -Iinclude $(COMP_OBJ) $@ $<

$(OBJDIR)$(TOOL_NAME)$(PINTOOL_SUFFIX): $(OBJDIR)$(TOOL_NAME)$(OBJ_SUFFIX)
	$(LINKER) $(TOOL_LDFLAGS) -Iinclude $(LINK_EXE) $@ $< $(TOOL_LPATHS) $(TOOL_LIBS)
