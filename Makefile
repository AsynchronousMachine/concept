LDLIBS          := -lboost_timer -lboost_thread -lboost_system -lpthread -ltbb
LDFLAGS         := -L/usr/local/lib #e.g. custom boost lib dir
TARGET          := asmExec
STRIP           ?= strip
CPPFLAGS        += -std=c++1z -fdiagnostics-color=always -Ofast -Wall -Wfatal-errors -I/usr/local/include #e.g. custom boost include dir

all: $(TARGET)

$(TARGET): src/*
	$(CXX) $(LDFLAGS) $(CPPFLAGS) -o $(TARGET) src/asm/asm.cpp src/modules/*.cpp src/maker/maker_reflection.cpp src/communication/ReceiveHandler.cpp src/testcases/*.cpp src/main.cpp $(LDLIBS)

clean:
	-rm $(TARGET)

valgrind:
	valgrind --version
	valgrind --leak-check=full --track-origins=yes --xml=yes --xml-file=ValgrindOut.xml "./$(TARGET)"

cachegrind:
	valgrind --version
	valgrind --tool=cachegrind "./$(TARGET)"

# run with custom lib path: LD_LIBRARY_PATH="/usr/local/lib" ./asmExec
