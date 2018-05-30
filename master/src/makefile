CXXFLAGS =  -g3 -O0 -w -fPIC -std=c++11 -DTHREAD
CXX = g++ -m64
CC = gcc 

BUILDDIR = ./
TARGETDIR = ../build/
TARGET = $(TARGETDIR)Master
LDFLAGS = -fPIC -shared

SRC = $(wildcard $(BUILDDIR)/*.c $(BUILDDIR)/*.cpp $(BUILDDIR)/*.cc)

OBJS = $(patsubst %.cpp,%.o,$(SRC))   

INCLUDES = -I../../depend/zk/include  -I../../depend/hlog/include -I../../depend/xlogger/include\
			-I../../depend/common
LIBDIR = -Wl,-rpath=./ -lpthread -ldl -L../../depend/zk/lib/ -lzookeeper_mt \
	-L../../depend/xlogger/lib -llog4cxx -lxlogger


all:$(TARGET)
	
$(TARGET): $(OBJS)
	$(CXX)   $(LIBDIR) -o $@ $^ 
	
%.o : %.cpp
	$(CXX) $(INCLUDES) -c $(CXXFLAGS) $< -o $@

%.o : %.c
	$(CXX) $(INCLUDES) -c $(CXXFLAGS) $< -o $@

%.o : %.cc
	$(CXX) $(INCLUDES) -c $(CXXFLAGS) $< -o $@

clean:
	@rm -rvf $(OBJS) $(TARGET)

install:$(TARGET)
	install -d $(TARGETDIR)
	install -m 0755  ../../depend/zk/lib/*  $(TARGETDIR)
	install -m 0755  ../../depend/xlogger/lib/lib*  $(TARGETDIR)
