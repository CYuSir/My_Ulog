#include <iostream>
#include <string>
#include <thread>
#include <ulog_cpp/zz_data_log.hpp>
#include <vector>

#include "escape_structs.hpp"

using namespace std;

bool ZzDataLogOn = true;
extern std::vector<DataVariant> all_structs;

void PrintStruct(const MyData1& s) {
    printf("%s %d %s %ld\n", __func__, __LINE__, s.messageName().c_str(), s.fields().size());
    printf("timestamp: %ld\n", s.timestamp);
    printf("cpuload: %f\n", s.cpuload);
    printf("temperature: %f\n", s.temperature);
    printf("counter: %d\n", s.counter);
    printf("debug_array: ");
    for (int i = 0; i < 4; ++i) {
        printf("%f ", s.debug_array[i]);
    }
}

void threadFunc1(int threadId) {
    std::cout << "Thread " << threadId << " is running." << std::endl;
    printf("MyData1 size: %ld\n", sizeof(MyData1));
    for (int K = 0; K < 10; K++) {
        float cpuload = 25.423F;
        for (int i = 0; i < 10; ++i) {
            MyData1 data{};
            data.timestamp = currentTimeUs();
            data.cpuload = cpuload;
            data.counter = i;
            data.temperature = i + 0.423F;
            for (int j = 0; j < 4; ++j) {
                data.debug_array[j] = rand();
            }
            for (int j = 0; j < 6; ++j) {
                data.array[j] = i + j + 10;
            }
            PrintStruct(data);
            zz_data_log::GetInstance()->Write(data);
            printf("%s %d\n", __func__, __LINE__);

            cpuload -= 1.424F;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    cout << "Thread " << threadId << " is done." << endl;
}

void threadFunc2(int threadId) {
    std::cout << "Thread " << threadId << " is running." << std::endl;
    printf("MyData2 size: %ld\n", sizeof(MyData2));
    for (int K = 0; K < 10; K++) {
        float cpuload2 = 50.846F;
        for (int i = 0; i < 10; ++i) {
            MyData2 data{};
            data.timestamp = currentTimeUs();
            data.cpuload = cpuload2;
            data.counter = i;
            for (int j = 0; j < 4; ++j) {
                data.debug_array[j] = i + j + 3;
            }

            zz_data_log::GetInstance()->Write(data);
            printf("%s %d\n", __func__, __LINE__);

            cpuload2 -= 1.848F;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    cout << "Thread " << threadId << " is done." << endl;
}

void threadFunc3(int threadId) {
    std::cout << "Thread " << threadId << " is running." << std::endl;
    printf("MyData3 size: %ld\n", sizeof(MyData3));
    for (int K = 0; K < 10; K++) {
        float cpuload2 = 50.846F;
        for (int i = 0; i < 10; ++i) {
            MyData3 data{};
            data.timestamp = currentTimeUs();
            data.cpuload = cpuload2;
            data.counter = i;
            for (int j = 0; j < 4; ++j) {
                data.debug_array[j] = i + j + 6;
            }

            zz_data_log::GetInstance()->Write(data);
            printf("%s %d\n", __func__, __LINE__);

            cpuload2 -= 2.848F;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    cout << "Thread " << threadId << " is done." << endl;
}

int main(int argc, char** argv) {
    zz_data_log::CreateInstance("test.ulg");
    zz_data_log::GetInstance()->Init<DataVariant>("test", "testUlogWriter", all_structs);
    std::thread thread1(threadFunc1, 1);
    // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::thread thread2(threadFunc2, 2);
    // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::thread thread3(threadFunc3, 3);
    // while (true) {
    //     std::this_thread::sleep_for(std::chrono::milliseconds(100));
    // }
    zz_data_log::GetInstance()->fsync();
    thread1.join();
    thread2.join();
    thread3.join();
    printf("MyData1 size: %ld\n", sizeof(MyData1));
    printf("MyData2 size: %ld\n", sizeof(MyData2));
    printf("MyData3 size: %ld\n", sizeof(MyData3));
    return 0;
}