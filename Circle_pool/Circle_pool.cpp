#include "Circle_pool.h"
#include <QDateTime>
#include <QRandomGenerator>
#include <QScrollBar>

Circle_pool::Circle_pool(QWidget *parent)
    : QMainWindow(parent)
    , m_ringBuffer(nullptr)
{
    ui.setupUi(this);
    
   
    updateStatus();
    addLog("环形缓冲区可视化");
    addLog("使用方法：1.先设置缓冲区大小并点击\"初始化\"按钮");
}

Circle_pool::~Circle_pool()
{
    
    if (m_ringBuffer != nullptr) {
        RingBuffer_Destroy(m_ringBuffer);
        m_ringBuffer = nullptr;
    }
}

void Circle_pool::on_btn_init_clicked()
{
   
    if (m_ringBuffer != nullptr) {
        RingBuffer_Destroy(m_ringBuffer);
        m_ringBuffer = nullptr;
        addLog("释放旧的缓冲区");
    }
    
    
    uint32_t size = ui.spinBox_size->value();
    

    m_ringBuffer = RingBuffer_Init(size);
    
    if (m_ringBuffer != nullptr) {
        addLog(QString("成功初始化环形缓冲区，大小: %1 字节").arg(size));
        updateStatus();
    } else {
        addLog("初始化环形缓冲区失败！");
    }
}

void Circle_pool::on_btn_clear_clicked()
{
    if (m_ringBuffer == nullptr) {
        addLog("错误: 缓冲区未初始化");
        return;
    }
    
    RingBuffer_Clear(m_ringBuffer);
    addLog("已清空缓冲区");
    updateStatus();
}

void Circle_pool::on_btn_push_clicked()
{
    if (m_ringBuffer == nullptr) {
        addLog("错误: 缓冲区未初始化");
        return;
    }
    
    
    if (RingBuffer_IsFull(m_ringBuffer)) {
        addLog("错误: 缓冲区已满，无法写入");
        return;
    }
    

    uint8_t data = static_cast<uint8_t>(ui.spinBox_data->value());
    
  
    if (RingBuffer_Push(m_ringBuffer, data)) {
        addLog(QString("成功写入数据: %1 (%2) 到位置 Head=%3")
               .arg(data)
               .arg(formatByte(data))
               .arg(RingBuffer_GetHead(m_ringBuffer)));
        updateStatus();
    } else {
        addLog("写入数据失败");
    }
}

void Circle_pool::on_btn_push_random_clicked()
{
    if (m_ringBuffer == nullptr) {
        addLog("错误: 缓冲区未初始化");
        return;
    }

    uint32_t freeSpace = RingBuffer_GetFreeSpace(m_ringBuffer);
    if (freeSpace == 0) {
        addLog("错误: 缓冲区已满，无法写入");
        return;
    }
    
    
    uint32_t writeCount = QRandomGenerator::global()->bounded(1, qMax(2, static_cast<int>(freeSpace / 2)));
    writeCount = qMin(writeCount, freeSpace);
    
    uint32_t actualWritten = 0;
    QString dataStr;
    
    for (uint32_t i = 0; i < writeCount; ++i) {
        uint8_t randomData = static_cast<uint8_t>(QRandomGenerator::global()->bounded(0, 256));
        if (RingBuffer_Push(m_ringBuffer, randomData)) {
            actualWritten++;
            dataStr += QString("%1 ").arg(randomData);
        } else {
            break;
        }
    }
    
    addLog(QString("成功写入 %1 个随机字节: %2").arg(actualWritten).arg(dataStr.trimmed()));
    updateStatus();
}

void Circle_pool::on_btn_pop_clicked()
{
    if (m_ringBuffer == nullptr) {
        addLog("错误: 缓冲区未初始化");
        return;
    }
    

    if (RingBuffer_IsEmpty(m_ringBuffer)) {
        addLog("错误: 缓冲区为空，无法读取");
        return;
    }
    
 
    uint8_t data;
    if (RingBuffer_Pop(m_ringBuffer, &data)) {
        ui.lineEdit_pop->setText(QString("%1 (%2)").arg(data).arg(formatByte(data)));
        addLog(QString("成功从位置 Tail=%1 读取数据: %2 (%3)")
               .arg(RingBuffer_GetTail(m_ringBuffer))
               .arg(data)
               .arg(formatByte(data)));
        updateStatus();
    } else {
        addLog("读取数据失败");
    }
}

void Circle_pool::on_btn_pop_all_clicked()
{
    if (m_ringBuffer == nullptr) {
        addLog("错误: 缓冲区未初始化");
        return;
    }
    
   
    if (RingBuffer_IsEmpty(m_ringBuffer)) {
        addLog("错误: 缓冲区为空，无法读取");
        return;
    }
    
    uint32_t count = RingBuffer_GetCount(m_ringBuffer);
    QString dataStr;
    uint32_t readCount = 0;
    
    while (!RingBuffer_IsEmpty(m_ringBuffer)) {
        uint8_t data;
        if (RingBuffer_Pop(m_ringBuffer, &data)) {
            dataStr += QString("%1 ").arg(data);
            readCount++;
        } else {
            break;
        }
    }
    
    addLog(QString("成功读取全部 %1 个字节: %2").arg(readCount).arg(dataStr.trimmed()));
    updateStatus();
}

void Circle_pool::updateStatus()
{
    if (m_ringBuffer == nullptr) {
    
        ui.label_size->setText("0 字节");
        ui.label_count->setText("0 字节");
        ui.label_head->setText("0");
        ui.label_tail->setText("0");
        ui.label_free->setText("0 字节");
        ui.label_status->setText("未初始化");
        ui.label_status->setStyleSheet("font-weight: bold; color: gray;");
        ui.progressBar->setValue(0);
        ui.textEdit_data->setHtml("<p style='color: #a0a0a0;'>缓冲区未初始化</p>");
        
  
        ui.btn_push->setEnabled(false);
        ui.btn_push_random->setEnabled(false);
        ui.btn_pop->setEnabled(false);
        ui.btn_pop_all->setEnabled(false);
        ui.btn_clear->setEnabled(false);
    } else {
  
        uint32_t size = RingBuffer_GetSize(m_ringBuffer);
        uint32_t count = RingBuffer_GetCount(m_ringBuffer);
        uint32_t head = RingBuffer_GetHead(m_ringBuffer);
        uint32_t tail = RingBuffer_GetTail(m_ringBuffer);
        uint32_t freeSpace = RingBuffer_GetFreeSpace(m_ringBuffer);
        bool isFull = RingBuffer_IsFull(m_ringBuffer);
        bool isEmpty = RingBuffer_IsEmpty(m_ringBuffer);
        
        ui.label_size->setText(QString("%1 字节").arg(size));
        ui.label_count->setText(QString("%1 字节").arg(count));
        ui.label_head->setText(QString::number(head));
        ui.label_tail->setText(QString::number(tail));
        ui.label_free->setText(QString("%1 字节").arg(freeSpace));
        
       
        if (isFull) {
            ui.label_status->setText("已满");
            ui.label_status->setStyleSheet("font-weight: bold; color: red;");
        } else if (isEmpty) {
            ui.label_status->setText("为空");
            ui.label_status->setStyleSheet("font-weight: bold; color: #00aa00;");
        } else {
            ui.label_status->setText("正常");
            ui.label_status->setStyleSheet("font-weight: bold; color: blue;");
        }
        
  
        int percentage = (count * 100) / size;
        ui.progressBar->setValue(percentage);
        
      
        ui.btn_push->setEnabled(!isFull);
        ui.btn_push_random->setEnabled(!isFull);
        ui.btn_pop->setEnabled(!isEmpty);
        ui.btn_pop_all->setEnabled(!isEmpty);
        ui.btn_clear->setEnabled(true);
        
      
        updateBufferDisplay();
    }
}

void Circle_pool::updateBufferDisplay()
{
    if (m_ringBuffer == nullptr) {
        return;
    }
    
    uint32_t size = RingBuffer_GetSize(m_ringBuffer);
    uint32_t head = RingBuffer_GetHead(m_ringBuffer);
    uint32_t tail = RingBuffer_GetTail(m_ringBuffer);
    bool isFull = RingBuffer_IsFull(m_ringBuffer);
    bool isEmpty = RingBuffer_IsEmpty(m_ringBuffer);
    
    QString html = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">";
    html += "<html><head><style type=\"text/css\">";
    html += ".cell { display: inline-block; width: 60px; height: 60px; margin: 2px; ";
    html += "border: 2px solid #999; text-align: center; vertical-align: middle; ";
    html += "font-family: monospace; font-size: 11px; }";
    html += ".empty { background-color: #f0f0f0; color: #999; }";
    html += ".filled { background-color: #90EE90; color: #000; }";
    html += ".head { border-color: #ff0000; border-width: 3px; }";
    html += ".tail { border-color: #ffa500; border-width: 3px; }";
    html += ".both { border-color: #ff00ff; border-width: 3px; }";
    html += "</style></head><body>";
    
 
    html += "<p style='font-size: 12px;'>";
    html += "<span style='color: red; font-weight: bold;'>红色边框 = 写指针(Head)</span> | ";
    html += "<span style='color: orange; font-weight: bold;'>橙色边框 = 读指针(Tail)</span> | ";
    html += "<span style='color: magenta; font-weight: bold;'>紫色边框 = 两者重合</span>";
    html += "</p>";
    html += "<div style='line-height: 0px;'>";
    
   
    for (uint32_t i = 0; i < size; ++i) {
        QString cellClass = "cell ";
        
      
        bool hasData = false;
        if (isEmpty) {
            hasData = false;
        } else if (isFull) {
            hasData = true;
        } else if (head > tail) {
           
            hasData = (i >= tail && i < head);
        } else {
           
            hasData = (i >= tail || i < head);
        }
        
       
        if (hasData) {
            cellClass += "filled ";
            
           
            uint8_t data;
            if (RingBuffer_Peek(m_ringBuffer, 0, &data)) {
               
                uint32_t offset;
                if (i >= tail) {
                    offset = i - tail;
                } else {
                    offset = (size - tail) + i;
                }
                RingBuffer_Peek(m_ringBuffer, offset, &data);
            }
        } else {
            cellClass += "empty ";
        }
        
        
        bool isHead = (i == head);
        bool isTail = (i == tail);
        
        if (isHead && isTail) {
            cellClass += "both ";
        } else if (isHead) {
            cellClass += "head ";
        } else if (isTail) {
            cellClass += "tail ";
        }
        
       
        QString dataStr = "-";
        if (hasData) {
            
            uint32_t offset;
            if (i >= tail) {
                offset = i - tail;
            } else {
                offset = (size - tail) + i;
            }
            uint8_t data;
            if (RingBuffer_Peek(m_ringBuffer, offset, &data)) {
                dataStr = QString("%1<br/>%2").arg(data).arg(QChar(data));
            }
        }
        
        html += QString("<div class='%1'>[%2]<br/>%3</div>")
                .arg(cellClass)
                .arg(i)
                .arg(dataStr);
    }
    
    html += "</div></body></html>";
    
    ui.textEdit_data->setHtml(html);
}

void Circle_pool::addLog(const QString& message)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString logEntry = QString("[%1] %2").arg(timestamp).arg(message);
    
    ui.textEdit_log->append(logEntry);
    
    
    QScrollBar* sb = ui.textEdit_log->verticalScrollBar();
    sb->setValue(sb->maximum());
}

QString Circle_pool::formatByte(uint8_t byte)
{
    if (byte >= 32 && byte <= 126) {
        return QString("'%1'").arg(QChar(byte));
    } else {
        return QString("0x%1").arg(byte, 2, 16, QChar('0')).toUpper();
    }
}
