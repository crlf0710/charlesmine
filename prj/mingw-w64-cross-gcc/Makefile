export PATH    := /opt/bin:$(PATH)
export CC      := i686-w64-mingw32-g++.exe
export CXX     := i686-w64-mingw32-g++.exe
export WINDRES := i686-w64-mingw32-windres.exe

.PHONY: all clean

all:
	@$(MAKE) -C ../shared all  --no-print-directory

clean:
	@$(MAKE) -C ../shared clean --no-print-directory
