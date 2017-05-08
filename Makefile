LDLIBS          := -lboost_timer -lboost_thread -lboost_system
LDFLAGS         := -L/usr/local/lib #e.g. custom boost lib dir
TARGET_PLATFORM ?= linux
TARGET          := asmExec
STRIP           ?= strip
CPPFLAGS        += -std=c++17 -fdiagnostics-color=always -Ofast -Wall \
                   -DTARGET_PLATFORM=$(TARGET_PLATFORM) \
                   -I/usr/local/include #e.g. custom boost include dir

all: $(TARGET)

$(TARGET): src/*
	$(CXX) $(LDFLAGS) $(CPPFLAGS) -o $(TARGET) src/main.cpp $(LDLIBS)

clean:
	-rm $(TARGET)

# run with custom lib path: LD_LIBRARY_PATH="/usr/local/lib" ./asm
