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

#pragma once

#include "../../Globals.h"


#ifdef MINTGGGAMEENGINE_PORT_DESKTOP

#include <unordered_map>

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMainWindow>

#include "../../core/Game.h"

namespace MINTGGGameEngine
{

class MainWindow : public QMainWindow
{
private:
    enum class TransformMode
    {
        None,
        Fast,
        Smooth
    };

public:
    static void setup(Game* game);
    static void shutdown(Game* game);
    static MainWindow* instance();

public:
    void displayFrame (
        const void* data,
        uint16_t width, uint16_t height,
        PixelFormat pixelFormat, ByteOrder byteOrder
        );

protected:
    void closeEvent(QCloseEvent* evt) override;
    void keyPressEvent(QKeyEvent* evt) override;
    void keyReleaseEvent(QKeyEvent* evt) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;

private:
    MainWindow(Game* game);

    void loadConfig();

    std::string mapQtKeyToButtonID(Qt::Key key) const;

private:
    static MainWindow* inst;

    Game* game;

    QGraphicsView* graphicsView;
    QGraphicsScene* graphicsScene;
    TransformMode transformMode;
    bool keepAspectRatio;

    std::unordered_map<Qt::Key, std::string> keymap;
};

}

#endif