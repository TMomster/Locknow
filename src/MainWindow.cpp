#include "MainWindow.h"
#include "NoteListWidget.h"
#include "NoteEditWidget.h"
#include "DatabaseManager.h"
#include <QSplitter>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QCloseEvent>
#include <QStandardPaths>
#include <QStatusBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
    loadSettings();
    setWindowTitle("Locknow - 新条目");
    resize(1000, 700);
    setMinimumSize(800, 500);
}

MainWindow::~MainWindow()
{
    saveSettings();
    delete m_settings;
}

void MainWindow::setupUI()
{
    m_noteList = new NoteListWidget(this);
    m_editWidget = new NoteEditWidget(this);

    m_splitter = new QSplitter(Qt::Horizontal);
    m_splitter->addWidget(m_noteList);
    m_splitter->addWidget(m_editWidget);
    m_splitter->setStretchFactor(0, 1);
    m_splitter->setStretchFactor(1, 3);
    setCentralWidget(m_splitter);

    QStatusBar* bar = statusBar();
    bar->showMessage("就绪");
    m_statusTimer = new QTimer(this);
    m_statusTimer->setSingleShot(true);
    connect(m_statusTimer, &QTimer::timeout, [bar]() {
        bar->showMessage("就绪");
    });

    connect(m_noteList, &NoteListWidget::noteSelected, this, &MainWindow::onNoteSelected);
    connect(m_editWidget, &NoteEditWidget::noteSaved, this, &MainWindow::onNoteSaved);
    connect(m_editWidget, &NoteEditWidget::noteDeleted, this, &MainWindow::onNoteDeleted);
    connect(m_editWidget, &NoteEditWidget::statusMessage, this, &MainWindow::onStatusMessage);
    connect(m_editWidget, &NoteEditWidget::titleChanged, this, &MainWindow::onTitleChanged);
    connect(m_editWidget, &NoteEditWidget::noteSelected, this, &MainWindow::onNoteSelected);
}

void MainWindow::loadSettings()
{
    m_settings = new QSettings(QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/MomsterTech/Locknow/config.ini",
                               QSettings::IniFormat, this);
    restoreGeometry(m_settings->value("geometry").toByteArray());
    restoreState(m_settings->value("state").toByteArray());
    m_splitter->restoreState(m_settings->value("splitterSizes").toByteArray());
}

void MainWindow::saveSettings()
{
    m_settings->setValue("geometry", saveGeometry());
    m_settings->setValue("state", saveState());
    m_settings->setValue("splitterSizes", m_splitter->saveState());
    m_settings->sync();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    saveSettings();
    event->accept();
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        switch (event->key()) {
        case Qt::Key_N:
            onNewNote();
            event->accept();
            return;
        case Qt::Key_S:
            QMetaObject::invokeMethod(m_editWidget, "onSave", Qt::QueuedConnection);
            event->accept();
            return;
        case Qt::Key_E:
            onToggleEditPreview();
            event->accept();
            return;
        }
    }
    QMainWindow::keyPressEvent(event);
}

void MainWindow::onNoteSelected(int noteId)
{
    m_editWidget->loadNote(noteId);
    m_noteList->selectNoteById(noteId);
}

void MainWindow::onNoteSaved()
{
    m_noteList->refreshNotes();
}

void MainWindow::onNoteDeleted()
{
    m_noteList->refreshNotes();
    m_editWidget->loadNote(-1);
}

void MainWindow::onNewNote()
{
    m_editWidget->onNew();
    m_noteList->clearSelection();
}

void MainWindow::onToggleEditPreview()
{
    static bool editMode = true;
    editMode = !editMode;
    m_editWidget->setEditMode(editMode);
}

void MainWindow::onStatusMessage(const QString& msg)
{
    statusBar()->showMessage(msg);
    m_statusTimer->start(5000);
}

void MainWindow::onTitleChanged(const QString& title)
{
    if (title.isEmpty())
        setWindowTitle("Locknow - 新条目");
    else
        setWindowTitle(QString("Locknow - %1").arg(title));
}