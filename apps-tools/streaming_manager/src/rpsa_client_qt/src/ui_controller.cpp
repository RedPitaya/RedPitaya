#include <QtQml>
#include <QDesktopServices>
#include "ui_controller.h"
#include "models/boardsmodel.h"
#include "models/consolemodel.h"

ui_controller::ui_controller(QObject *parent)
    : QObject{parent}
{

}

ui_controller * ui_controller::instance()
{
    static ui_controller * _instance = 0;
    if ( _instance == 0 ) {
        qmlRegisterType<ui_controller>("UI_Controller", 1, 0, "UI_Controller");
        _instance = new ui_controller();
    }
    return _instance;
}

QObject* ui_controller::getBoardsModel(){
    return BoardsModel::instance();
}

QObject* ui_controller::getConsoleModel(){
    return ConsoleModel::instance();
}

void ui_controller::runAll(){
    BoardsModel::instance()->runAll();
}

void ui_controller::stopAll(){
    BoardsModel::instance()->stopAll();
}

void ui_controller::openFolder(){
    QDesktopServices::openUrl(QUrl("output", QUrl::TolerantMode));
}
