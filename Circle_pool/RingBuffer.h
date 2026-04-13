#pragma once

#include <cstdint>
#include <cstring>
#include <atomic>

#ifdef __cplusplus
extern "C" {
#endif


struct RingBuffer {
    uint8_t* buffer;        // 缓冲区空间指针
    uint32_t size;          // 缓冲区总大小
    uint32_t head;          // 写指针（Head）：指向下一个写入位置
    uint32_t tail;          // 读指针（Tail）：指向下一个读取位置
    bool is_full;           // 缓冲区是否已满标志
};

RingBuffer* RingBuffer_Init(uint32_t size);


void RingBuffer_Destroy(RingBuffer* rb);


bool RingBuffer_Push(RingBuffer* rb, uint8_t data);

bool RingBuffer_Pop(RingBuffer* rb, uint8_t* p_data);


uint32_t RingBuffer_PushBatch(RingBuffer* rb, const uint8_t* data, uint32_t length);


uint32_t RingBuffer_PopBatch(RingBuffer* rb, uint8_t* data, uint32_t length);


uint32_t RingBuffer_GetCount(const RingBuffer* rb);


uint32_t RingBuffer_GetFreeSpace(const RingBuffer* rb);


bool RingBuffer_IsEmpty(const RingBuffer* rb);


bool RingBuffer_IsFull(const RingBuffer* rb);


void RingBuffer_Clear(RingBuffer* rb);


uint32_t RingBuffer_GetSize(const RingBuffer* rb);


uint32_t RingBuffer_GetTail(const RingBuffer* rb);


uint32_t RingBuffer_GetHead(const RingBuffer* rb);


bool RingBuffer_Peek(const RingBuffer* rb, uint32_t offset, uint8_t* p_data);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

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
    
private:
    RingBuffer* m_rb;
    mutable std::atomic_flag m_lock;
};
#endif
