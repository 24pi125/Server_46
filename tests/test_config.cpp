#include "../include/config.h"
#include <UnitTest++/UnitTest++.h>
#include <iostream>
#include <cstring>
#include <vector>
#include <stdexcept>
#include <cstdio>
#include <string>

SUITE(ConfigTest) {
    TEST(DefaultValues) {
        char* argv[] = {(char*)"program", nullptr};
        ServerConfig config = ServerConfig::parse_args(1, argv);
        
        CHECK_EQUAL("/etc/vealc.conf", config.client_db_file);
        CHECK_EQUAL("/var/log/vealc.log", config.log_file);
        CHECK_EQUAL(33333, config.port);
    }
    
    TEST(CustomPort) {
        char* argv[] = {(char*)"program", (char*)"-p", (char*)"44444", nullptr};
        ServerConfig config = ServerConfig::parse_args(3, argv);
        
        CHECK_EQUAL(44444, config.port);
        
        // Проверяем другие параметры остались по умолчанию
        CHECK_EQUAL("/etc/vealc.conf", config.client_db_file);
        CHECK_EQUAL("/var/log/vealc.log", config.log_file);
    }
    
    TEST(CustomConfigFileWithC) {
        char* argv[] = {(char*)"program", (char*)"-c", (char*)"./myconfig.conf", nullptr};
        ServerConfig config = ServerConfig::parse_args(3, argv);
        
        CHECK_EQUAL("./myconfig.conf", config.client_db_file);
        CHECK_EQUAL(33333, config.port); // порт по умолчанию
    }
    
    TEST(CustomConfigFileWithD) {
        char* argv[] = {(char*)"program", (char*)"-d", (char*)"./config.conf", nullptr};
        ServerConfig config = ServerConfig::parse_args(3, argv);
        
        CHECK_EQUAL("./config.conf", config.client_db_file);
    }
    
    TEST(CustomLogFile) {
        char* argv[] = {(char*)"program", (char*)"-l", (char*)"./custom.log", nullptr};
        ServerConfig config = ServerConfig::parse_args(3, argv);
        
        CHECK_EQUAL("./custom.log", config.log_file);
    }
    
    TEST(MultipleOptions) {
        char* argv[] = {
            (char*)"program", 
            (char*)"-p", (char*)"55555",
            (char*)"-c", (char*)"./config.conf",
            (char*)"-l", (char*)"./server.log",
            nullptr
        };
        ServerConfig config = ServerConfig::parse_args(7, argv);
        
        CHECK_EQUAL(55555, config.port);
        CHECK_EQUAL("./config.conf", config.client_db_file);
        CHECK_EQUAL("./server.log", config.log_file);
    }
    
    TEST(HelpOption) {
        // Проверяем что help не вызывает ошибок
        // Временно подавляем вывод для этого теста
        char* argv1[] = {(char*)"program", (char*)"-h", nullptr};
        
        // Сохраняем оригинальный stdout
        std::FILE* original_stdout = stdout;
        std::FILE* original_stderr = stderr;
        
        // Перенаправляем в /dev/null только для этого теста
        stdout = std::fopen("/dev/null", "w");
        stderr = std::fopen("/dev/null", "w");
        
        ServerConfig config1 = ServerConfig::parse_args(2, argv1);
        
        // Восстанавливаем stdout/stderr
        std::fclose(stdout);
        std::fclose(stderr);
        stdout = original_stdout;
        stderr = original_stderr;
        
        CHECK(true);
    }
    
    TEST(UnknownOption) {
        char* argv[] = {(char*)"program", (char*)"-x", (char*)"value", nullptr};
        
        // Временно подавляем stderr чтобы не видеть сообщение об ошибке
        std::FILE* original_stderr = stderr;
        stderr = std::fopen("/dev/null", "w");
        
        ServerConfig config = ServerConfig::parse_args(3, argv);
        
        // Восстанавливаем stderr
        std::fclose(stderr);
        stderr = original_stderr;
        
        // Проверяем что возвращаются значения по умолчанию
        CHECK_EQUAL("/etc/vealc.conf", config.client_db_file);
        CHECK_EQUAL(33333, config.port);
        CHECK_EQUAL("/var/log/vealc.log", config.log_file);
    }
    
    TEST(MissingArgument) {
        // Отсутствует аргумент для -p
        char* argv[] = {(char*)"program", (char*)"-p", nullptr};
        
        // Временно подавляем stderr
        std::FILE* original_stderr = stderr;
        stderr = std::fopen("/dev/null", "w");
        
        ServerConfig config = ServerConfig::parse_args(2, argv);
        
        // Восстанавливаем stderr
        std::fclose(stderr);
        stderr = original_stderr;
        
        // Должны получить значения по умолчанию
        CHECK_EQUAL(33333, config.port);
    }
    
    TEST(PortBoundaries) {
        // Проверяем различные значения портов
        
        // Временно подавляем stderr
        std::FILE* original_stderr = stderr;
        stderr = std::fopen("/dev/null", "w");
        
        char* argv1[] = {(char*)"program", (char*)"-p", (char*)"1", nullptr};
        ServerConfig config1 = ServerConfig::parse_args(3, argv1);
        CHECK_EQUAL(1, config1.port);
        
        char* argv2[] = {(char*)"program", (char*)"-p", (char*)"65535", nullptr};
        ServerConfig config2 = ServerConfig::parse_args(3, argv2);
        CHECK_EQUAL(65535, config2.port);
        
        char* argv3[] = {(char*)"program", (char*)"-p", (char*)"1024", nullptr};
        ServerConfig config3 = ServerConfig::parse_args(3, argv3);
        CHECK_EQUAL(1024, config3.port);
        
        // Восстанавливаем stderr
        std::fclose(stderr);
        stderr = original_stderr;
    }
    
    TEST(ConfigInterface) {
        // Тестируем интерфейс парсинга аргументов
        char* argv[] = {
            (char*)"vealc_server",
            (char*)"-p", (char*)"9999",
            (char*)"-c", (char*)"/tmp/test.conf",
            (char*)"-l", (char*)"/tmp/test.log",
            nullptr
        };
        
        ServerConfig config = ServerConfig::parse_args(7, argv);
        
        // Проверяем что все параметры установлены правильно
        CHECK_EQUAL(9999, config.port);
        CHECK_EQUAL("/tmp/test.conf", config.client_db_file);
        CHECK_EQUAL("/tmp/test.log", config.log_file);
    }
    
    TEST(ConfigInterfaceDefault) {
        // Тест интерфейса с минимальными аргументами
        char* argv[] = {(char*)"server", nullptr};
        ServerConfig config = ServerConfig::parse_args(1, argv);
        
        CHECK_EQUAL(33333, config.port);
        CHECK_EQUAL("/etc/vealc.conf", config.client_db_file);
        CHECK_EQUAL("/var/log/vealc.log", config.log_file);
    }
    
    TEST(ConfigInterfaceMixedOrder) {
        // Аргументы в разном порядке
        char* argv[] = {
            (char*)"server",
            (char*)"-l", (char*)"/var/log/myapp.log",
            (char*)"-p", (char*)"8080",
            (char*)"-c", (char*)"/etc/myapp.conf",
            nullptr
        };
        
        ServerConfig config = ServerConfig::parse_args(7, argv);
        
        CHECK_EQUAL(8080, config.port);
        CHECK_EQUAL("/etc/myapp.conf", config.client_db_file);
        CHECK_EQUAL("/var/log/myapp.log", config.log_file);
    }
}

int main() {
    // НЕ перенаправляем stdout/stderr здесь!
    // UnitTest++ сам управляет выводом
    
    return UnitTest::RunAllTests();
}
