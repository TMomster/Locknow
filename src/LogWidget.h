#ifndef LOGWIDGET_H
#define LOGWIDGET_H

#include <QWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

class LogWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LogWidget(QWidget *parent = nullptr);
    void appendMessage(const QString& msg);
    void clear();

public slots:
    void onAppendMessage(const QString& msg);  // 用于跨线程调用

private slots:
    void onClearClicked();
    void onToggleClicked();

private:
    QTextEdit* m_textEdit;
    QPushButton* m_clearBtn;
    QPushButton* m_toggleBtn;
    bool m_isVisible;
};

#endif // LOGWIDGET_H