#include "LogWidget.h"
#include <QDateTime>
#include <QScrollBar>

LogWidget::LogWidget(QWidget *parent)
    : QWidget(parent), m_isVisible(true)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    m_toggleBtn = new QPushButton("收起日志", this);
    m_clearBtn = new QPushButton("清空日志", this);
    btnLayout->addWidget(m_toggleBtn);
    btnLayout->addWidget(m_clearBtn);
    btnLayout->addStretch();
    layout->addLayout(btnLayout);

    m_textEdit = new QTextEdit(this);
    m_textEdit->setReadOnly(true);
    m_textEdit->setStyleSheet("QTextEdit { background-color: #1e1f22; color: #e4e6eb; font-family: monospace; font-size: 12px; }");
    layout->addWidget(m_textEdit);

    connect(m_clearBtn, &QPushButton::clicked, this, &LogWidget::onClearClicked);
    connect(m_toggleBtn, &QPushButton::clicked, this, &LogWidget::onToggleClicked);
}

void LogWidget::appendMessage(const QString& msg)
{
    // 直接调用槽函数（如果在主线程则直接执行，否则通过队列连接）
    QMetaObject::invokeMethod(this, "onAppendMessage", Qt::AutoConnection, Q_ARG(QString, msg));
}

void LogWidget::onAppendMessage(const QString& msg)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    QString formatted = QString("[%1] %2").arg(timestamp, msg);
    m_textEdit->append(formatted);
    QScrollBar* bar = m_textEdit->verticalScrollBar();
    bar->setValue(bar->maximum());
}

void LogWidget::clear()
{
    m_textEdit->clear();
}

void LogWidget::onClearClicked()
{
    clear();
}

void LogWidget::onToggleClicked()
{
    m_isVisible = !m_isVisible;
    m_textEdit->setVisible(m_isVisible);
    m_toggleBtn->setText(m_isVisible ? "收起日志" : "展开日志");
}