# 高性能环形缓冲区静态库

## 项目简介

本项目实现了一个高性能的环形缓冲区（Ring Buffer / Circular Buffer）静态库，并提供了Qt5可视化界面来展示缓冲区的运行状态。

## 环形缓冲区原理

环形缓冲区是一种先进先出（FIFO）的数据结构，使用一块固定大小的内存空间，通过读写指针的循环移动来实现数据的存取。

### 核心概念

- **写指针（Head）**：指向下一个数据写入的位置
- **读指针（Tail）**：指向下一个数据读取的位置
- **缓冲区状态**：
  - 空：`head == tail` 且 `is_full == false`
  - 满：`head == tail` 且 `is_full == true`
  - 非空非满：`head != tail`

### 物理内存映射

```
初始状态（空）：
+---+---+---+---+---+---+---+---+
|   |   |   |   |   |   |   |   |
+---+---+---+---+---+---+---+---+
  ^
  H/T (head == tail)

写入数据后：
+---+---+---+---+---+---+---+---+
| A | B | C |   |   |   |   |   |
+---+---+---+---+---+---+---+---+
  ^           ^
  T           H

读取数据后：
+---+---+---+---+---+---+---+---+
|   |   | C |   |   |   |   |   |
+---+---+---+---+---+---+---+---+
          ^   ^
          T   H

继续写入（环绕）：
+---+---+---+---+---+---+---+---+
| X |   | C | D | E | F | G | H |
+---+---+---+---+---+---+---+---+
  ^   ^
  H   T
```

## 项目结构

```
Circle_pool/
├── RingBuffer.h          # 环形缓冲区头文件（接口定义）
├── RingBuffer.cpp        # 环形缓冲区实现
├── Circle_pool.h         # Qt可视化界面头文件
├── Circle_pool.cpp       # Qt可视化界面实现
├── Circle_pool.ui        # Qt界面设计文件
├── main.cpp              # 主程序入口
└── README.md             # 说明文档
```

## API接口

### C接口（C风格API）

#### 结构体定义

```cpp
struct RingBuffer {
    uint8_t* buffer;    // 缓冲区空间指针
    uint32_t size;      // 缓冲区总大小
    uint32_t head;      // 写指针：指向下一个写入位置
    uint32_t tail;      // 读指针：指向下一个读取位置
    bool is_full;       // 缓冲区是否已满标志
};
```

#### 函数接口

| 函数名 | 功能 | 参数 | 返回值 |
|--------|------|------|--------|
| `RingBuffer_Init` | 初始化：分配内存并重置读写指针 | `size`: 缓冲区大小 | RingBuffer指针 |
| `RingBuffer_Destroy` | 释放环形缓冲区 | `rb`: 环形缓冲区指针 | void |
| `RingBuffer_Push` | 入队：向缓冲区写入一个字节 | `rb`, `data` | 成功返回true，满返回false |
| `RingBuffer_Pop` | 出队：从缓冲区读取一个字节 | `rb`, `p_data` | 成功返回true，空返回false |
| `RingBuffer_PushBatch` | 批量写入数据 | `rb`, `data`, `length` | 实际写入的字节数 |
| `RingBuffer_PopBatch` | 批量读取数据 | `rb`, `data`, `length` | 实际读取的字节数 |
| `RingBuffer_GetCount` | 获取已存储的数据长度 | `rb` | 数据长度 |
| `RingBuffer_GetFreeSpace` | 获取可用空间 | `rb` | 可用空间大小 |
| `RingBuffer_IsEmpty` | 判断是否为空 | `rb` | 为空返回true |
| `RingBuffer_IsFull` | 判断是否已满 | `rb` | 已满返回true |
| `RingBuffer_Clear` | 清空缓冲区（逻辑重置指针） | `rb` | void |
| `RingBuffer_GetSize` | 获取缓冲区总大小 | `rb` | 缓冲区大小 |
| `RingBuffer_GetTail` | 获取读指针位置 | `rb` | 读指针位置 |
| `RingBuffer_GetHead` | 获取写指针位置 | `rb` | 写指针位置 |
| `RingBuffer_Peek` | 查看指定位置的数据（不移除） | `rb`, `offset`, `p_data` | 成功返回true |

### C++封装类（线程安全）

```cpp
class RingBufferCPP {
public:
    explicit RingBufferCPP(uint32_t size);
    ~RingBufferCPP();
    
    bool push(uint8_t data);
    bool pop(uint8_t* p_data);
    uint32_t pushBatch(const uint8_t* data, uint32_t length);
    uint32_t popBatch(uint8_t* data, uint32_t length);
    uint32_t getCount() const;
    uint32_t getFreeSpace() const;
    bool isEmpty() const;
    bool isFull() const;
    void clear();
    uint32_t getSize() const;
    uint32_t getTail() const;
    uint32_t getHead() const;
    bool peek(uint32_t offset, uint8_t* p_data) const;
};
```

## 使用示例

### C接口使用示例

```cpp
#include "RingBuffer.h"
#include <iostream>

int main() {
    // 1. 初始化缓冲区（16字节）
    RingBuffer* rb = RingBuffer_Init(16);
    
    // 2. 写入数据
    uint8_t data[] = {65, 66, 67, 68, 69}; // A, B, C, D, E
    for (int i = 0; i < 5; i++) {
        if (RingBuffer_Push(rb, data[i])) {
            std::cout << "写入: " << (char)data[i] << std::endl;
        }
    }
    
    // 3. 查看状态
    std::cout << "数据量: " << RingBuffer_GetCount(rb) << std::endl;
    std::cout << "可用空间: " << RingBuffer_GetFreeSpace(rb) << std::endl;
    
    // 4. 读取数据
    uint8_t value;
    while (!RingBuffer_IsEmpty(rb)) {
        if (RingBuffer_Pop(rb, &value)) {
            std::cout << "读取: " << (char)value << std::endl;
        }
    }
    
    // 5. 清空缓冲区
    RingBuffer_Clear(rb);
    
    // 6. 释放资源
    RingBuffer_Destroy(rb);
    
    return 0;
}
```

### C++类使用示例

```cpp
#include "RingBuffer.h"
#include <iostream>

int main() {
    // 1. 创建缓冲区（线程安全）
    RingBufferCPP rb(32);
    
    // 2. 写入数据
    rb.push('H');
    rb.push('e');
    rb.push('l');
    rb.push('l');
    rb.push('o');
    
    // 3. 查看状态
    std::cout << "数据量: " << rb.getCount() << std::endl;
    std::cout << "是否已满: " << (rb.isFull() ? "是" : "否") << std::endl;
    
    // 4. 读取数据
    uint8_t value;
    while (!rb.isEmpty()) {
        rb.pop(&value);
        std::cout << (char)value;
    }
    std::cout << std::endl;
    
    return 0;
}
```

## Qt可视化界面功能

### 主要功能

1. **缓冲区初始化**
   - 设置缓冲区大小（1-100字节）
   - 初始化或重新创建缓冲区

2. **数据写入**
   - 写入单个字节（可指定值0-255）
   - 批量写入随机数据

3. **数据读取**
   - 读取单个字节
   - 读取全部数据

4. **状态显示**
   - 缓冲区大小
   - 已存储数据量
   - 可用空间
   - 写指针位置（Head）
   - 读指针位置（Tail）
   - 当前状态（为空/正常/已满）

5. **可视化展示**
   - 进度条显示填充率
   - 图形化显示每个字节的状态
   - 不同颜色标记读写指针位置
   - 显示字节的数值和对应字符

6. **操作日志**
   - 记录所有操作的时间和结果

### 可视化说明

- **绿色格子**：已存储数据的位置
- **灰色格子**：空闲位置
- **红色边框**：写指针（Head）位置
- **橙色边框**：读指针（Tail）位置
- **紫色边框**：读写指针重合位置

## 编译说明

### 前置要求

- Visual Studio 2019 或更高版本
- Qt 5.15.0（项目配置中指定）
- Qt Visual Studio Tools 插件

### 编译步骤

1. 打开 `Circle_pool.sln` 解决方案
2. 选择编译配置（Debug 或 Release）
3. 编译项目（Ctrl+Shift+B）
4. 运行程序（F5）

## 性能特点

1. **高效内存使用**：固定大小的内存空间，避免频繁的内存分配和释放
2. **快速存取**：O(1)时间复杂度的读写操作
3. **线程安全**：C++封装类提供了线程安全的操作（使用原子锁）
4. **批量操作**：支持批量读写，提高数据传输效率
5. **内存安全**：边界检查，防止缓冲区溢出

## 应用场景

1. **数据通信**：串口、网络数据缓存
2. **音频处理**：音频数据流缓冲
3. **生产者-消费者模型**：多线程数据交换
4. **嵌入式系统**：资源受限环境下的数据缓存
5. **实时系统**：需要确定性的数据处理

## 注意事项

1. 初始化后需要手动调用 `RingBuffer_Destroy` 释放资源
2. C接口不是线程安全的，多线程环境下请使用 `RingBufferCPP` 类
3. 缓冲区满时，写入操作会失败，不会覆盖旧数据
4. 建议根据实际应用场景选择合适的缓冲区大小

## 作者信息

- 项目：环形缓冲区静态库
- 环境：Windows + Visual Studio + Qt5
- 日期：2026年4月
