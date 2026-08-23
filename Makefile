CXX      := g++
STD      := -std=c++17
WARN     := -Wall -Wextra -Wpedantic
OPTFLAGS := -O2
DBGFLAGS := -O0 -g

VERSION  := 1.2.0
TARGET   := getip
TESTBIN  := tests/test_runner
TESTBIN_SAN := tests/test_runner_san

# Alvo do sistema: alvo nativo por padrao; passe TARGET_OS=Windows_NT ao
# compilar com mingw-w64 (x86_64-w64-mingw32-g++ / i686-w64-mingw32-g++).
TARGET_OS ?= $(shell uname -s)

# Windows (MinGW): binario 100% estatico com transporte WinHTTP nativo.
# POSIX: transporte libcurl dinamico (libcurl4-openssl-dev).
ifeq ($(TARGET_OS),Windows_NT)
  LIBS := -static -static-libgcc -static-libstdc++ -lwinhttp
else
  LIBS := -lcurl
endif

CXXFLAGS := $(STD) $(WARN)

.PHONY: all check test sanitize version install clean

all: $(TARGET)

$(TARGET): main.cpp getip.cpp config.cpp getip.hpp
	$(CXX) $(CXXFLAGS) $(OPTFLAGS) -DGETIP_VERSION=\"$(VERSION)\" -o $@ main.cpp getip.cpp config.cpp $(LIBS)

# Verificacao estatica: warnings tratados como erro
check:
	$(CXX) $(CXXFLAGS) -Werror -fsyntax-only main.cpp getip.cpp config.cpp
	@echo "verificacao ok (zero warnings)"

$(TESTBIN): tests/getip_tests.cpp getip.cpp config.cpp getip.hpp tests/test_framework.hpp
	$(CXX) $(CXXFLAGS) $(DBGFLAGS) -DGETIP_VERSION=\"$(VERSION)\" -o $@ tests/getip_tests.cpp getip.cpp config.cpp $(LIBS)

test: $(TESTBIN)
	./$(TESTBIN)

# Build com sanitizers (AddressSanitizer + UndefinedBehaviorSanitizer)
$(TESTBIN_SAN): tests/getip_tests.cpp getip.cpp config.cpp getip.hpp tests/test_framework.hpp
	$(CXX) $(CXXFLAGS) $(DBGFLAGS) -fsanitize=address,undefined -fno-omit-frame-pointer -DGETIP_VERSION=\"$(VERSION)\" -o $@ tests/getip_tests.cpp getip.cpp config.cpp $(LIBS)

sanitize: $(TESTBIN_SAN)
	./$(TESTBIN_SAN)

version:
	@echo "$(TARGET) v$(VERSION)"

install: $(TARGET)
	install -m 0755 $(TARGET) /usr/local/bin/$(TARGET)

clean:
	rm -f $(TARGET) $(TARGET).exe $(TESTBIN) $(TESTBIN_SAN)
	rm -rf getip_test_tmp_*
