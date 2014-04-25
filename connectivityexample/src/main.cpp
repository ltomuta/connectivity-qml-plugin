/**
 * Copyright (c) 2012-2014 Microsoft Mobile.
 */

#include <QApplication>
#include <QDeclarativeView>
#include <QDeclarativeEngine>
#include <QDeclarativeContext>
#include <QObject>

#ifdef Q_WS_SIMULATOR
#include <QNetworkProxyFactory>
#endif

#include "connectivityplugin.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Construct and register the connectivity plug-in
    ConnectivityPlugin connectivityPlugin;
    connectivityPlugin.registerTypes("ConnectivityPlugin");

    QDeclarativeView view;

#ifdef Q_OS_SYMBIAN
    view.setAttribute(Qt::WA_NoSystemBackground);
#endif

#ifdef Q_WS_SIMULATOR
    QNetworkProxyFactory::setUseSystemConfiguration(true);
#endif

    view.setResizeMode(QDeclarativeView::SizeRootObjectToView);
    QObject::connect(view.engine(), SIGNAL(quit()), &app, SLOT(quit()));

#if defined(Q_OS_SYMBIAN) || defined(Q_WS_SIMULATOR)
    view.setSource(QUrl("qrc:/symbian/main.qml"));
#elif defined(Q_WS_HARMATTAN)
    view.setSource(QUrl("qrc:/harmattan/main.qml"));
#endif

    view.showFullScreen();
    return app.exec();
}
