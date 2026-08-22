CXX      := g++
STD      := -std=c++17
WARN     := -Wall -Wextra -Wpedantic
OPTFLAGS := -O2
DBGFLAGS := -O0 -g

VERSION  := 1.1.0
TARGET   := getip
TESTBIN  := tests/test_runner
TESTBIN_SAN := tests/test_runner_san
LIBS     := -lcurl

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
	rm -f $(TARGET) $(TESTBIN) $(TESTBIN_SAN)
	rm -rf getip_test_tmp_*
