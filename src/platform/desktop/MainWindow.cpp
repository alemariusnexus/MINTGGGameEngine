/*
 * This file is part of MINTGGGameEngine.
 *
 * Copyright 2025-2026 David Lerch
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

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

void MainWindow::shutdown(Game* game)
{
    inst->close();
}

MainWindow* MainWindow::instance()
{
    return inst;
}


MainWindow::MainWindow(Game* game)
    : QMainWindow(), game(game), transformMode(TransformMode::Fast), keepAspectRatio(true)
{
    setWindowTitle(QString::fromStdString(game->getApplicationName()));

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

    if (StorageEngine::getInstance().hasValue("_QtMainGeometry")) {
        auto geom = QByteArray::fromStdString(StorageEngine::getInstance().readValue<std::string>("_QtMainGeometry"));
        restoreGeometry(geom);
    }

    show();
}

void MainWindow::displayFrame (
    const void* data,
    uint16_t width, uint16_t height,
    PixelFormat pixelFormat, ByteOrder byteOrder
) {
    if (pixelFormat == PixelFormat::RGBX8888  ||  pixelFormat == PixelFormat::XBGR8888) {
        // TODO: Support these formats maybe.
        return;
    }

    QImage screenImage;

    auto qimageData = new uchar[width*height*CalcPixelFormatBytesPerPixel(pixelFormat)];
    bool qimageDataFilled = false;

    if (byteOrder != ByteOrder::Native) {
        if (pixelFormat == PixelFormat::RGB565  ||  pixelFormat == PixelFormat::BGR565) {
            // Swap endianness
            auto data16 = reinterpret_cast<uint16_t*>(const_cast<void*>(data));
            auto qimageData16 = reinterpret_cast<uint16_t*>(qimageData);
            for (uint16_t y = 0 ; y < height ; y++) {
                for (uint16_t x = 0 ; x < width ; x++) {
                    *qimageData16++ = SwapEndianness(*data16++);
                }
            }
            qimageDataFilled = true;
        } else if ( pixelFormat == PixelFormat::RGB888
                ||  pixelFormat == PixelFormat::BGR888
                ||  pixelFormat == PixelFormat::RGBX8888
                ||  pixelFormat == PixelFormat::XBGR8888
        ) {
            // Byte order doesn't have a meaning for these formats
        } else {
            assert(false);
        }
    }

    if (!qimageDataFilled) {
        memcpy(qimageData, data, width*height*CalcPixelFormatBytesPerPixel(pixelFormat));
    }

    const QImageCleanupFunction cleanupFunc = [](void* d) { delete[] reinterpret_cast<uchar*>(d); };
    switch (pixelFormat) {
    case PixelFormat::RGB565:
        screenImage = QImage(reinterpret_cast<const uchar*>(qimageData), width, height, QImage::Format_RGB16,
            cleanupFunc, qimageData);
        break;
    case PixelFormat::BGR565:
        screenImage = QImage(reinterpret_cast<const uchar*>(qimageData), width, height, QImage::Format_RGB16,
            cleanupFunc, qimageData);
        screenImage.rgbSwap();
        break;
    case PixelFormat::RGB888:
        screenImage = QImage(reinterpret_cast<const uchar*>(qimageData), width, height, QImage::Format_RGB888,
            cleanupFunc, qimageData);
        break;
    case PixelFormat::BGR888:
        screenImage = QImage(reinterpret_cast<const uchar*>(qimageData), width, height, QImage::Format_BGR888,
            cleanupFunc, qimageData);
        break;
    default:
        assert(false);
        delete[] qimageData;
        return;
    }

    graphicsScene->clear();

    QGraphicsPixmapItem* item = graphicsScene->addPixmap(QPixmap::fromImage(screenImage));

    if (transformMode == TransformMode::None) {
        item->setTransformationMode(Qt::FastTransformation);
        graphicsView->resetTransform();
    } else if (transformMode == TransformMode::Fast) {
        item->setTransformationMode(Qt::FastTransformation);
        graphicsView->fitInView(0, 0, width, height, keepAspectRatio ? Qt::KeepAspectRatio : Qt::IgnoreAspectRatio);
    } else if (transformMode == TransformMode::Smooth) {
        item->setTransformationMode(Qt::SmoothTransformation);
        graphicsView->fitInView(0, 0, width, height, keepAspectRatio ? Qt::KeepAspectRatio : Qt::IgnoreAspectRatio);
    }
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
            this->transformMode = TransformMode::Smooth;
        } else if (transformMode == "fast") {
            this->transformMode = TransformMode::Fast;
        } else if (transformMode == "none") {
            this->transformMode = TransformMode::None;
        }

        keepAspectRatio = jdoc.object().value("screen").toObject().value("keepAspectRatio").toBool(keepAspectRatio);
    } else {
        LogWarning("Desktop config not found. You can create one at %s...", desktopCfgPath.toUtf8().constData());
    }
}

void MainWindow::closeEvent(QCloseEvent* evt)
{
    QByteArray geom = saveGeometry();
    StorageEngine::getInstance().writeValue("_QtMainGeometry", geom.toStdString());

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

void MainWindow::mouseDoubleClickEvent(QMouseEvent* evt)
{
    QMainWindow::mouseDoubleClickEvent(evt);

    if (isFullScreen()) {
        showNormal();
    } else {
        showFullScreen();
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
