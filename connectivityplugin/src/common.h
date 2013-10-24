/**
 * Copyright (c) 2012 Nokia Corporation.
 * All rights reserved.
 *
 * For the applicable distribution terms see the license text file included in
 * the distribution.
 */

#ifndef COMMON_H
#define COMMON_H

#include <QIODevice>
#include <QByteArray>
#include <QObject>
#include <QString>

namespace Common
{
void resetBuffer();
void readFromSocket(QIODevice *socket, QObject *caller,
                    const QString &finishedSignal, const QString &callee);

QByteArray toMessage(const QString &message, bool compression = false);
QByteArray toMessage(const QByteArray &message, bool compression = false);
}

#endif // COMMON_H
