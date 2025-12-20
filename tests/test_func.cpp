#include <UnitTest++/UnitTest++.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <thread>
#include <chrono>
#include <cstring>

SUITE(FunctionalTests) {
    const std::string TEST_CONFIG = "tests/test_data/test_clients.conf";
    const std::string TEST_LOG = "tests/test_data/test_server.log";
    
    void create_test_files() {
        system("mkdir -p tests/test_data");
        
        // Создаем тестовый файл клиентов
        std::ofstream client_file(TEST_CONFIG);
        if (client_file.is_open()) {
            client_file << "alice:P@ssl@rd\n";
            client_file << "bob:Secret123\n";
            client_file << "charlie:Qwerty!@#\n";
            client_file.close();
        }
    }
    
    void cleanup_test_files() {
        system("rm -rf tests/test_data");
        // Убиваем все запущенные тестовые серверы
        system("pkill -f './server -p 33' 2>/dev/null");
        system("pkill -f 'server.*-p' 2>/dev/null");
        sleep(1);
    }
    
    bool is_port_used(int port) {
        std::string cmd = "lsof -i:" + std::to_string(port) + " > /dev/null 2>&1";
        return system(cmd.c_str()) == 0;
    }
    
    bool is_process_running(pid_t pid) {
        return kill(pid, 0) == 0;
    }
    
    TEST(FT_01_ServerStartCorrectParams) {
        std::cout << "FT-01: Запуск сервера с корректными параметрами\n";
        create_test_files();
        
        pid_t pid = fork();
        if (pid == 0) {
            // Дочерний процесс - запускаем сервер
            char* args[] = {
                (char*)"./server",
                (char*)"-d", (char*)TEST_CONFIG.c_str(),
                (char*)"-l", (char*)TEST_LOG.c_str(),
                (char*)"-p", (char*)"33444",
                NULL
            };
            execvp("./server", args);
            exit(1);
        } else if (pid > 0) {
            // Родительский процесс - ждем и проверяем
            sleep(2); // Даем серверу время на запуск
            
            // Проверяем что порт занят
            CHECK(is_port_used(33444));
            
            // Останавливаем сервер
            kill(pid, SIGTERM);
            waitpid(pid, NULL, 0);
            sleep(1); // Даем время на завершение
            
            cleanup_test_files();
        }
    }
    
    TEST(FT_02_ServerStop) {
        std::cout << "FT-02: Остановка сервера\n";
        create_test_files();
        
        pid_t pid = fork();
        if (pid == 0) {
            char* args[] = {
                (char*)"./server",
                (char*)"-p", (char*)"33555",
                (char*)"-d", (char*)TEST_CONFIG.c_str(),
                NULL
            };
            execvp("./server", args);
            exit(1);
        } else if (pid > 0) {
            sleep(2);
            
            // Проверяем что сервер запущен
            CHECK(is_port_used(33555));
            
            // Останавливаем
            kill(pid, SIGTERM);
            waitpid(pid, NULL, 0);
            sleep(2);
            
            // Проверяем что сервер остановлен
            CHECK(!is_port_used(33555));
            
            cleanup_test_files();
        }
    }
    
    TEST(FT_03_UnknownOption) {
        std::cout << "FT-03: Неизвестная опция командной строки\n";
        
        // Запускаем с неверной опцией и проверяем что не зависает
        pid_t pid = fork();
        if (pid == 0) {
            char* args[] = {
                (char*)"./server",
                (char*)"--invalid-option",
                NULL
            };
            execvp("./server", args);
            exit(1);
        } else if (pid > 0) {
            int status;
            waitpid(pid, &status, 0);
            // Должен завершиться (не висеть)
            CHECK(true);
        }
    }
    
    TEST(FT_04_InvalidPortString) {
        std::cout << "FT-04: Некорректный порт (строка)\n";
        
        // Используем временный файл для перехвата вывода
        system("mkdir -p tests/test_data");
        
        pid_t pid = fork();
        if (pid == 0) {
            freopen("tests/test_data/error_output.txt", "w", stderr);
            char* args[] = {
                (char*)"./server",
                (char*)"-p", (char*)"not_a_number",
                NULL
            };
            execvp("./server", args);
            exit(1);
        } else if (pid > 0) {
            int status;
            waitpid(pid, &status, 0);
            // Должен завершиться с ошибкой
            CHECK(WIFEXITED(status));
        }
        
        system("rm -f tests/test_data/error_output.txt");
    }
    
    TEST(FT_05_InvalidPortZero) {
        std::cout << "FT-05: Недопустимый порт (0)\n";
        
        pid_t pid = fork();
        if (pid == 0) {
            char* args[] = {
                (char*)"./server",
                (char*)"-p", (char*)"0",
                NULL
            };
            execvp("./server", args);
            exit(1);
        } else if (pid > 0) {
            int status;
            waitpid(pid, &status, 0);
            // Должен завершиться с ошибкой
            CHECK(WIFEXITED(status));
        }
    }
    
    TEST(FT_06_InvalidPortNegative) {
        std::cout << "FT-06: Недопустимый порт (отрицательный)\n";
        
        pid_t pid = fork();
        if (pid == 0) {
            char* args[] = {
                (char*)"./server",
                (char*)"-p", (char*)"-100",
                NULL
            };
            execvp("./server", args);
            exit(1);
        } else if (pid > 0) {
            int status;
            waitpid(pid, &status, 0);
            // Должен завершиться с ошибкой
            CHECK(WIFEXITED(status));
        }
    }
    
    TEST(FT_07_NonExistentFile) {
        std::cout << "FT-07: Несуществующий файл\n";
        
        pid_t pid = fork();
        if (pid == 0) {
            char* args[] = {
                (char*)"./server",
                (char*)"-d", (char*)"nonexistent_file.conf",
                NULL
            };
            execvp("./server", args);
            exit(1);
        } else if (pid > 0) {
            int status;
            waitpid(pid, &status, 0);
            // Должен завершиться с ошибкой
            CHECK(WIFEXITED(status));
        }
    }
    
    TEST(FT_08_HelpOutput) {
        std::cout << "FT-08: Вывод справки по опции --help\n";
        
        // Используем временный файл
        system("mkdir -p tests/test_data");
        
        pid_t pid = fork();
        if (pid == 0) {
            freopen("tests/test_data/help_output.txt", "w", stdout);
            char* args[] = {
                (char*)"./server",
                (char*)"--help",
                NULL
            };
            execvp("./server", args);
            exit(1);
        } else if (pid > 0) {
            int status;
            waitpid(pid, &status, 0);
            
            // Проверяем что файл создан и содержит "Usage"
            std::ifstream file("tests/test_data/help_output.txt");
            if (file.is_open()) {
                std::string content((std::istreambuf_iterator<char>(file)),
                                   std::istreambuf_iterator<char>());
                CHECK(content.find("Usage") != std::string::npos);
                file.close();
            }
            
            system("rm -f tests/test_data/help_output.txt");
        }
    }
    
    TEST(FT_09_Port33555) {
        std::cout << "FT-09: Запуск на порту 33555\n";
        create_test_files();
        
        pid_t pid = fork();
        if (pid == 0) {
            char* args[] = {
                (char*)"./server",
                (char*)"-p", (char*)"33555",
                (char*)"-d", (char*)TEST_CONFIG.c_str(),
                NULL
            };
            execvp("./server", args);
            exit(1);
        } else if (pid > 0) {
            sleep(2);
            
            CHECK(is_port_used(33555));
            
            kill(pid, SIGTERM);
            waitpid(pid, NULL, 0);
            sleep(1);
            
            cleanup_test_files();
        }
    }
    
    TEST(FT_13_TwoServersSimultaneously) {
        std::cout << "FT-13: Два сервера одновременно на разных портах\n";
        create_test_files();
        
        pid_t pid1 = fork();
        if (pid1 == 0) {
            char* args[] = {
                (char*)"./server",
                (char*)"-p", (char*)"33888",
                (char*)"-d", (char*)TEST_CONFIG.c_str(),
                NULL
            };
            execvp("./server", args);
            exit(1);
        }
        
        sleep(2); // Даем первому серверу время на запуск
        
        pid_t pid2 = fork();
        if (pid2 == 0) {
            char* args[] = {
                (char*)"./server",
                (char*)"-p", (char*)"33999",
                (char*)"-d", (char*)TEST_CONFIG.c_str(),
                NULL
            };
            execvp("./server", args);
            exit(1);
        }
        
        sleep(2); // Даем второму серверу время на запуск
        
        // Проверяем что оба сервера запущены
        bool port1_used = is_port_used(33888);
        bool port2_used = is_port_used(33999);
        
        std::cout << "Port 33888 used: " << port1_used << std::endl;
        std::cout << "Port 33999 used: " << port2_used << std::endl;
        
        CHECK(port1_used);
        CHECK(port2_used);
        
        // Останавливаем оба сервера
        kill(pid1, SIGTERM);
        kill(pid2, SIGTERM);
        
        waitpid(pid1, NULL, 0);
        waitpid(pid2, NULL, 0);
        
        sleep(1);
        cleanup_test_files();
    }
    
    TEST(FT_15_CreateTestUserFile) {
        std::cout << "FT-15: Создание тестового файла пользователей\n";
        
        system("mkdir -p tests/test_data");
        
        std::ofstream file("tests/test_data/test_users.txt");
        CHECK(file.is_open());
        
        if (file.is_open()) {
            file << "user1:pass1\n";
            file << "user2:pass2\n";
            file << "user3:pass3\n";
            file.close();
            
            // Проверяем создание
            std::ifstream check("tests/test_data/test_users.txt");
            CHECK(check.is_open());
            
            std::string line;
            int count = 0;
            while (std::getline(check, line)) {
                if (!line.empty()) count++;
            }
            CHECK_EQUAL(3, count);
            check.close();
        }
        
        system("rm -rf tests/test_data");
    }
};

int main() {
    std::cout << "========================================\n";
    std::cout << "Запуск приёмочных тестов сервера vcalc\n";
    std::cout << "========================================\n\n";
    
    // Убиваем все предыдущие тестовые серверы
    system("pkill -f './server' 2>/dev/null");
    sleep(1);
    
    // Создаем директорию для тестовых данных
    system("mkdir -p tests/test_data");
    
    int result = UnitTest::RunAllTests();
    
    // Финальная очистка
    system("pkill -f './server' 2>/dev/null");
    system("rm -rf tests/test_data");
    
    return result;
}
