CXX      := g++
STD      := -std=c++17
WARN     := -Wall -Wextra -Wpedantic
OPTFLAGS := -O2
DBGFLAGS := -O0 -g

VERSION  := 1.3.0
TARGET   := getip
TESTBIN  := tests/test_runner
TESTBIN_SAN := tests/test_runner_san

# Alvo do sistema: alvo nativo por padrao; passe TARGET_OS=Windows_NT ao
# compilar com mingw-w64 (x86_64-w64-mingw32-g++ / i686-w64-mingw32-g++).
TARGET_OS ?= $(shell uname -s)

# Windows (MinGW): binario 100% estatico com transporte WinHTTP nativo.
# POSIX: transporte libcurl dinamico (libcurl4-openssl-dev).
ifeq ($(TARGET_OS),Windows_NT)
  LIBS := -static -static-libgcc -static-libstdc++ -lwinhttp -Wl,--no-insert-timestamp
else
  LIBS := -lcurl
endif

CXXFLAGS := $(STD) $(WARN)

.PHONY: all check test sanitize version install clean

all: $(TARGET)

$(TARGET): main.cpp getip.cpp config.cpp selfupdate.cpp getip.hpp selfupdate.hpp
	$(CXX) $(CXXFLAGS) $(OPTFLAGS) -DGETIP_VERSION=\"$(VERSION)\" -o $@ main.cpp getip.cpp config.cpp selfupdate.cpp $(LIBS)

# Verificacao estatica: warnings tratados como erro
check:
	$(CXX) $(CXXFLAGS) -Werror -fsyntax-only main.cpp getip.cpp config.cpp selfupdate.cpp
	@echo "verificacao ok (zero warnings)"

$(TESTBIN): tests/getip_tests.cpp getip.cpp config.cpp selfupdate.cpp getip.hpp selfupdate.hpp tests/test_framework.hpp
	$(CXX) $(CXXFLAGS) $(DBGFLAGS) -DGETIP_VERSION=\"$(VERSION)\" -o $@ tests/getip_tests.cpp getip.cpp config.cpp selfupdate.cpp $(LIBS)

test: $(TESTBIN)
	./$(TESTBIN)

# Build com sanitizers (AddressSanitizer + UndefinedBehaviorSanitizer)
$(TESTBIN_SAN): tests/getip_tests.cpp getip.cpp config.cpp selfupdate.cpp getip.hpp selfupdate.hpp tests/test_framework.hpp
	$(CXX) $(CXXFLAGS) $(DBGFLAGS) -fsanitize=address,undefined -fno-omit-frame-pointer -DGETIP_VERSION=\"$(VERSION)\" -o $@ tests/getip_tests.cpp getip.cpp config.cpp selfupdate.cpp $(LIBS)

sanitize: $(TESTBIN_SAN)
	./$(TESTBIN_SAN)

version:
	@echo "$(TARGET) v$(VERSION)"

# Padrão na pasta do usuário (sem sudo), para o `getip update` conseguir se
# substituir. Outro destino: sudo make install BINDIR=/usr/local/bin
BINDIR ?= $(HOME)/.local/bin

install: $(TARGET)
	install -d $(DESTDIR)$(BINDIR)
	install -m 0755 $(TARGET) $(DESTDIR)$(BINDIR)/$(TARGET)

clean:
	rm -f $(TARGET) $(TARGET).exe $(TESTBIN) $(TESTBIN_SAN)
	rm -rf getip_test_tmp_*
