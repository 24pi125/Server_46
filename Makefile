CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -Iinclude
LDFLAGS = -lssl -lcrypto
BUILD_DIR = build
SRC_DIR = src
CLIENT_DIR = clients

SERVER_SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
SERVER_OBJECTS = $(SERVER_SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
SERVER_TARGET = server

CLIENT_SOURCES = $(CLIENT_DIR)/test_client.cpp
CLIENT_TARGET = test_client

.PHONY: all clean install run-server run-client

all: $(SERVER_TARGET) $(CLIENT_TARGET)

$(SERVER_TARGET): $(SERVER_OBJECTS)
	$(CXX) $(SERVER_OBJECTS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(CLIENT_TARGET): $(CLIENT_SOURCES)
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR) $(SERVER_TARGET) $(CLIENT_TARGET)

install: $(SERVER_TARGET)
	sudo cp $(SERVER_TARGET) /usr/local/bin/

run-server: $(SERVER_TARGET)
	./$(SERVER_TARGET)

run-client: $(CLIENT_TARGET)
	./$(CLIENT_TARGET)

debug: CXXFLAGS += -g
debug: clean all

.PHONY: setup
setup:
	sudo mkdir -p /etc/
	echo "user1:password1" | sudo tee /etc/vealc.conf
	echo "user2:password2" | sudo tee -a /etc/vealc.conf
	echo "admin:admin123" | sudo tee -a /etc/vealc.conf
	sudo touch /var/log/vealc.log
	sudo chmod 666 /var/log/vealc.log
