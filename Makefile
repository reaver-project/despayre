CC=clang++
LD=clang++
CFLAGS=-c -Os -Wall -Wextra -pedantic -Werror -std=c++1y -stdlib=libc++ -g -MD -pthread -fPIC -Wno-unused-private-field
LDFLAGS=-stdlib=libc++ -lc++abi
SOFLAGS=-stdlib=libc++ -shared
SOURCES=$(shell find . -type f -name "*.cpp" ! -path "./main.cpp" ! -path "./tests/*" ! -name "buildlist.cpp")
TESTSRC=$(shell find ./tests/ -type f -name "*.cpp")
OBJECTS=$(SOURCES:.cpp=.o)
TESTOBJ=$(TESTSRC:.cpp=.o)
LIBRARY=libdespayre.so
EXECUTABLE=despayre
TESTEXE=despayre-test

all: $(SOURCES) $(LIBRARY) $(EXECUTABLE) $(TESTEXE)

library: $(LIBRARY)

library-install: $(LIBRARY)
	@sudo mkdir -p /usr/local/include/reaver/despayre
	@find . -name "*.h" ! -path "*-old" ! -name "despayre.h" | sudo cpio -pdm /usr/local/include/reaver/despayre 2> /dev/null
	@sudo cp despayre.h /usr/local/include/reaver
	@sudo cp $(LIBRARY) /usr/local/lib/$(LIBRARY).1.0
	@sudo ln -sfn /usr/local/lib/$(LIBRARY).1.0 /usr/local/lib/$(LIBRARY).1
	@sudo ln -sfn /usr/local/lib/$(LIBRARY).1.0 /usr/local/lib/$(LIBRARY)

$(EXECUTABLE): library-install main.o
	$(LD) $(LDFLAGS) -o $@ main.o -lreaver -ldespayre -pthread

test: $(TESTEXE)

$(TESTEXE): library-install $(TESTOBJ)
	$(LD) $(LDFLAGS) -o $@ $(TESTOBJ) -lreaver -ldespayre -pthread -lboost_system -lboost_program_options -lboost_iostreams -lboost_filesystem -ldl

$(LIBRARY): $(OBJECTS)
	$(LD) $(SOFLAGS) -o $@ $(OBJECTS) -lreaver

%.o: %.cpp
	$(CC) $(CFLAGS) $< -o $@

clean:
	@find . -name "*.o" -delete
	@find . -name "*.d" -delete
	@find . -name "*.so" -delete
	@rm -rf $(EXECUTABLE)

-include $(SOURCES:.cpp=.d)
-include $(TESTSRC:.cpp=.d)
-include main.d
