CPPFLAGS        += -fdiagnostics-color=always -Wall \
                   -Iexternal/spdlog/include -Iexternal/rapidjson/include \
                   -DVERSION=\"$(shell git describe --tags --always --dirty)\" -DBUILD_TIMESTAMP=\"$(shell date -u '+%FT%T')\"

#Enable if e.g. custom boost or tbb include dir
CPPFLAGSCUSTOM  = -I/opt/local/include

LDLIBS          = -lboost_system -lboost_chrono -lboost_thread  -lpthread -ltbb -lstdc++fs

#Enable if e.g. custom boost or tbb lib dir
LDFLAGSCUSTOM   = -L/opt/local/lib

FILES           = src/asm/asm.cpp src/maker/maker_reflection.cpp src/communication/ReceiveHandler.cpp src/logger/logger.cpp src/modules/*.cpp src/testcases/*.cpp src/main.cpp
TARGET          = asm
STRIP           = strip -s
ECHO            = echo

.PHONY: help clean ctags

help:
	@$(ECHO) "Available targets are:"
	@$(ECHO) "make $(TARGET)_debug"
	@$(ECHO) "make $(TARGET)_release"
	@$(ECHO) "make clean"
	@$(ECHO) "make valgrind"
	@$(ECHO) "make cachegrind"
	@$(ECHO) "make ctags"

$(TARGET)_debug: $(FILES)
	$(CXX) $(CPPFLAGS) -Og -g -DTBB_USE_DEBUG=1 -DSPDLOG_DEBUG_ON -DSPDLOG_TRACE_ON $(CPPFLAGSCUSTOM) $(LDFLAGS) $(LDFLAGSCUSTOM) -o $@ $? $(LDLIBS)

$(TARGET)_release: $(FILES)
	$(CXX) $(CPPFLAGS) -Ofast $(CPPFLAGSCUSTOM) $(LDFLAGS) $(LDFLAGSCUSTOM) -o $@ $? $(LDLIBS)
	$(STRIP) $@

clean:
	-$(RM) $(TARGET)_release $(TARGET)_debug CppCheck* cachegrind*

valgrind: $(TARGET)_debug
	@valgrind --version
	valgrind --leak-check=full --track-origins=yes --xml=yes --xml-file=ValgrindOut.xml "./$(TARGET)_debug"

cachegrind: $(TARGET)_release
	@valgrind --version
	valgrind --tool=cachegrind "./$(TARGET)_release"

ctags:
	@ctags -R .
# E.g. run with custom lib path: LD_LIBRARY_PATH="/usr/local/lib" ./asm_xxx
