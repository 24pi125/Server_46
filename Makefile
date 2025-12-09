# Makefile
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -Iinclude -Wno-deprecated-declarations
LDFLAGS = -lssl -lcrypto -lcryptopp
BUILD_DIR = build
SRC_DIR = src
CLIENT_DIR = clients
TEST_DIR = tests

SERVER_SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
SERVER_OBJECTS = $(SERVER_SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
SERVER_TARGET = server

CLIENT_TARGET = test_client
TEST_SERVER_CLIENT_TARGET = test_server_client

# Тестовые цели
UNIT_TEST_TARGETS = test_config test_vector_processor test_auth test_session
INTEGRATION_TEST_TARGET = test_server_integration

.PHONY: all clean install run-server run-client run-test-client unit-tests integration-test test-all setup check-deps debug

all: $(SERVER_TARGET) $(CLIENT_TARGET) $(TEST_SERVER_CLIENT_TARGET)

$(SERVER_TARGET): $(SERVER_OBJECTS)
	$(CXX) $(SERVER_OBJECTS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Клиенты
$(CLIENT_TARGET): $(CLIENT_DIR)/test_client.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

$(TEST_SERVER_CLIENT_TARGET): $(CLIENT_DIR)/test_client_for_test_server.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

# Модульные тесты
test_config: $(TEST_DIR)/test_config.cpp $(BUILD_DIR)/config.o
	$(CXX) $(CXXFLAGS) $< $(BUILD_DIR)/config.o -o $@ $(LDFLAGS) -lUnitTest++

test_vector_processor: $(TEST_DIR)/test_vector_processor.cpp $(BUILD_DIR)/vector_processor.o
	$(CXX) $(CXXFLAGS) $< $(BUILD_DIR)/vector_processor.o -o $@ $(LDFLAGS) -lUnitTest++

test_auth: $(TEST_DIR)/test_auth.cpp $(BUILD_DIR)/auth.o
	$(CXX) $(CXXFLAGS) $< $(BUILD_DIR)/auth.o -o $@ $(LDFLAGS) -lUnitTest++ -lcryptopp

test_session: $(TEST_DIR)/test_session.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS) -lUnitTest++ -lcryptopp

# Интеграционный тест
test_server_integration: $(TEST_DIR)/test_server_integration.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR) $(SERVER_TARGET) $(CLIENT_TARGET) $(TEST_SERVER_CLIENT_TARGET) \
		$(UNIT_TEST_TARGETS) $(INTEGRATION_TEST_TARGET) *.log /tmp/test_* /tmp/server_* test_data/

install: $(SERVER_TARGET)
	sudo cp $(SERVER_TARGET) /usr/local/bin/

run-server: $(SERVER_TARGET)
	./$(SERVER_TARGET)

run-client: $(CLIENT_TARGET)
	./$(CLIENT_TARGET)

run-test-client: $(TEST_SERVER_CLIENT_TARGET)
	./$(TEST_SERVER_CLIENT_TARGET)

# Тестирование
unit-tests: $(UNIT_TEST_TARGETS)
	@echo "=========================================="
	@echo "Running Unit Tests"
	@echo "=========================================="
	@for test in $(UNIT_TEST_TARGETS); do \
		echo "Running $$test..."; \
		./$$test; \
		echo ""; \
	done

integration-test: $(SERVER_TARGET) $(CLIENT_TARGET) $(INTEGRATION_TEST_TARGET)
	@echo "=========================================="
	@echo "Running Integration Test"
	@echo "=========================================="
	@./$(INTEGRATION_TEST_TARGET)

test-all: unit-tests integration-test
	@echo "=========================================="
	@echo "All Tests Completed"
	@echo "=========================================="

debug: CXXFLAGS += -g
debug: clean all

setup:
	@echo "Setting up test environment..."
	@mkdir -p test_data
	@echo "user:P@ssW0rd" > test_data/vealc.conf
	@echo "alice:P@ssl@rd" >> test_data/vealc.conf
	@echo "bob:secret456" >> test_data/vealc.conf
	@touch test_data/vealc.log
	@chmod 666 test_data/vealc.log
	@echo "✓ Test environment created in test_data/"
	@echo "✓ Config: test_data/vealc.conf"
	@echo "✓ Log: test_data/vealc.log"

check-deps:
	@echo "Checking dependencies..."
	@which g++ > /dev/null && echo "✓ g++ found" || echo "✗ g++ not found"
	@pkg-config --exists openssl && echo "✓ OpenSSL found" || echo "✗ OpenSSL not found"
	@ldconfig -p | grep -q libcryptopp && echo "✓ Crypto++ found" || echo "✗ Crypto++ not found"
	@ldconfig -p | grep -q libUnitTest++ && echo "✓ UnitTest++ found" || echo "✗ UnitTest++ not found"

# Правила для компиляции объектов из тестов (если понадобятся)
$(BUILD_DIR)/test_%.o: $(TEST_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Зависимости
$(BUILD_DIR)/main.o: $(SRC_DIR)/main.cpp include/server.h include/config.h
$(BUILD_DIR)/server.o: $(SRC_DIR)/server.cpp include/server.h include/logger.h include/config.h
$(BUILD_DIR)/session.o: $(SRC_DIR)/session.cpp include/session.h include/logger.h
$(BUILD_DIR)/config.o: $(SRC_DIR)/config.cpp include/config.h
$(BUILD_DIR)/logger.o: $(SRC_DIR)/logger.cpp include/logger.h
$(BUILD_DIR)/vector_processor.o: $(SRC_DIR)/vector_processor.cpp include/vector_processor.h include/types.h
$(BUILD_DIR)/auth.o: $(SRC_DIR)/auth.cpp include/auth.h
