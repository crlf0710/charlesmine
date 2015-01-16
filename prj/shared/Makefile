SHELL   := bash
CXX     ?= g++
CC      ?= gcc
WINDRES ?= windres
MKDIR   ?= mkdir
RM      ?= rm -f
RMDIR   ?= rm -rf

OBJDIR = ../../obj/
SRCDIR = ../../src/
BINDIR = ../../bin/

LIBS   = -static -mwindows -lmsimg32 -ladvapi32

TARGETNAME = CharlesMine.exe

.PHONY: all clean

all: $(BINDIR)$(TARGETNAME)

clean:
	-@echo "  RMDIR   obj/ bin/"; $(RMDIR) $(OBJDIR) $(BINDIR)

$(OBJDIR)%.o : $(SRCDIR)%.cpp
	@$(MKDIR) -p $(OBJDIR)
	@echo  "  CXX     $(^F)"; $(CXX) -c $< -o $@

$(OBJDIR)%.res: $(SRCDIR)%.rc 
	@$(MKDIR) -p $(OBJDIR)
	@echo  "  WINDRES $(^F)"; $(WINDRES) -i $< --input-format=rc -o $@ -O coff 

$(BINDIR)$(TARGETNAME) : $(patsubst $(SRCDIR)%.cpp, $(OBJDIR)%.o, $(wildcard $(SRCDIR)*.cpp)) $(patsubst $(SRCDIR)%.rc, $(OBJDIR)%.res, $(wildcard $(SRCDIR)*.rc))
	@$(MKDIR) -p $(BINDIR)
	@echo  "  LD      $(@F)"; $(CXX) $^ -o $@ $(LIBS)
