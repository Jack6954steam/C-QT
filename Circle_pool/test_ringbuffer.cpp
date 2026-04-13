/**
 * @file test_ringbuffer.cpp
 * @brief 环形缓冲区独立测试程序
 * 
 * 本文件演示如何独立使用环形缓冲区库，不依赖Qt界面
 * 可以作为静态库的使用示例
 */

#include "RingBuffer.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>

using namespace std;

// 打印分隔线
void printSeparator() {
    cout << string(60, '=') << endl;
}

// 打印缓冲区状态
void printBufferStatus(const RingBuffer* rb) {
    cout << "缓冲区状态:" << endl;
    cout << "  总大小: " << RingBuffer_GetSize(rb) << " 字节" << endl;
    cout << "  已存储: " << RingBuffer_GetCount(rb) << " 字节" << endl;
    cout << "  可用空间: " << RingBuffer_GetFreeSpace(rb) << " 字节" << endl;
    cout << "  写指针(Head): " << RingBuffer_GetHead(rb) << endl;
    cout << "  读指针(Tail): " << RingBuffer_GetTail(rb) << endl;
    cout << "  是否为空: " << (RingBuffer_IsEmpty(rb) ? "是" : "否") << endl;
    cout << "  是否已满: " << (RingBuffer_IsFull(rb) ? "是" : "否") << endl;
}

// 打印缓冲区内容
void printBufferContent(const RingBuffer* rb) {
    uint32_t count = RingBuffer_GetCount(rb);
    cout << "缓冲区内容 (" << count << " 字节):" << endl;
    cout << "  ";
    
    for (uint32_t i = 0; i < count; i++) {
        uint8_t data;
        if (RingBuffer_Peek(rb, i, &data)) {
            cout << setw(3) << (int)data;
            if (i < count - 1) cout << ", ";
            if ((i + 1) % 10 == 0) cout << endl << "  ";
        }
    }
    cout << endl;
}

// 测试1：基本功能测试
void testBasicOperations() {
    printSeparator();
    cout << "测试1：基本功能测试" << endl;
    printSeparator();
    
    // 创建缓冲区
    RingBuffer* rb = RingBuffer_Init(10);
    cout << "创建缓冲区（10字节）" << endl;
    printBufferStatus(rb);
    cout << endl;
    
    // 写入数据
    cout << "写入数据: 1, 2, 3, 4, 5" << endl;
    for (int i = 1; i <= 5; i++) {
        RingBuffer_Push(rb, i);
    }
    printBufferStatus(rb);
    printBufferContent(rb);
    cout << endl;
    
    // 读取数据
    cout << "读取3个字节:" << endl;
    uint8_t data;
    for (int i = 0; i < 3; i++) {
        if (RingBuffer_Pop(rb, &data)) {
            cout << "  读取: " << (int)data << endl;
        }
    }
    printBufferStatus(rb);
    printBufferContent(rb);
    cout << endl;
    
    // 继续写入（测试环绕）
    cout << "继续写入数据: 10, 20, 30, 40, 50, 60, 70, 80" << endl;
    uint8_t writeData[] = {10, 20, 30, 40, 50, 60, 70, 80};
    uint32_t written = RingBuffer_PushBatch(rb, writeData, 8);
    cout << "实际写入: " << written << " 字节" << endl;
    printBufferStatus(rb);
    printBufferContent(rb);
    cout << endl;
    
    // 清空缓冲区
    cout << "清空缓冲区" << endl;
    RingBuffer_Clear(rb);
    printBufferStatus(rb);
    cout << endl;
    
    // 释放资源
    RingBuffer_Destroy(rb);
    cout << "测试1完成" << endl << endl;
}

// 测试2：满和空的状态测试
void testFullAndEmpty() {
    printSeparator();
    cout << "测试2：满和空的状态测试" << endl;
    printSeparator();
    
    RingBuffer* rb = RingBuffer_Init(5);
    cout << "创建缓冲区（5字节）" << endl << endl;
    
    // 填满缓冲区
    cout << "尝试填满缓冲区" << endl;
    for (int i = 1; i <= 10; i++) {
        bool success = RingBuffer_Push(rb, i);
        cout << "写入 " << i << ": " << (success ? "成功" : "失败（缓冲区已满）") << endl;
    }
    cout << endl;
    printBufferStatus(rb);
    cout << endl;
    
    // 清空缓冲区
    cout << "清空缓冲区" << endl;
    uint8_t data;
    while (!RingBuffer_IsEmpty(rb)) {
        RingBuffer_Pop(rb, &data);
        cout << "读取: " << (int)data << endl;
    }
    cout << endl;
    
    // 尝试从空缓冲区读取
    cout << "尝试从空缓冲区读取:" << endl;
    bool success = RingBuffer_Pop(rb, &data);
    cout << "读取结果: " << (success ? "成功" : "失败（缓冲区为空）") << endl;
    cout << endl;
    
    printBufferStatus(rb);
    cout << endl;
    
    RingBuffer_Destroy(rb);
    cout << "测试2完成" << endl << endl;
}

// 测试3：批量操作测试
void testBatchOperations() {
    printSeparator();
    cout << "测试3：批量操作测试" << endl;
    printSeparator();
    
    RingBuffer* rb = RingBuffer_Init(20);
    cout << "创建缓冲区（20字节）" << endl << endl;
    
    // 批量写入
    uint8_t writeData[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    cout << "批量写入10个字节" << endl;
    uint32_t written = RingBuffer_PushBatch(rb, writeData, 10);
    cout << "实际写入: " << written << " 字节" << endl;
    printBufferContent(rb);
    cout << endl;
    
    // 批量读取
    uint8_t readData[5];
    cout << "批量读取5个字节" << endl;
    uint32_t read_count = RingBuffer_PopBatch(rb, readData, 5);
    cout << "实际读取: " << read_count << " 字节" << endl;
    cout << "读取内容: ";
    for (uint32_t i = 0; i < read_count; i++) {
        cout << (int)readData[i] << " ";
    }
    cout << endl << endl;
    
    printBufferStatus(rb);
    cout << endl;
    
    RingBuffer_Destroy(rb);
    cout << "测试3完成" << endl << endl;
}

// 测试4：环绕测试
void testWrapAround() {
    printSeparator();
    cout << "测试4：环绕测试" << endl;
    printSeparator();
    
    RingBuffer* rb = RingBuffer_Init(8);
    cout << "创建缓冲区（8字节）" << endl << endl;
    
    // 填充一部分
    cout << "写入数据: 1-5" << endl;
    for (int i = 1; i <= 5; i++) {
        RingBuffer_Push(rb, i);
    }
    printBufferStatus(rb);
    cout << endl;
    
    // 读取一部分
    cout << "读取3个字节" << endl;
    uint8_t data;
    for (int i = 0; i < 3; i++) {
        RingBuffer_Pop(rb, &data);
        cout << "读取: " << (int)data << endl;
    }
    printBufferStatus(rb);
    cout << endl;
    
    // 继续写入（触发环绕）
    cout << "继续写入: 10-15" << endl;
    for (int i = 10; i <= 15; i++) {
        bool success = RingBuffer_Push(rb, i);
        cout << "写入 " << i << ": " << (success ? "成功" : "失败") << endl;
    }
    printBufferStatus(rb);
    printBufferContent(rb);
    cout << endl;
    
    // 读取所有数据
    cout << "读取所有数据:" << endl;
    while (!RingBuffer_IsEmpty(rb)) {
        RingBuffer_Pop(rb, &data);
        cout << (int)data << " ";
    }
    cout << endl << endl;
    
    RingBuffer_Destroy(rb);
    cout << "测试4完成" << endl << endl;
}

// 测试5：C++封装类测试（线程安全）
void testCppClass() {
    printSeparator();
    cout << "测试5：C++封装类测试（线程安全）" << endl;
    printSeparator();
    
    RingBufferCPP rb(15);
    cout << "创建缓冲区（15字节）" << endl << endl;
    
    // 写入数据
    cout << "写入数据: 'H', 'e', 'l', 'l', 'o'" << endl;
    rb.push('H');
    rb.push('e');
    rb.push('l');
    rb.push('l');
    rb.push('o');
    
    cout << "数据量: " << rb.getCount() << endl;
    cout << "写指针: " << rb.getHead() << endl;
    cout << "读指针: " << rb.getTail() << endl;
    cout << endl;
    
    // 读取数据
    cout << "读取数据:" << endl;
    uint8_t data;
    while (!rb.isEmpty()) {
        rb.pop(&data);
        cout << (char)data;
    }
    cout << endl << endl;
    
    cout << "测试5完成" << endl << endl;
}

// 测试6：多线程测试（生产者-消费者）
void testMultiThread() {
    printSeparator();
    cout << "测试6：多线程测试（生产者-消费者）" << endl;
    printSeparator();
    
    const int bufferSize = 100;
    const int itemCount = 50;
    RingBufferCPP rb(bufferSize);
    
    cout << "缓冲区大小: " << bufferSize << " 字节" << endl;
    cout << "生产数量: " << itemCount << " 字节" << endl << endl;
    
    auto start = chrono::high_resolution_clock::now();
    
    // 生产者线程
    thread producer([&rb, itemCount]() {
        for (int i = 0; i < itemCount; i++) {
            rb.push(i % 256);
            this_thread::sleep_for(chrono::milliseconds(10));
        }
    });
    
    // 消费者线程
    thread consumer([&rb, itemCount]() {
        int consumed = 0;
        uint8_t data;
        while (consumed < itemCount) {
            if (rb.pop(&data)) {
                consumed++;
            }
            this_thread::sleep_for(chrono::milliseconds(15));
        }
    });
    
    producer.join();
    consumer.join();
    
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
    
    cout << "多线程测试完成" << endl;
    cout << "耗时: " << duration.count() << " 毫秒" << endl;
    cout << "最终缓冲区状态: " << rb.getCount() << " 字节" << endl << endl;
}

// 测试7：性能测试
void testPerformance() {
    printSeparator();
    cout << "测试7：性能测试" << endl;
    printSeparator();
    
    const int bufferSize = 10000;
    const int iterations = 100000;
    
    RingBuffer* rb = RingBuffer_Init(bufferSize);
    cout << "缓冲区大小: " << bufferSize << " 字节" << endl;
    cout << "操作次数: " << iterations << " 次" << endl << endl;
    
    // 写入性能测试
    auto start = chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; i++) {
        RingBuffer_Push(rb, i % 256);
        if (RingBuffer_IsFull(rb)) {
            uint8_t data;
            RingBuffer_Pop(rb, &data);
        }
    }
    auto end = chrono::high_resolution_clock::now();
    auto writeDuration = chrono::duration_cast<chrono::microseconds>(end - start);
    
    cout << "写入性能:" << endl;
    cout << "  总耗时: " << writeDuration.count() << " 微秒" << endl;
    cout << "  平均每次: " << (double)writeDuration.count() / iterations << " 微秒" << endl;
    cout << "  吞吐量: " << (double)iterations * 1000000 / writeDuration.count() << " 次/秒" << endl;
    cout << endl;
    
    // 读取性能测试
    RingBuffer_Clear(rb);
    for (int i = 0; i < bufferSize; i++) {
        RingBuffer_Push(rb, i % 256);
    }
    
    start = chrono::high_resolution_clock::now();
    uint8_t data;
    for (int i = 0; i < bufferSize; i++) {
        RingBuffer_Pop(rb, &data);
    }
    end = chrono::high_resolution_clock::now();
    auto readDuration = chrono::duration_cast<chrono::microseconds>(end - start);
    
    cout << "读取性能:" << endl;
    cout << "  总耗时: " << readDuration.count() << " 微秒" << endl;
    cout << "  平均每次: " << (double)readDuration.count() / bufferSize << " 微秒" << endl;
    cout << "  吞吐量: " << (double)bufferSize * 1000000 / readDuration.count() << " 次/秒" << endl;
    cout << endl;
    
    RingBuffer_Destroy(rb);
    cout << "测试7完成" << endl << endl;
}

int main() {
    cout << "环形缓冲区静态库测试程序" << endl;
    cout << "================================" << endl << endl;
    
    // 运行所有测试
    testBasicOperations();
    testFullAndEmpty();
    testBatchOperations();
    testWrapAround();
    testCppClass();
    testMultiThread();
    testPerformance();
    
    printSeparator();
    cout << "所有测试完成！" << endl;
    printSeparator();
    
    return 0;
}
