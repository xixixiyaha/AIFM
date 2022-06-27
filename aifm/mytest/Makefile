SHENANGO_PATH=../../shenango
include $(SHENANGO_PATH)/shared.mk

librt_libs = $(SHENANGO_PATH)/bindings/cc/librt++.a
INC += -I$(SHENANGO_PATH)/bindings/cc -I$(SHENANGO_PATH)/ksched -Iinc \
       -IDataFrame/AIFM/include/

my_test_array_add_src = ../test/my_test_array_add.cpp
my_test_array_add_obj = $(my_test_array_add_src:.cpp=.o)

lib_src = $(wildcard ../src/*.cpp)
lib_src := $(filter-out ../src/tcp_device_server.cpp,$(lib_src))
lib_obj = $(lib_src:.cpp=.o)

test_src = $(my_test_array_add_src) 
test_obj = $(test_src:.cpp=.o)

src = $(lib_src) $(test_src)
obj = $(src:.cpp=.o)
dep = $(obj:.o=.d)

tcp_device_server_src = ../src/tcp_device_server.cpp
tcp_device_server_obj = $(tcp_device_server_src:.cpp=.o)

override CXXFLAGS += -std=gnu++2a -fconcepts -Wno-unused-function
CXXFLAGS := $(filter-out -std=gnu++17,$(CXXFLAGS))
override LDFLAGS += -lnuma

all: bin/my_test_array_add libaifm.a

bin/my_test_array_add: $(my_test_array_add_obj) $(librt_libs) $(RUNTIME_DEPS) $(lib_obj)
	$(LDXX) -o $@ $(my_test_array_add_obj) $(lib_obj) $(librt_libs) $(RUNTIME_LIBS) $(LDFLAGS)

$(tcp_device_server_obj): $(tcp_device_server_src)
	$(CXX) $(CXXFLAGS) -c $< -o $@

libaifm.a: $(lib_obj)
	$(AR) rcs $@ $^

#rule to generate a dep file by using the C preprocessor
#(see man cpp for details on the - MM and - MT options)
%.d: %.cpp
	@$(CXX) $(CXXFLAGS) $< -MM -MT $(@:.d=.o) >$@
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

ifneq ($(MAKECMDGOALS),clean)
-include $(dep)   # include all dep files in the makefile
endif

.PHONY: clean
clean:
	rm -f ../src/*.o ../test/*.o $(dep) bin/* lib*.a
