# 快速入门指南

## 编译和运行

### 方法1：使用Qt可视化界面

1. 在Visual Studio中打开 `Circle_pool.sln`
2. 确保已安装 Qt 5.15.0 和 Qt Visual Studio Tools
3. 选择配置（Debug 或 Release）
4. 编译项目（Ctrl+Shift+B）
5. 运行程序（F5）

### 方法2：运行独立测试程序

1. 在项目中添加 `test_ringbuffer.cpp` 到编译列表
2. 或者创建一个新的控制台项目，只包含：
   - `RingBuffer.h`
   - `RingBuffer.cpp`
   - `test_ringbuffer.cpp`
3. 编译并运行

## 可视化界面使用说明

### 1. 初始化缓冲区
- 设置缓冲区大小（1-100字节）
- 点击"初始化"按钮创建缓冲区

### 2. 写入数据
- **写入单个字节**：
  - 在"数据值"输入框中输入0-255的数值
  - 点击"写入一个字节"
  
- **写入随机数据**：
  - 点击"写入随机数据"按钮
  - 程序会随机写入1到可用空间一半的字节数

### 3. 读取数据
- **读取单个字节**：
  - 点击"读取一个字节"
  - 读取的数据会显示在右侧输入框
  
- **读取全部**：
  - 点击"读取全部"按钮
  - 一次性读取缓冲区中的所有数据

### 4. 查看状态
界面实时显示：
- 缓冲区总大小
- 已存储数据量
- 可用空间
- 写指针（Head）位置
- 读指针（Tail）位置
- 当前状态（为空/正常/已满）

### 5. 可视化展示
- **进度条**：显示缓冲区填充率
- **格子视图**：
  - 绿色格子 = 已存储数据
  - 灰色格子 = 空闲位置
  - 红色边框 = 写指针位置
  - 橙色边框 = 读指针位置
  - 紫色边框 = 读写指针重合
  - 显示每个字节的数值和对应字符

### 6. 操作日志
记录所有操作的时间和结果

## 独立使用环形缓冲区库

### C接口示例

```cpp
#include "RingBuffer.h"

int main() {
    // 1. 创建缓冲区
    RingBuffer* rb = RingBuffer_Init(1024);
    
    // 2. 写入数据
    uint8_t data = 65; // 'A'
    if (RingBuffer_Push(rb, data)) {
        // 写入成功
    }
    
    // 3. 读取数据
    uint8_t value;
    if (RingBuffer_Pop(rb, &value)) {
        // 读取成功，value = 65
    }
    
    // 4. 查看状态
    uint32_t count = RingBuffer_GetCount(rb);
    bool isFull = RingBuffer_IsFull(rb);
    bool isEmpty = RingBuffer_IsEmpty(rb);
    
    // 5. 清空缓冲区
    RingBuffer_Clear(rb);
    
    // 6. 释放资源
    RingBuffer_Destroy(rb);
    
    return 0;
}
```

### C++类示例（线程安全）

```cpp
#include "RingBuffer.h"

int main() {
    // 1. 创建缓冲区（自动管理内存）
    RingBufferCPP rb(1024);
    
    // 2. 写入数据
    rb.push('H');
    rb.push('e');
    rb.push('l');
    rb.push('l');
    rb.push('o');
    
    // 3. 读取数据
    uint8_t data;
    while (!rb.isEmpty()) {
        rb.pop(&data);
        std::cout << (char)data;
    }
    
    // 自动析构，无需手动释放
    return 0;
}
```

## 常见问题

### Q1: 缓冲区满了还能写入吗？
A: 不能。当缓冲区满时，`Push` 函数会返回 `false`。数据不会被覆盖。

### Q2: 缓冲区空时读取会怎样？
A: `Pop` 函数会返回 `false`，`p_data` 参数不会被修改。

### Q3: 如何知道应该创建多大的缓冲区？
A: 根据应用场景：
- 串口通信：建议至少能存储一次完整的数据包
- 音频缓冲：建议至少存储50-100ms的音频数据
- 生产者-消费者：根据生产和消费速度的差异确定

### Q4: C接口和C++类有什么区别？
A: 
- C接口：手动内存管理，适合C语言或需要精确控制的情况
- C++类：自动内存管理，线程安全，适合现代C++项目

### Q5: 为什么我的数据显示"-"而不是数值？
A: 该位置是空闲的，还没有数据写入。写入数据后会显示具体数值。

### Q6: 读写指针重合时，缓冲区是空还是满？
A: 通过 `is_full` 标志区分：
- `is_full == false`：缓冲区为空
- `is_full == true`：缓冲区已满

## 性能建议

1. **合理设置缓冲区大小**：过小会导致频繁的满/空状态，过大会浪费内存
2. **使用批量操作**：批量读写比单个操作更高效
3. **多线程环境**：使用 `RingBufferCPP` 类，它提供了线程安全保证
4. **避免频繁初始化**：缓冲区可以重复使用，只需 `Clear` 而不需要重新 `Init`

## 下一步

- 查看 `README.md` 了解详细的API文档
- 运行 `test_ringbuffer.cpp` 查看完整的功能测试
- 根据自己的需求修改和扩展功能
