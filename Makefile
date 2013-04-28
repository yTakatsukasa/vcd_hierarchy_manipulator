V					?= 0
DEBUG				?= 0
SRC_DIRS			:= .
INCLUDE_DIRS		:= 
CC					?= gcc
CXX					?= g++
CPP					?= cpp
CPPFLAGS			:= $(addprefix -I,$(INCLUDE_DIRS))
ifeq ($(DEBUG),1)
DEBUGGER_CMD		:= gdb --args
CXXFLAGS			:= -g3 -O0 -Wall 
V					:= 1
else
DEBUGGER_CMD		:=
CXXFLAGS			:= -O3 -Wall -march=nocona -fomit-frame-pointer
endif
CFLAGS				:= $(CXXFLAGS)
LIB_DIRS			:=
LIBS				:=
LDFLAGS             := $(addprefix -L,$(LIB_DIRS)) $(addprefix -l,$(LIBS))
SRCS				= $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.cpp $(dir)/*.c))
OBJS				= $(addprefix .,$(addsuffix .o,$(basename $(notdir $(SRCS)))))
ifeq ($V,1)
SHOW_CMD_LINE   := 
SHOW_MSG        := > /dev/null
else
SHOW_CMD_LINE   := @
SHOW_MSG        :=
endif
ifeq (${MAKE_PROF},1)
CXXFLAGS		+= -fprofile-arcs
LDFLAGS			+= -fprofile-arcs
endif
ifeq (${USE_PROF},1)
CXXFLAGS		+= -fbranch-probabilities
LDFLAGS			+= -fbranch-probabilities
endif
.PHONY:clean runall

vpath %.cpp $(SRC_DIRS)
vpath %.c $(SRC_DIRS)


run.x:$(OBJS)
	@echo Linking $@ $(SHOW_MSG)
	$(SHOW_CMD_LINE) $(CXX) -o $@ $^ $(LDFLAGS)

.%.o:%.cpp
	@echo Compiling $< $(SHOW_MSG)
	$(SHOW_CMD_LINE) $(CXX)	$(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

.%.d:%.cpp
	@echo Generating dependency list of $< $(SHOW_MSG)
	@echo -n "$@ ." > $@
	$(SHOW_CMD_LINE) $(CPP) $(CPPFLAGS) $< -MM >> $@ ||  rm -f $@ || /bin/false

clean:
	rm -f .*.[do] *.x .*.*target transcript vsim.wlf
	rm -rf work verilated

-include $(addsuffix .d,$(basename $(OBJS)))
