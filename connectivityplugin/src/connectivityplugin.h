/**
 * Copyright (c) 2012 Nokia Corporation.
 * All rights reserved.
 *
 * For the applicable distribution terms see the license text file included in
 * the distribution.
 */

#ifndef CONNECTIVITYPLUGIN_H
#define CONNECTIVITYPLUGIN_H

#include <QDeclarativeExtensionPlugin>

class ConnectivityPlugin : public QDeclarativeExtensionPlugin
{
    Q_OBJECT

public:
    explicit ConnectivityPlugin(QObject *parent = 0);
    void registerTypes(const char *uri);
};


#endif // CONNECTIVITYPLUGIN_H

