#ifndef NOTEEDITWIDGET_H
#define NOTEEDITWIDGET_H

#include <QWidget>
#include <QTextEdit>
#include <QTextBrowser>
#include <QLineEdit>
#include <QStackedWidget>
#include <QLabel>
#include <QPushButton>

class NoteEditWidget : public QWidget
{
    Q_OBJECT
public:
    explicit NoteEditWidget(QWidget *parent = nullptr);
    void loadNote(int noteId);
    void setEditMode(bool edit);
    void zoomIn();
    void zoomOut();
    void resetZoom();
    int currentNoteId() const { return m_currentNoteId; }

public slots:
    void onNew();

signals:
    void noteSaved();
    void noteDeleted();
    void noteSelected(int noteId);
    void statusMessage(const QString& msg);
    void titleChanged(const QString& title);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void onSave();
    void onDelete();
    void onTitleChanged(const QString& text);
    void onModeChanged(int index);

private:
    void updatePreview();
    void updateTitleHint(bool duplicate);
    void clearEditor();
    QString processInternalLinks(const QString& html) const;
    QStringList getAllNoteTitles() const;

    int m_currentNoteId = -1;
    int m_zoomLevel = 0;

    QLineEdit* m_titleEdit = nullptr;
    QLabel* m_titleHint = nullptr;
    QStackedWidget* m_modeStack = nullptr;
    QTextEdit* m_editArea = nullptr;
    QTextBrowser* m_previewArea = nullptr;

    QPushButton* m_saveBtn = nullptr;
    QPushButton* m_deleteBtn = nullptr;
    QPushButton* m_newBtn = nullptr;
    QPushButton* m_editModeBtn = nullptr;
    QPushButton* m_previewModeBtn = nullptr;
};

#endif // NOTEEDITWIDGET_H