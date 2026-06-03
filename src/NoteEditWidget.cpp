#include "NoteEditWidget.h"
#include "DatabaseManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QDateTime>
#include <QTextDocument>
#include <QDebug>
#include <QDesktopServices>
#include <QUrl>
#include <QWheelEvent>
#include <QRegularExpression>

NoteEditWidget::NoteEditWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    QHBoxLayout* toolbar = new QHBoxLayout();
    toolbar->setContentsMargins(12, 8, 12, 8);
    m_editModeBtn = new QPushButton("编辑", this);
    m_editModeBtn->setCheckable(true);
    m_editModeBtn->setChecked(true);
    m_previewModeBtn = new QPushButton("预览", this);
    m_previewModeBtn->setCheckable(true);
    toolbar->addWidget(m_editModeBtn);
    toolbar->addWidget(m_previewModeBtn);
    toolbar->addStretch();
    m_newBtn = new QPushButton("新建", this);
    m_saveBtn = new QPushButton("保存", this);
    m_deleteBtn = new QPushButton("删除", this);
    toolbar->addWidget(m_newBtn);
    toolbar->addWidget(m_saveBtn);
    toolbar->addWidget(m_deleteBtn);
    mainLayout->addLayout(toolbar);

    QWidget* titleWidget = new QWidget(this);
    QVBoxLayout* titleLayout = new QVBoxLayout(titleWidget);
    titleLayout->setContentsMargins(20, 16, 20, 8);
    m_titleEdit = new QLineEdit(this);
    m_titleEdit->setPlaceholderText("笔记标题");
    m_titleEdit->setStyleSheet("font-size: 28px; font-weight: 600; border: none; background: transparent;");
    titleLayout->addWidget(m_titleEdit);
    m_titleHint = new QLabel(this);
    m_titleHint->setStyleSheet("color: #ff5555; font-size: 12px;");
    m_titleHint->hide();
    titleLayout->addWidget(m_titleHint);
    mainLayout->addWidget(titleWidget);

    m_modeStack = new QStackedWidget(this);
    m_editArea = new QTextEdit(this);
    m_editArea->setStyleSheet("background: #1e1f22; font-family: monospace; font-size: 14px;");
    m_previewArea = new QTextBrowser(this);
    m_previewArea->setOpenExternalLinks(false);
    // 设置预览区域默认字号为16px
    m_previewArea->setStyleSheet("background: #1e1f22; font-size: 16px; line-height: 1.6;");
    m_modeStack->addWidget(m_editArea);
    m_modeStack->addWidget(m_previewArea);
    mainLayout->addWidget(m_modeStack, 1);

    connect(m_newBtn, &QPushButton::clicked, this, &NoteEditWidget::onNew);
    connect(m_editModeBtn, &QPushButton::clicked, [this]() {
        setEditMode(true);
        m_editModeBtn->setChecked(true);
        m_previewModeBtn->setChecked(false);
        onModeChanged(0);
    });
    connect(m_previewModeBtn, &QPushButton::clicked, [this]() {
        setEditMode(false);
        m_editModeBtn->setChecked(false);
        m_previewModeBtn->setChecked(true);
        onModeChanged(1);
        // 切换到预览模式时刷新预览
        updatePreview();
    });
    connect(m_saveBtn, &QPushButton::clicked, this, &NoteEditWidget::onSave);
    connect(m_deleteBtn, &QPushButton::clicked, this, &NoteEditWidget::onDelete);
    connect(m_titleEdit, &QLineEdit::textChanged, this, &NoteEditWidget::onTitleChanged);
    connect(m_editArea, &QTextEdit::textChanged, this, &NoteEditWidget::updatePreview);

    // 内链跳转处理
    connect(m_previewArea, &QTextBrowser::anchorClicked, this, [this](const QUrl& url) {
        qDebug() << "内链点击 - URL:" << url.toString();
        
        if (url.scheme() == "note") {
            QString urlString = url.toString();
            QString title = urlString.mid(7);
            
            if (title.isEmpty()) {
                title = url.path();
            }
            
            title = QUrl::fromPercentEncoding(title.toUtf8());
            title = title.trimmed();
            
            qDebug() << "提取的标题:" << title;
            
            if (title.isEmpty()) {
                emit statusMessage("无效的链接");
                return;
            }
            
            NoteData note = DatabaseManager::instance().getNoteByTitle(title);
            if (note.id != -1) {
                loadNote(note.id);
                emit noteSelected(note.id);
                emit statusMessage(QString("已打开笔记: %1").arg(title));
            } else {
                emit statusMessage(QString("笔记不存在: %1").arg(title));
                QString errorHtml = QString("<span style='color:#ff5555;'>⚠️ 笔记 \"%1\" 不存在，请先创建。</span>").arg(title.toHtmlEscaped());
                m_previewArea->append(errorHtml);
            }
        } else {
            QDesktopServices::openUrl(url);
        }
    });

    // 安装事件过滤器，支持缩放
    m_editArea->installEventFilter(this);
    m_previewArea->installEventFilter(this);

    setEditMode(true);
    clearEditor();
}

void NoteEditWidget::loadNote(int noteId)
{
    if (noteId == -1) {
        clearEditor();
        m_currentNoteId = -1;
        emit titleChanged("");
        return;
    }

    NoteData note = DatabaseManager::instance().getNoteById(noteId);
    if (note.id == -1) {
        clearEditor();
        return;
    }

    m_currentNoteId = note.id;
    m_titleEdit->setText(note.title);
    m_editArea->setPlainText(note.content);
    updatePreview();
    updateTitleHint(false);
    emit titleChanged(note.title);
}

void NoteEditWidget::setEditMode(bool edit)
{
    m_modeStack->setCurrentIndex(edit ? 0 : 1);
    if (!edit) updatePreview();
}

void NoteEditWidget::zoomIn()
{
    if (m_zoomLevel < 10) {
        m_zoomLevel++;
        int editFontSize = 14 + m_zoomLevel;
        int previewFontSize = 16 + m_zoomLevel;
        m_editArea->setStyleSheet(QString("background: #1e1f22; font-family: monospace; font-size: %1px;").arg(editFontSize));
        m_previewArea->setStyleSheet(QString("background: #1e1f22; font-size: %1px; line-height: 1.6;").arg(previewFontSize));
        // 刷新预览内容
        updatePreview();
        emit statusMessage(QString("字号: 编辑 %1px / 预览 %2px").arg(editFontSize).arg(previewFontSize));
    }
}

void NoteEditWidget::zoomOut()
{
    if (m_zoomLevel > -5) {
        m_zoomLevel--;
        int editFontSize = 14 + m_zoomLevel;
        int previewFontSize = 16 + m_zoomLevel;
        m_editArea->setStyleSheet(QString("background: #1e1f22; font-family: monospace; font-size: %1px;").arg(editFontSize));
        m_previewArea->setStyleSheet(QString("background: #1e1f22; font-size: %1px; line-height: 1.6;").arg(previewFontSize));
        // 刷新预览内容
        updatePreview();
        emit statusMessage(QString("字号: 编辑 %1px / 预览 %2px").arg(editFontSize).arg(previewFontSize));
    }
}

void NoteEditWidget::resetZoom()
{
    m_zoomLevel = 0;
    m_editArea->setStyleSheet("background: #1e1f22; font-family: monospace; font-size: 14px;");
    m_previewArea->setStyleSheet("background: #1e1f22; font-size: 16px; line-height: 1.6;");
    updatePreview();
    emit statusMessage("字号已重置");
}

bool NoteEditWidget::eventFilter(QObject* obj, QEvent* event)
{
    if ((obj == m_editArea || obj == m_previewArea) && event->type() == QEvent::Wheel) {
        QWheelEvent* wheel = static_cast<QWheelEvent*>(event);
        if (wheel->modifiers() & Qt::ControlModifier) {
            if (wheel->angleDelta().y() > 0) {
                zoomIn();
            } else {
                zoomOut();
            }
            return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}

void NoteEditWidget::updatePreview()
{
    if (!m_previewArea) return;
    
    QString md = m_editArea->toPlainText();
    QTextDocument doc;
    doc.setMarkdown(md);
    
    // 获取当前预览字体大小（基础16px + 缩放）
    int previewFontSize = 16 + m_zoomLevel;
    
    QString styleSheet = QString(
        "body { color: #e4e6eb; background: #1e1f22; font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Helvetica, Arial, sans-serif; font-size: %1px; line-height: 1.6; }"
        "h1, h2, h3 { color: #f2f3f5; margin-top: 24px; margin-bottom: 16px; font-weight: 600; }"
        "h1 { font-size: 2em; border-bottom: 1px solid #3c3f45; padding-bottom: 0.3em; }"
        "h2 { font-size: 1.5em; border-bottom: 1px solid #3c3f45; padding-bottom: 0.3em; }"
        "h3 { font-size: 1.25em; }"
        "p { margin-bottom: 16px; }"
        "code { background: #2b2d31; border-radius: 4px; padding: 2px 6px; font-family: 'SF Mono', Monaco, 'Cascadia Code', monospace; font-size: 0.9em; }"
        "pre { background: #0f0f12; padding: 16px; border-radius: 8px; overflow-x: auto; }"
        "pre code { background: none; padding: 0; }"
        "blockquote { border-left: 4px solid #3c3f45; padding-left: 16px; margin: 16px 0; color: #b5bac1; }"
        "table { border-collapse: collapse; width: 100%; margin: 16px 0; }"
        "th, td { border: 1px solid #3c3f45; padding: 8px 12px; text-align: left; }"
        "th { background: #2b2d31; font-weight: 600; }"
        "a { color: #58a6ff; text-decoration: none; }"
        "a:hover { text-decoration: underline; }"
        "ul, ol { padding-left: 2em; margin-bottom: 16px; }"
        "li { margin: 4px 0; }"
        "hr { border: none; border-top: 1px solid #3c3f45; margin: 24px 0; }"
    ).arg(previewFontSize);
    
    doc.setDefaultStyleSheet(styleSheet);
    
    QString html = doc.toHtml();
    html = processInternalLinks(html);
    m_previewArea->setHtml(html);
}

QString NoteEditWidget::processInternalLinks(const QString& html) const
{
    QRegularExpression rx("\\[\\[([^\\]]+)\\]\\]");
    QString result = html;
    QRegularExpressionMatchIterator it = rx.globalMatch(result);
    
    QList<QPair<int, int>> matches;
    QList<QString> titles;
    
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        matches.append(qMakePair(match.capturedStart(), match.capturedLength()));
        titles.append(match.captured(1));
    }
    
    for (int i = matches.size() - 1; i >= 0; --i) {
        QString title = titles[i];
        NoteData note = DatabaseManager::instance().getNoteByTitle(title);
        QString linkColor = (note.id != -1) ? "#58a6ff" : "#ff5555";
        QString tooltip = (note.id != -1) ? QString("打开笔记: %1").arg(title) : QString("笔记不存在: %1").arg(title);
        
        QByteArray encodedTitle = QUrl::toPercentEncoding(title);
        QString link = QString("<a href=\"note://%1\" style=\"color: %2; text-decoration: none; border-bottom: 1px dashed %2;\" title=\"%3\">%4</a>")
                       .arg(QString::fromUtf8(encodedTitle), linkColor, tooltip, title.toHtmlEscaped());
        result.replace(matches[i].first, matches[i].second, link);
    }
    
    return result;
}

void NoteEditWidget::onTitleChanged(const QString& text)
{
    bool duplicate = DatabaseManager::instance().isTitleDuplicate(text, m_currentNoteId);
    updateTitleHint(duplicate);
    if (m_currentNoteId == -1 && !text.isEmpty()) {
        emit titleChanged(text);
    } else if (m_currentNoteId != -1 && !duplicate) {
        emit titleChanged(text);
    }
}

void NoteEditWidget::updateTitleHint(bool duplicate)
{
    if (duplicate) {
        m_titleHint->setText("此标题与已有的条目冲突");
        m_titleHint->show();
        m_saveBtn->setEnabled(false);
    } else {
        m_titleHint->hide();
        m_saveBtn->setEnabled(true);
    }
}

void NoteEditWidget::onSave()
{
    QString title = m_titleEdit->text().trimmed();
    if (title.isEmpty()) {
        emit statusMessage("标题不能为空");
        return;
    }
    if (DatabaseManager::instance().isTitleDuplicate(title, m_currentNoteId)) {
        emit statusMessage("标题已存在，保存失败");
        return;
    }

    NoteData note;
    note.id = m_currentNoteId;
    note.title = title;
    note.content = m_editArea->toPlainText();

    if (DatabaseManager::instance().saveNote(note)) {
        bool isNew = (m_currentNoteId == -1);
        m_currentNoteId = note.id;
        emit noteSaved();
        emit statusMessage("笔记已保存");
        if (isNew) {
            emit noteSelected(m_currentNoteId);
            emit titleChanged(title);
        }
        updatePreview();
    } else {
        emit statusMessage("保存失败");
    }
}

void NoteEditWidget::onDelete()
{
    if (m_currentNoteId == -1) return;
    int ret = QMessageBox::question(this, "确认删除", "确定删除此笔记？");
    if (ret == QMessageBox::Yes && DatabaseManager::instance().deleteNote(m_currentNoteId)) {
        emit noteDeleted();
        emit statusMessage("笔记已删除");
        clearEditor();
        emit titleChanged("");
    }
}

void NoteEditWidget::onNew()
{
    clearEditor();
    m_currentNoteId = -1;
    emit titleChanged("");
    emit statusMessage("新建笔记");
}

void NoteEditWidget::clearEditor()
{
    m_currentNoteId = -1;
    m_titleEdit->clear();
    m_editArea->clear();
    m_previewArea->clear();
    updateTitleHint(false);
}

void NoteEditWidget::onModeChanged(int index)
{
    Q_UNUSED(index);
}