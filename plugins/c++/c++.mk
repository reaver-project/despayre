PLUGIN := libdespayre.c++.so
DIR := plugins/c++
PLUGINSRC := $(shell find $(DIR) -name "*.cpp")
PLUGINOBJ := $(PLUGINSRC:.cpp=.o)

$(PLUGIN): $(PLUGINOBJ)
	$(LD) $(CXXFLAGS) $(SOFLAGS) $(PLUGINOBJ) -o $@ $(PLUGINLIBS)

all: $(PLUGIN)

clean-$(PLUGIN):
	@rm -rf $(PLUGIN)

clean: clean-$(PLUGIN)

