#!/bin/bash

echo "========================================"
echo "Functional Tests for Vcalc Server"
echo "========================================"

# Создаем директорию для тестовых данных
mkdir -p tests/test_data

echo ""
echo "1. Creating test configuration files..."
cat > tests/test_data/test_clients.conf << EOF
alice:P@ssl@rd
bob:Secret123
charlie:Qwerty!@#
EOF

echo "2. Building server if needed..."
make server 2>/dev/null || {
    echo "Building server..."
    make clean
    make all
}

echo ""
echo "3. Running functional tests..."
echo "========================================"

# Запускаем тесты
./test_func

echo ""
echo "4. Cleaning up..."
rm -rf tests/test_data

echo ""
echo "========================================"
echo "Functional tests completed!"
echo "========================================"
