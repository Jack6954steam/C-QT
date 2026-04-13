#include "RingBuffer.h"
#include <cstdlib>

#ifdef __cplusplus
extern "C" {
#endif


RingBuffer* RingBuffer_Init(uint32_t size) {
    if (size == 0) {
        return nullptr;
    }
    
    // 分配RingBuffer结构体
    RingBuffer* rb = (RingBuffer*)malloc(sizeof(RingBuffer));
    if (rb == nullptr) {
        return nullptr;
    }
    
    // 分配缓冲区空间
    rb->buffer = (uint8_t*)malloc(size * sizeof(uint8_t));
    if (rb->buffer == nullptr) {
        free(rb);
        return nullptr;
    }
    
    
    rb->size = size;
    rb->head = 0;
    rb->tail = 0;
    rb->is_full = false;
    
    
    memset(rb->buffer, 0, size);
    
    return rb;
}


void RingBuffer_Destroy(RingBuffer* rb) {
    if (rb != nullptr) {
        if (rb->buffer != nullptr) {
            free(rb->buffer);
            rb->buffer = nullptr;
        }
        free(rb);
    }
}


bool RingBuffer_Push(RingBuffer* rb, uint8_t data) {
    if (rb == nullptr || rb->buffer == nullptr) {
        return false;
    }
    
    
    if (rb->is_full) {
        return false;
    }
    
   
    rb->buffer[rb->head] = data;
    
    
    rb->head = (rb->head + 1) % rb->size;
    
    
    if (rb->head == rb->tail) {
        rb->is_full = true;
    }
    
    return true;
}


bool RingBuffer_Pop(RingBuffer* rb, uint8_t* p_data) {
    if (rb == nullptr || rb->buffer == nullptr || p_data == nullptr) {
        return false;
    }
    
   
    if (RingBuffer_IsEmpty(rb)) {
        return false;
    }
    
   
    *p_data = rb->buffer[rb->tail];
    
    
    rb->tail = (rb->tail + 1) % rb->size;
    
   
    rb->is_full = false;
    
    return true;
}


uint32_t RingBuffer_PushBatch(RingBuffer* rb, const uint8_t* data, uint32_t length) {
    if (rb == nullptr || rb->buffer == nullptr || data == nullptr || length == 0) {
        return 0;
    }
    
    uint32_t written = 0;
    
    for (uint32_t i = 0; i < length; ++i) {
        if (!RingBuffer_Push(rb, data[i])) {
            break;
        }
        written++;
    }
    
    return written;
}


uint32_t RingBuffer_PopBatch(RingBuffer* rb, uint8_t* data, uint32_t length) {
    if (rb == nullptr || rb->buffer == nullptr || data == nullptr || length == 0) {
        return 0;
    }
    
    uint32_t read_count = 0;
    
    for (uint32_t i = 0; i < length; ++i) {
        if (!RingBuffer_Pop(rb, &data[i])) {
            break;
        }
        read_count++;
    }
    
    return read_count;
}


uint32_t RingBuffer_GetCount(const RingBuffer* rb) {
    if (rb == nullptr || rb->buffer == nullptr) {
        return 0;
    }
    
    if (rb->is_full) {
        return rb->size;
    }
    
    if (rb->head >= rb->tail) {
        return rb->head - rb->tail;
    } else {
        return rb->size - rb->tail + rb->head;
    }
}


uint32_t RingBuffer_GetFreeSpace(const RingBuffer* rb) {
    if (rb == nullptr || rb->buffer == nullptr) {
        return 0;
    }
    
    return rb->size - RingBuffer_GetCount(rb);
}


bool RingBuffer_IsEmpty(const RingBuffer* rb) {
    if (rb == nullptr || rb->buffer == nullptr) {
        return true;
    }
    
    return (rb->head == rb->tail) && !rb->is_full;
}


bool RingBuffer_IsFull(const RingBuffer* rb) {
    if (rb == nullptr || rb->buffer == nullptr) {
        return false;
    }
    
    return rb->is_full;
}


void RingBuffer_Clear(RingBuffer* rb) {
    if (rb == nullptr || rb->buffer == nullptr) {
        return;
    }
    
    rb->head = 0;
    rb->tail = 0;
    rb->is_full = false;
}


uint32_t RingBuffer_GetSize(const RingBuffer* rb) {
    if (rb == nullptr) {
        return 0;
    }
    return rb->size;
}


uint32_t RingBuffer_GetTail(const RingBuffer* rb) {
    if (rb == nullptr) {
        return 0;
    }
    return rb->tail;
}


uint32_t RingBuffer_GetHead(const RingBuffer* rb) {
    if (rb == nullptr) {
        return 0;
    }
    return rb->head;
}


bool RingBuffer_Peek(const RingBuffer* rb, uint32_t offset, uint8_t* p_data) {
    if (rb == nullptr || rb->buffer == nullptr || p_data == nullptr) {
        return false;
    }
    
    uint32_t count = RingBuffer_GetCount(rb);
    if (offset >= count) {
        return false;
    }
    
    uint32_t index = (rb->tail + offset) % rb->size;
    *p_data = rb->buffer[index];
    
    return true;
}

#ifdef __cplusplus
}


RingBufferCPP::RingBufferCPP(uint32_t size) : m_rb(nullptr) {
    m_rb = RingBuffer_Init(size);
    m_lock.clear();
}

RingBufferCPP::~RingBufferCPP() {
    RingBuffer_Destroy(m_rb);
}

bool RingBufferCPP::push(uint8_t data) {
    while (m_lock.test_and_set(std::memory_order_acquire)) {}
    bool result = RingBuffer_Push(m_rb, data);
    m_lock.clear(std::memory_order_release);
    return result;
}

bool RingBufferCPP::pop(uint8_t* p_data) {
    while (m_lock.test_and_set(std::memory_order_acquire)) {}
    bool result = RingBuffer_Pop(m_rb, p_data);
    m_lock.clear(std::memory_order_release);
    return result;
}

uint32_t RingBufferCPP::pushBatch(const uint8_t* data, uint32_t length) {
    while (m_lock.test_and_set(std::memory_order_acquire)) {}
    uint32_t result = RingBuffer_PushBatch(m_rb, data, length);
    m_lock.clear(std::memory_order_release);
    return result;
}

uint32_t RingBufferCPP::popBatch(uint8_t* data, uint32_t length) {
    while (m_lock.test_and_set(std::memory_order_acquire)) {}
    uint32_t result = RingBuffer_PopBatch(m_rb, data, length);
    m_lock.clear(std::memory_order_release);
    return result;
}

uint32_t RingBufferCPP::getCount() const {
    while (m_lock.test_and_set(std::memory_order_acquire)) {}
    uint32_t result = RingBuffer_GetCount(m_rb);
    m_lock.clear(std::memory_order_release);
    return result;
}

uint32_t RingBufferCPP::getFreeSpace() const {
    while (m_lock.test_and_set(std::memory_order_acquire)) {}
    uint32_t result = RingBuffer_GetFreeSpace(m_rb);
    m_lock.clear(std::memory_order_release);
    return result;
}

bool RingBufferCPP::isEmpty() const {
    while (m_lock.test_and_set(std::memory_order_acquire)) {}
    bool result = RingBuffer_IsEmpty(m_rb);
    m_lock.clear(std::memory_order_release);
    return result;
}

bool RingBufferCPP::isFull() const {
    while (m_lock.test_and_set(std::memory_order_acquire)) {}
    bool result = RingBuffer_IsFull(m_rb);
    m_lock.clear(std::memory_order_release);
    return result;
}

void RingBufferCPP::clear() {
    while (m_lock.test_and_set(std::memory_order_acquire)) {}
    RingBuffer_Clear(m_rb);
    m_lock.clear(std::memory_order_release);
}

uint32_t RingBufferCPP::getSize() const {
    while (m_lock.test_and_set(std::memory_order_acquire)) {}
    uint32_t result = RingBuffer_GetSize(m_rb);
    m_lock.clear(std::memory_order_release);
    return result;
}

uint32_t RingBufferCPP::getTail() const {
    while (m_lock.test_and_set(std::memory_order_acquire)) {}
    uint32_t result = RingBuffer_GetTail(m_rb);
    m_lock.clear(std::memory_order_release);
    return result;
}

uint32_t RingBufferCPP::getHead() const {
    while (m_lock.test_and_set(std::memory_order_acquire)) {}
    uint32_t result = RingBuffer_GetHead(m_rb);
    m_lock.clear(std::memory_order_release);
    return result;
}

bool RingBufferCPP::peek(uint32_t offset, uint8_t* p_data) const {
    while (m_lock.test_and_set(std::memory_order_acquire)) {}
    bool result = RingBuffer_Peek(m_rb, offset, p_data);
    m_lock.clear(std::memory_order_release);
    return result;
}

#endif
