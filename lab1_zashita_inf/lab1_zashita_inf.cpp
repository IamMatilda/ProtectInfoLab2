#include <windows.h>
#include <wincrypt.h>
#include <stdio.h>
#include <stdlib.h>
#include <direct.h>  

#define DATA_SIZE 40
#define CHECKSUM_SIZE 2

// Функция для вычисления контрольной суммы
unsigned short calculateChecksum(unsigned char* data, size_t size) {
    unsigned short checksum = 0;
    for (size_t i = 0; i < size; i++) {
        checksum += data[i];
    }
    return checksum;
}

// Функция для генерации случайного массива данных и записи в файл
void generateRandomDataAndSave(HCRYPTPROV hCryptProv, const char* filePath, int fileCount) {
    for (int i = 0; i < fileCount; i++) {
        // Генерация случайных данных
        unsigned char data[DATA_SIZE] = { 0 };  // Инициализация массива нулями
        if (!CryptGenRandom(hCryptProv, DATA_SIZE, data)) {
            printf("Error generating random data.\n");
            continue;  // Переход к следующему файлу
        }

        // Вычисление контрольной суммы
        unsigned short checksum = calculateChecksum(data, DATA_SIZE);

        // Открытие файла для записи
        char fileName[260];
        snprintf(fileName, sizeof(fileName), "%s/file_%d.bin", filePath, i + 1);
        FILE* file;
        if (fopen_s(&file, fileName, "wb") != 0) {
            printf("Error opening file: %s\n", fileName);
            continue;  // Переход к следующему файлу
        }

        // Запись данных и контрольной суммы в файл
        if (fwrite(data, 1, DATA_SIZE, file) != DATA_SIZE || fwrite(&checksum, 1, CHECKSUM_SIZE, file) != CHECKSUM_SIZE) {
            printf("Error writing to file: %s\n", fileName);
        }
        else {
            printf("File %s created successfully.\n", fileName);
        }

        // Закрытие файла
        fclose(file);
    }
}

int main() {
    HCRYPTPROV hCryptProv = NULL;
    const char* userContainer = "MyKeyContainer";
    const char* filePath = "C:\\random";  // Путь для сохранения файлов
    int fileCount = 10;  // Количество файлов

    // Проверка существования директории и создание, если не существует
    if (_mkdir(filePath) != 0 && errno != EEXIST) {
        printf("Error creating directory: %s\n", filePath);
        return 1;
    }

    // Инициализация криптопровайдера
    if (!CryptAcquireContext(&hCryptProv, userContainer, NULL, PROV_RSA_FULL, 0)) {
        if (GetLastError() == NTE_BAD_KEYSET) {
            if (!CryptAcquireContext(&hCryptProv, userContainer, NULL, PROV_RSA_FULL, CRYPT_NEWKEYSET)) {
                printf("Error creating a new key container.\n");
                return 1;
            }
        }
        else {
            printf("Error acquiring cryptographic context.\n");
            return 1;
        }
    }

    printf("Cryptographic context acquired successfully.\n");

    // Генерация данных и запись в файлы
    generateRandomDataAndSave(hCryptProv, filePath, fileCount);

    // Освобождение дескриптора криптопровайдера
    if (!CryptReleaseContext(hCryptProv, 0)) {
        printf("Error releasing cryptographic context.\n");
        return 1;
    }

    printf("Cryptographic context released successfully.\n");

    return 0;
}