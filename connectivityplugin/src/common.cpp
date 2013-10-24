/**
 * Copyright (c) 2012 Nokia Corporation.
 * All rights reserved.
 *
 * For the applicable distribution terms see the license text file included in
 * the distribution.
 */

#include "common.h"

#include <QMetaObject>
#include <QDebug>
#include <QStringList>
#include <QBitArray>

namespace
{

int endian();
int bitsToInt(const QBitArray &bits);
QBitArray readHeader(QByteArray &message);
QBitArray bytesToBits(const QByteArray &bytes);
QBitArray numberToBits(const int &number);
QByteArray bitsToBytes(const QBitArray &bits);

/*!
  Returns 0 if platform is little endian, 1 for big endian platforms.
*/
int endian()
{
    int i = 1;
    char *p = (char *)&i;

    if (p[0] == 1)
        return 0; //LITTLE_ENDIAN
    else
        return 1; //BIG_ENDIAN
}

/*!
  Reads the header from \a message.
  Header is the first 4 bytes, indexed from 0 to 3, 4th byte is ":" if header exists.
  Returns the header in bits if exists, otherwise empty QBitArray.
*/
QBitArray readHeader(QByteArray &message)
{
    int index = message.indexOf(":");
    if (index == -1 || index != 4) {
        return QBitArray();
    }

    //First 4 bytes contain the header information
    QByteArray headerBytes = message.mid(0, 4);

    message = message.mid(index + 1);

    return ::bytesToBits(headerBytes);
}

int bitsToInt(const QBitArray &bits)
{
    int number = 0;
    int bit = 1;

    for (int i = 0; i < bits.size(); ++i) {
        if (bits.testBit(i)) {
            int index = endian() == 0 ? ((bits.size() - 1) - i) : i;
            int mask = (bit << index);
            number |= mask;
        }
    }

    return number;
}


// Convert from QByteArray to QBitArray
QBitArray bytesToBits(const QByteArray &bytes)
{
    QBitArray bits(bytes.size()*8);
    for(int i = 0; i < bytes.size(); ++i) {
        for(int b = 0; b < 8; ++b) {
            bits.setBit(i * 8 + b, bytes.at(i) & (1 << b));
        }
    }
    return bits;
}

// Convert from QBitArray to QByteArray
QByteArray bitsToBytes(const QBitArray &bits)
{
    QByteArray bytes;
    bytes.resize(bits.count()/8);
    bytes.fill(0);
    for(int b = 0; b < bits.count(); ++b) {
        bytes[b / 8] = (bytes.at(b / 8) | ((bits[b] ? 1 : 0) << (b % 8)));
    }
    return bytes;
}

QBitArray numberToBits(const int &number)
{
    QBitArray num(sizeof(int) * 8);

    qDebug() << "Common::numberToBits(): Endian:" << ::endian();

    for (int i = 0; i < num.size(); ++i) {
        int index = ::endian() == 0 ? ((num.size() - 1) - i) : i;
        bool bit = !!(number & (1 << index));
        num.setBit(i, bit);
    }

    return num;
}


bool gCompressionEnabled = false; //Used to determine whether or not compression is enabled for the incoming data.
int gExpectedSize = -1; //Expected size in bytes, -1 means that we accept all data
QByteArray gBuffer = QByteArray(); //Buffer to store data

} //anonymous namespace

namespace Common
{

/*!
  Used to reset buffer and related variables.
*/

void resetBuffer()
{
    ::gCompressionEnabled = false;
    ::gExpectedSize = -1;
    ::gBuffer.clear();
}

/*!
  A helper function to help reading data from \a socket into \a buffer. \a expectedSize
  is used to compare size of the buffer to the size of data that should be recieved.
  \a caller is used in combination with \a finishedSignal as parameters to
  QMetaObject::invokeMethod to emit a signal when \c buffer.size() == \a expectedSize.
  \a callee is used only for debugging purposes to output debug information containing
  the name of the calling function.
*/
void readFromSocket(QIODevice *socket, QObject *caller,
                    const QString &finishedSignal, const QString &callee)
{
#define PRINT_DEBUG(dbgMessage) \
    if (!callee.isEmpty()) { \
        qDebug() << callee.toLocal8Bit().data() << dbgMessage;\
    }\

    qint64 bytes = socket->bytesAvailable();

    QByteArray byteArray;
    byteArray.resize(bytes);

    PRINT_DEBUG("Bytes available" << bytes);

    qint64 read = socket->read(byteArray.data(), bytes);

    if (read == -1) {
        PRINT_DEBUG("Error reading data");
    }   

    QBitArray header = ::readHeader(byteArray);

    if (!header.isEmpty()) {
        ::gCompressionEnabled = header.testBit(0); //set compression
        header.setBit(0, false); //Reset first bit
        ::gExpectedSize = ::bitsToInt(header);

        PRINT_DEBUG("Expecting" << ::gExpectedSize << "bytes");
    }

    if (!byteArray.isEmpty()) {
        int totalSize = ::gBuffer.size() + byteArray.size();

        if (::gExpectedSize == -1 || totalSize <= ::gExpectedSize) {
            ::gBuffer.append(byteArray);
        } else {
            //If we have unexpected elements we'll add as much as we can and discard the rest.
            unsigned int roomFor = ::gExpectedSize - ::gBuffer.size();
            QByteArray temp = byteArray.mid(0, roomFor);
            ::gBuffer.append(temp);
        }
    }

    if ((::gBuffer.size() == ::gExpectedSize && ::gExpectedSize > 0) || ::gExpectedSize == -1) {        

        PRINT_DEBUG("Received" << ::gBuffer.size() << "bytes");
        if (::gBuffer.size() > 5) {
            PRINT_DEBUG("Ends with" << ::gBuffer.mid(::gBuffer.size() - 5));
        }

        if (::gCompressionEnabled) {
            PRINT_DEBUG("Data was compressed");
            ::gBuffer = qUncompress(::gBuffer);
        }

        QMetaObject::invokeMethod(caller, finishedSignal.toAscii(), Q_ARG(QByteArray, ::gBuffer));

        Common::resetBuffer();

    } else {
        PRINT_DEBUG("Waiting for:" << ::gExpectedSize - ::gBuffer.size() << "bytes");
    }

#undef PRINT_DEBUG
}

/*!
  Creates and returns a new bytearray from \a message, compressed if \a compression was set.
  Adds a 4 byte header to the message indicating the size and compression status.
*/
QByteArray toMessage(const QByteArray &message, bool compression )
{
    QByteArray compressed = (compression ? qCompress(message) : message);
    QBitArray header = ::numberToBits(compressed.size());
    //First bit is used to indicate compression
    header.setBit(0, compression);
    QByteArray msg = ::bitsToBytes(header) + ":" + compressed;
    return msg;
}

/*!
  Creates a new bytearray from \a message, compressed if \a compression was set.
  Adds a 4 byte header to the message indicating the size and compression status.
*/
QByteArray toMessage(const QString &message, bool compression)
{
    return toMessage(message.toAscii(), compression);
}

} //namespace Common
