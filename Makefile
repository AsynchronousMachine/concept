LDLIBS          := -lboost_timer -lboost_thread -lboost_system -lpthread -ltbb
LDFLAGS         := -L/usr/local/lib #e.g. custom boost lib dir
TARGET          := asmExec
STRIP           ?= strip
CPPFLAGS        += -std=c++1z -fdiagnostics-color=always -Ofast -Wall \
                   -I/usr/local/include #e.g. custom boost include dir

all: $(TARGET)

$(TARGET): src/*
	$(CXX) $(LDFLAGS) $(CPPFLAGS) -o $(TARGET) src/asm/asm.cpp src/modules/global_modules.cpp src/maker/maker_reflection.cpp src/testcases/*.cpp src/main.cpp $(LDLIBS)

clean:
	-rm $(TARGET)

# run with custom lib path: LD_LIBRARY_PATH="/usr/local/lib" ./asmExec
