#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QSettings>
#include <QTimer>

class NoteListWidget;
class NoteEditWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    void onNoteSelected(int noteId);
    void onNoteSaved();
    void onNoteDeleted();
    void onNewNote();
    void onToggleEditPreview();
    void onStatusMessage(const QString& msg);
    void onTitleChanged(const QString& title);

private:
    void setupUI();
    void loadSettings();
    void saveSettings();

    QSplitter* m_splitter = nullptr;
    NoteListWidget* m_noteList = nullptr;
    NoteEditWidget* m_editWidget = nullptr;
    QSettings* m_settings = nullptr;
    QTimer* m_statusTimer = nullptr;
};

#endif // MAINWINDOW_H