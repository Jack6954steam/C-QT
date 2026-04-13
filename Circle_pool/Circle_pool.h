#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_Circle_pool.h"
#include "RingBuffer.h"

class Circle_pool : public QMainWindow
{
    Q_OBJECT

public:
    Circle_pool(QWidget *parent = nullptr);
    ~Circle_pool();

private slots:
    // 初始化缓冲区
    void on_btn_init_clicked();
    
    // 清空缓冲区
    void on_btn_clear_clicked();
    
    // 写入一个字节
    void on_btn_push_clicked();
    
    // 写入随机数据
    void on_btn_push_random_clicked();
    
    // 读取一个字节
    void on_btn_pop_clicked();
    
    // 读取全部数据
    void on_btn_pop_all_clicked();

private:
    Ui::Circle_poolClass ui;
    
    // 环形缓冲区指针
    RingBuffer* m_ringBuffer;
    
    // 更新显示状态
    void updateStatus();
    
    // 更新缓冲区数据显示
    void updateBufferDisplay();
    
    // 添加日志
    void addLog(const QString& message);
    
    // 格式化字节显示（十六进制和字符）
    QString formatByte(uint8_t byte);
};
