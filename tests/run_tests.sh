#!/bin/bash
echo "========================================"
echo "Running All Tests for Server"
echo "========================================"

# Создаем директорию для сборки если её нет
mkdir -p ../build

# Переходим в корневую директорию проекта
cd ..

echo ""
echo "1. Cleaning previous builds..."
make clean

echo ""
echo "2. Building server and tests..."
make all
make unit-tests

echo ""
echo "3. Checking dependencies..."
make check-deps

echo ""
echo "4. Running unit tests..."
echo "========================================"
make unit-tests

echo ""
echo "5. Running integration test..."
echo "========================================"
make integration-test

echo ""
echo "========================================"
echo "All tests completed!"
echo "========================================"
