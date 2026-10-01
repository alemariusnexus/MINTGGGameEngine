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
public:
    static void setup(Game* game);
    static MainWindow* instance();

public:
    void displayFrame(const uint8_t* data, uint16_t width, uint16_t height, QImage::Format format);

protected:
    void closeEvent(QCloseEvent* evt) override;
    void keyPressEvent(QKeyEvent* evt) override;
    void keyReleaseEvent(QKeyEvent* evt) override;

private:
    MainWindow(Game* game);

    void loadConfig();

    std::string mapQtKeyToButtonID(Qt::Key key) const;

private:
    static MainWindow* inst;

    Game* game;

    QGraphicsView* graphicsView;
    QGraphicsScene* graphicsScene;
    Qt::TransformationMode pixmapTransformMode;

    std::unordered_map<Qt::Key, std::string> keymap;
};

}

#endif