#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <cstdio>
#include <vector>
#include "windows.h"
#include <random>

using namespace std;

static string firstComp = "табак";
static string secondComp = "бумага";
static string thirdComp = "спички";

random_device generator;
uniform_int_distribution<int> UID(0, 2);


/**
 * Класс семафора
 */
class Semaphore {

private:

    HANDLE event;
    unsigned int maxThread, freeThread; //  максимальное количество потоков, количество свободных потоков

public:

    explicit Semaphore(unsigned int size) : // создание семафора относительно изначального значения
            maxThread(size),
            freeThread(size) {
        event = CreateEvent(NULL, TRUE, TRUE, NULL); // manual reset, signal state
    }

    ~Semaphore() { // Деструктор
        CloseHandle(event); // Уничтожение события
    }

    bool WaitForSemaphore(DWORD ms) { // аналог метода WaitForSingleObject из библиотеки windows.h

        while (TRUE) {

            if (WaitForSingleObject(event, ms) == WAIT_OBJECT_0 &&
                freeThread > 0) {

                if (freeThread > 0) {
                    freeThread--;
                    if (freeThread == 0)
                        ResetEvent(event);
                    return (FALSE);
                }
            }
        }


    }

    void LeaveSemaphore() { // аналог ReleaseSemaphore из windows.h

        if (freeThread == 0)SetEvent(event);
        freeThread++;

    }

};

/**
 * Класс объект стола
 */
class Table {

private:
    bool containSecondComp = false;
    bool containFirstComp = false;
    bool containThirdComp = false;

public:
    Table() = default;

    ~Table() = default;

    /**
     * проверяет есть ли на столе компоненты относительно указаного
     * для создания сигареты
     * @param treadComp компонент который есть у курильщика
     * @return true, если можно сделать сиграету, иначе false
     */
    [[nodiscard]] bool checkTable(const string& treadComp) const {
        if (treadComp == firstComp) {
            if (containSecondComp && containThirdComp) {
                return true;
            }
        } else if (treadComp == secondComp) {
            if (containThirdComp && containFirstComp) {
                return true;
            }
        } else {
            if (containSecondComp && containFirstComp) {
                return true;
            }
        }

        return false;
    }

    /**
     * Возвращает компоненты со стола в виде строкогого представления
     * Также удаляет забранные компоненты у объекта стол
     * @return строковое представления компонентов
     */
    string getComponents() {
        string result;
        if (containFirstComp) {
            setFirstComp(false);
            result += firstComp + " ";
        }

        if (containSecondComp) {
            setSecondComp(false);
            result += secondComp + " ";
        }

        if (containThirdComp) {
            setThirdComp(false);
            result += thirdComp + " ";
        }

        return result;
    }

    void setFirstComp(bool value) {
        containFirstComp = value;
    }

    void setSecondComp(bool value) {
        containSecondComp = value;
    }

    void setThirdComp(bool value) {
        containThirdComp = value;
    }

};


Semaphore *s; // Семафор
Table *table; // Стол
LONG initValue = 0; // Начальное значение симафора



/**
 * Функция определяющее события курения сигареты курильщиком
 * @param id номер курильщика
 * @param nameThread имя курильщика
 * @param threadComp компонент у курильщика
 */
[[noreturn]] void smokerFunction(int id, string nameThread, string threadComp) {


    while (true) {

        // Проверка есть ли пододящие компоненты, еоторые нужны курильщику
        // Если их нет то курильщик их ожидает
        while (!table->checkTable(threadComp)) {}

        // Если есть курильщик их забирает и начинает процесс курения сигареты
        cout << "Курильщик-" << id << "(" << nameThread << ") забрал(-a) " << table->getComponents()
             << "cо стола и сделал(-а) сигарету и начал(-а) курить" << endl;
        cout << "Курильщик-" << id << "(" << nameThread << ") докурил(-a) сиграету, крикнул(-a) и начал(-a) ждать" << endl;
        // Как только курильщик докуривает сигарету, он увеличивает семафор на 1,
        // сообщая об окончании процесса
        s->LeaveSemaphore();
    }
}


int main([[maybe_unused]] int argc, char *argv[]) {
    // Установка локализации
    setlocale(LC_ALL, "Russian");

    // количество операций курения сигарет
    int countOperations = stoi(argv[1]);
    try {
        if (countOperations < 1 || countOperations > 1000) {
            throw exception(
                    "wrong format! Count of operations can be [1;1000]");
        }

        // Переопределение консольного вывода в файл
        fstream file;
        file.open(argv[2], ios::out);
        streambuf *stream_buffer_file = file.rdbuf();
        cout.rdbuf(stream_buffer_file);


        // Созддание семаформа с начальным значением
        s = new Semaphore(initValue);
        // Создание стола
        table = new Table();

        // Созддание 3 потоков курильщиков
        auto *first = new thread(smokerFunction, 1, "Саня", firstComp);
        auto *second = new thread(smokerFunction, 2, "Федор", secondComp);
        auto *third = new thread(smokerFunction, 3, "Кристина", thirdComp);


        cout << "\"И так давайте покурим!\",- говорят курильщики." << endl;
        for (int i = 0; i < countOperations; ++i) {

            // Выбор сулчаных 2 компонентов
            int key = UID(generator);
            if (key == 0) {
                cout << "Посредник положил на стол бумагу и спички и решил чуть чуть вздремнуть..." << endl;
                // кладем на стол компоненты бумага и спички
                table->setSecondComp(true);
                table->setThirdComp(true);
            } else if (key == 1) {
                cout << "Посредник положил на стол табак и спички и решил чуть чуть вздремнуть..." << endl;
                // Кладем на стол компоненты табак и спички
                table->setFirstComp(true);
                table->setThirdComp(true);
            } else {
                cout << "Посредник положил на стол табак и бумагу и решил чуть чуть вздремнуть..." << endl;
                // Кладем на стол компоненты бумага и табак
                table->setSecondComp(true);
                table->setFirstComp(true);
            }

            // Ждем когда курильщик докурит сигарету
            s->WaitForSemaphore(INFINITE);
            cout << "Посредник вздрогнул и быстро сунул руки в карманы" << endl;
        }
        // Сообщение об окончании всего процесса
        cout << "У посредника кончились компоненты для сигарет и он ушел в магазин..." << endl;
        // Уничтожение курильщиков
        first->detach();
        second->detach();
        third->detach();
        // Уничтожение семафора
        s->~Semaphore();
    }
    catch (exception ex){
        cout<<ex.what()<<endl;
        cout<<"example command string:\"{countOperations} {pathToOutputFile}\"" << endl;
    }
    return 0;
}

