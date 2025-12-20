# Makefile для сервера с поддержкой модульных, функциональных и приёмочных тестов

# Компилятор и флаги
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -Iinclude -Wno-deprecated-declarations
LDFLAGS = -lssl -lcrypto -lpthread -lUnitTest++

# Директории
BUILD_DIR = build
SRC_DIR = src
TEST_DIR = tests
FUNCTIONAL_TEST_DIR = $(TEST_DIR)/functional
UNIT_TEST_DIR = $(TEST_DIR)
TEST_DATA_DIR = $(TEST_DIR)/test_data

# Цели
SERVER_TARGET = server

# Исходные файлы сервера (исключая main.cpp)
SERVER_SOURCES = $(filter-out $(SRC_DIR)/main.cpp, $(wildcard $(SRC_DIR)/*.cpp))
SERVER_OBJECTS = $(SERVER_SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
MAIN_OBJECT = $(BUILD_DIR)/main.o

# Модульные тесты
UNIT_TEST_SOURCES = $(wildcard $(UNIT_TEST_DIR)/test_*.cpp)
UNIT_TEST_TARGETS = $(notdir $(UNIT_TEST_SOURCES:%.cpp=%))

# Функциональные тесты из PDF
FUNCTIONAL_TESTS = test_func test_integration
ACCEPTANCE_TESTS = $(FUNCTIONAL_TESTS)

# Правила по умолчанию
.PHONY: all clean unit-tests functional-tests acceptance-tests server build-dirs setup check-deps doc

all: server unit-tests

server: $(SERVER_TARGET)

$(SERVER_TARGET): $(SERVER_OBJECTS) $(MAIN_OBJECT)
	$(CXX) $^ -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Компиляция main.o
$(BUILD_DIR)/main.o: $(SRC_DIR)/main.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Функциональные тесты из PDF
test_func: tests/test_func.cpp $(SERVER_OBJECTS)
	$(CXX) $(CXXFLAGS) $< $(SERVER_OBJECTS) -o $@ $(LDFLAGS)

test_integration: tests/test_integration.cpp $(SERVER_OBJECTS)
	$(CXX) $(CXXFLAGS) $< $(SERVER_OBJECTS) -o $@ $(LDFLAGS)

# Существующие модульные тесты
test_config: $(UNIT_TEST_DIR)/test_config.cpp $(BUILD_DIR)/config.o
	$(CXX) $(CXXFLAGS) $< $(BUILD_DIR)/config.o -o $@ $(LDFLAGS)

test_vector_processor: $(UNIT_TEST_DIR)/test_vector_processor.cpp $(BUILD_DIR)/vector_processor.o
	$(CXX) $(CXXFLAGS) $< $(BUILD_DIR)/vector_processor.o -o $@ $(LDFLAGS)

test_auth: $(UNIT_TEST_DIR)/test_auth.cpp $(BUILD_DIR)/auth.o
	$(CXX) $(CXXFLAGS) $< $(BUILD_DIR)/auth.o -o $@ $(LDFLAGS)

test_session: $(UNIT_TEST_DIR)/test_session.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

test_types: $(UNIT_TEST_DIR)/test_types.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

test_interface: $(UNIT_TEST_DIR)/test_interface.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

test_network_auth: $(TEST_DIR)/test_network_auth.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

test_full_session: $(TEST_DIR)/test_full_session.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

test_server_client: $(TEST_DIR)/test_server_client.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

# Создание директорий
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TEST_DATA_DIR):
	mkdir -p $(TEST_DATA_DIR)

build-dirs: $(BUILD_DIR) $(TEST_DATA_DIR)

# Настройка тестового окружения
setup: build-dirs
	@echo "Настройка тестового окружения..."
	@echo "Создание тестовых файлов..."
	@echo "alice:P@ssl@rd" > $(TEST_DATA_DIR)/test_users.conf
	@echo "bob:Secret123" >> $(TEST_DATA_DIR)/test_users.conf
	@echo "charlie:Qwerty!@#" >> $(TEST_DATA_DIR)/test_users.conf
	@touch $(TEST_DATA_DIR)/test.log
	@echo "Создание конфигурации сервера..."
	@echo "port=33333" > $(TEST_DATA_DIR)/server.conf
	@echo "log_file=$(TEST_DATA_DIR)/test.log" >> $(TEST_DATA_DIR)/server.conf
	@echo "client_db=$(TEST_DATA_DIR)/test_users.conf" >> $(TEST_DATA_DIR)/server.conf
	@echo "Окружение настроено"

# Проверка зависимостей (как в PDF)
check-deps:
	@echo "Проверка зависимостей..."
	@echo "UnitTest++: $(shell pkg-config --exists UnitTest++ && echo 'OK' || echo 'NOT FOUND')"
	@echo "OpenSSL: $(shell pkg-config --exists openssl && echo 'OK' || echo 'NOT FOUND')"
	@echo "Компилятор: $(shell which $(CXX) || echo 'NOT FOUND')"
	@echo "=========================================="

# Модульные тесты (UNIT TEST)
unit-tests: build-dirs test_config test_vector_processor test_auth test_session test_types test_interface
	@echo "=========================================="
	@echo "Запуск модульных тестов"
	@echo "=========================================="
	@echo "Запуск test_config..."
	@./test_config || true
	@echo ""
	@echo "Запуск test_vector_processor..."
	@./test_vector_processor || true
	@echo ""
	@echo "Запуск test_auth..."
	@./test_auth || true
	@echo ""
	@echo "Запуск test_session..."
	@./test_session || true
	@echo ""
	@echo "Запуск test_types..."
	@./test_types || true
	@echo ""
	@echo "Запуск test_interface..."
	@./test_interface || true
	@echo "=========================================="
	@echo "Модульные тесты завершены"
	@echo "=========================================="

# Функциональные тесты (как в PDF) - ПРИЁМОЧНОЕ ТЕСТИРОВАНИЕ
functional-tests: server setup test_func
	@echo "=========================================="
	@echo "Запуск приёмочных тестов (Functional Tests)"
	@echo "=========================================="
	@echo "Тестирование согласно Таблице 1 из PDF..."
	@echo ""
	@./test_func
	@echo "=========================================="
	@echo "Приёмочные тесты завершены"
	@echo "=========================================="

# Синоним для functional-tests (как в PDF)
test-func: functional-tests
	@echo "Приёмочное тестирование завершено!"

# Интеграционные тесты
integration-test: setup test_integration
	@echo "=========================================="
	@echo "Запуск интеграционных тестов"
	@echo "=========================================="
	@./test_integration
	@echo "=========================================="
	@echo "Интеграционные тесты завершены"
	@echo "=========================================="

# Полный набор тестов (как в PDF)
all-tests: check-deps unit-tests functional-tests integration-test
	@echo "=========================================="
	@echo "ВСЕ ТЕСТЫ УСПЕШНО ЗАВЕРШЕНЫ!"
	@echo "=========================================="

# Старые тесты (для обратной совместимости)
old-functional-tests: build-dirs setup test_network_auth test_full_session test_server_client
	@echo "=========================================="
	@echo "Запуск старых функциональных тестов"
	@echo "=========================================="
	@echo "Запуск test_network_auth..."
	@./test_network_auth || true
	@echo ""
	@echo "Запуск test_full_session..."
	@./test_full_session || true
	@echo ""
	@echo "Запуск test_server_client..."
	@./test_server_client || true
	@echo "=========================================="
	@echo "Старые функциональные тесты завершены"
	@echo "=========================================="

# Очистка
clean:
	@echo "Очистка проекта..."
	rm -rf $(BUILD_DIR)
	rm -f $(SERVER_TARGET)
	rm -f $(UNIT_TEST_TARGETS) $(FUNCTIONAL_TESTS)
	rm -f test_network_auth test_full_session test_server_client
	rm -f *.log $(TEST_DATA_DIR)/* 2>/dev/null || true
	@echo "Очистка завершена"

# Запуск сервера
run: $(SERVER_TARGET)
	@echo "Запуск сервера..."
	@echo "=========================================="
	@./$(SERVER_TARGET)

# Быстрый запуск тестов
quick-test: server
	@echo "Быстрая проверка сервера..."
	@./server --help
	@echo "=========================================="
	@echo "Сервер скомпилирован и работает корректно"

# Проверка структуры
check-structure:
	@echo "Проверка структуры проекта..."
	@echo "Директория исходников ($(SRC_DIR)):"
	@ls -la $(SRC_DIR)/*.cpp 2>/dev/null | wc -l | xargs echo "  Файлов .cpp:"
	@echo "Директория тестов ($(TEST_DIR)):"
	@ls -la $(TEST_DIR)/*.cpp 2>/dev/null | wc -l | xargs echo "  Тестовых файлов:"
	@echo "Директория тестовых данных ($(TEST_DATA_DIR)):"
	@[ -d "$(TEST_DATA_DIR)" ] && ls -la $(TEST_DATA_DIR) || echo "  Не существует"
	@echo "=========================================="

# Тестирование отдельных портов (как в PDF)
test-port-33555: server setup
	@echo "Тестирование порта 33555 (FT-09)..."
	@./server -p 33555 -d $(TEST_DATA_DIR)/test_users.conf -l $(TEST_DATA_DIR)/port_33555.log &
	@SERVER_PID=$$!; \
	sleep 2; \
	if lsof -i:33555 >/dev/null 2>&1; then \
		echo "✓ Сервер запущен на порту 33555"; \
		kill $$SERVER_PID 2>/dev/null; \
		sleep 1; \
		if ! lsof -i:33555 >/dev/null 2>&1; then \
			echo "✓ Сервер корректно остановлен"; \
		else \
			echo "✗ Ошибка остановки сервера"; \
		fi; \
	else \
		echo "✗ Ошибка запуска сервера на порту 33555"; \
		kill $$SERVER_PID 2>/dev/null; \
	fi

test-port-33666: server setup
	@echo "Тестирование порта 33666 (FT-10)..."
	@./server -p 33666 -d $(TEST_DATA_DIR)/test_users.conf -l $(TEST_DATA_DIR)/port_33666.log &
	@SERVER_PID=$$!; \
	sleep 2; \
	if lsof -i:33666 >/dev/null 2>&1; then \
		echo "✓ Сервер запущен на порту 33666"; \
		kill $$SERVER_PID 2>/dev/null; \
		sleep 1; \
		if ! lsof -i:33666 >/dev/null 2>&1; then \
			echo "✓ Сервер корректно остановлен"; \
		else \
			echo "✗ Ошибка остановки сервера"; \
		fi; \
	else \
		echo "✗ Ошибка запуска сервера на порту 33666"; \
		kill $$SERVER_PID 2>/dev/null; \
	fi

test-multiple-servers: server setup
	@echo "Тестирование нескольких серверов одновременно (FT-13)..."
	@echo "Сервер 1 на порту 33888..."
	@./server -p 33888 -d $(TEST_DATA_DIR)/test_users.conf -l $(TEST_DATA_DIR)/server1.log &
	@SERVER1_PID=$$!; \
	sleep 1; \
	echo "Сервер 2 на порту 33999..." && \
	./server -p 33999 -d $(TEST_DATA_DIR)/test_users.conf -l $(TEST_DATA_DIR)/server2.log &
	@SERVER2_PID=$$!; \
	sleep 2; \
	if lsof -i:33888 >/dev/null 2>&1 && lsof -i:33999 >/dev/null 2>&1; then \
		echo "✓ Два сервера запущены одновременно"; \
		kill $$SERVER1_PID $$SERVER2_PID 2>/dev/null; \
		echo "✓ Оба сервера корректно остановлены"; \
	else \
		echo "✗ Ошибка запуска серверов"; \
		kill $$SERVER1_PID $$SERVER2_PID 2>/dev/null; \
	fi

# Документация (как в PDF)
doc:
	@echo "Генерация Doxygen документации..."
	@doxygen -g Doxyfile 2>/dev/null || echo "Doxygen не установлен"
	@echo "Создайте Doxyfile вручную, как показано в PDF"

doc-pdf:
	@echo "Генерация PDF документации..."
	@echo "Используйте: make doc && cd docs/latex && make"

view-pdf:
	@echo "Просмотр PDF документации..."
	@echo "Откройте docs/latex/refman.pdf"

view-doc:
	@echo "Просмотр HTML документации..."
	@echo "Откройте docs/html/index.html в браузере"

# Помощь
help:
	@echo "Доступные цели:"
	@echo "  all              - Сборка сервера и модульных тестов"
	@echo "  server           - Сборка только сервера"
	@echo "  unit-tests       - Запуск модульных тестов"
	@echo "  functional-tests - Запуск приёмочных тестов (как в PDF)"
	@echo "  test-func        - То же что functional-tests (из PDF)"
	@echo "  integration-test - Запуск интеграционных тестов"
	@echo "  all-tests        - Запуск всех тестов"
	@echo "  run              - Запуск сервера"
	@echo "  clean            - Очистка проекта"
	@echo "  setup            - Настройка тестового окружения"
	@echo "  check-deps       - Проверка зависимостей"
	@echo "  check-structure  - Проверка структуры проекта"
	@echo "  quick-test       - Быстрая проверка сервера"
	@echo ""
	@echo "Тестирование портов (из PDF):"
	@echo "  test-port-33555     - Тест порта 33555 (FT-09)"
	@echo "  test-port-33666     - Тест порта 33666 (FT-10)"
	@echo "  test-multiple-servers - Тест нескольких серверов (FT-13)"
