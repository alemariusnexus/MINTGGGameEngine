#include "MainWindow.h"

#ifdef MINTGGGAMEENGINE_PORT_DESKTOP

#include <QFile>
#include <QFileInfo>
#include <QGraphicsPixmapItem>
#include <QImage>
#include <QJsonDocument>
#include <QJsonObject>
#include <QKeyEvent>
#include <QVBoxLayout>
#include <QWidget>

#include "../../storage/StorageEngine.h"
#include "../../util/Log.h"

LOG_USE_TAG("MainWindow")


namespace MINTGGGameEngine
{

MainWindow* MainWindow::inst = nullptr;

void MainWindow::setup(Game* game)
{
    inst = new MainWindow(game);
}

MainWindow* MainWindow::instance()
{
    return inst;
}


MainWindow::MainWindow(Game* game)
    : QMainWindow(), game(game), pixmapTransformMode(Qt::FastTransformation)
{
    setWindowTitle(QString::fromStdString(game->getApplicationName()));

    qApp->setOrganizationName("MINTGGGameEngine");
    qApp->setApplicationName(QString::fromStdString(game->getApplicationID()));

    setFocusPolicy(Qt::StrongFocus);

    resize(600, 600);

    auto mainWidget = new QWidget(this);
    auto mainLayout = new QVBoxLayout(mainWidget);
    mainWidget->setLayout(mainLayout);
    setCentralWidget(mainWidget);

    graphicsScene = new QGraphicsScene;
    graphicsView = new QGraphicsView(graphicsScene, mainWidget);
    graphicsView->setFocusPolicy(Qt::NoFocus);
    graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mainLayout->addWidget(graphicsView);

    loadConfig();

    show();
}

void MainWindow::displayFrame(const uint8_t* data, uint16_t width, uint16_t height, QImage::Format format)
{
    QImage screenImage(data, width, height, format);

    graphicsScene->clear();

    QGraphicsPixmapItem* item = graphicsScene->addPixmap(QPixmap::fromImage(screenImage));
    item->setTransformationMode(pixmapTransformMode);

    graphicsView->fitInView(0, 0, width, height, Qt::KeepAspectRatio);
}

void MainWindow::loadConfig()
{
    auto cfgDir = game->storage().getConfigDirectory();
    auto desktopCfgPath = QString::fromStdString(cfgDir.append("/desktop.json"));

    if (QFileInfo(desktopCfgPath).isFile()) {
        LogInfo("Loading desktop config from %s...", desktopCfgPath.toUtf8().constData());

        QFile cfgFile(desktopCfgPath);
        if (!cfgFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            LogError("Error opening desktop.json.");
            return;
        }
        QByteArray jsonRaw = cfgFile.readAll();
        cfgFile.close();

        QJsonParseError jerr;
        QJsonDocument jdoc = QJsonDocument::fromJson(jsonRaw, &jerr);
        if (jdoc.isNull()) {
            LogError("Error loading desktop.json: %s", jerr.errorString().toUtf8().constData());
            return;
        }

        QJsonObject jkeymap = jdoc.object().value("keymap").toObject();
        for (auto it = jkeymap.begin() ; it != jkeymap.end() ; ++it) {
            QKeySequence kseq = QKeySequence::fromString(it.key());

            if (kseq.isEmpty()  ||  kseq.count() != 1) {
                LogWarning("Invalid key in desktop.json: %s", it.key().toUtf8().constData());
                continue;
            }

            keymap[kseq[0].key()] = it.value().toString().toStdString();
        }

        QString transformMode = jdoc.object().value("screen").toObject().value("transformMode").toString().toLower();
        if (transformMode == "smooth") {
            pixmapTransformMode = Qt::SmoothTransformation;
        } else if (transformMode == "fast") {
            pixmapTransformMode = Qt::FastTransformation;
        }
    }
}

void MainWindow::closeEvent(QCloseEvent* evt)
{
    game->quit();
}

void MainWindow::keyPressEvent(QKeyEvent* evt)
{
    QMainWindow::keyPressEvent(evt);

    if (evt->isAutoRepeat()) {
        return;
    }

    std::string buttonID = mapQtKeyToButtonID(static_cast<Qt::Key>(evt->key()));
    if (!buttonID.empty()) {
        game->input().injectButtonPress(buttonID);
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent* evt)
{
    QMainWindow::keyReleaseEvent(evt);

    if (evt->isAutoRepeat()) {
        return;
    }

    std::string buttonID = mapQtKeyToButtonID(static_cast<Qt::Key>(evt->key()));
    if (!buttonID.empty()) {
        game->input().injectButtonRelease(buttonID);
    }
}

std::string MainWindow::mapQtKeyToButtonID(Qt::Key key) const
{
    auto it = keymap.find(key);
    if (it == keymap.end()) {
        return {};
    }
    return it->second;
}

}

#endif
