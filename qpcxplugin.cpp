/*
Copyright (c) 2013 Ronie Martinez (ronmarti18@gmail.com)
All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU Lesser General Public License for more
details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301  USA
*/


#include "qpcxplugin.h"
#include "qpcxhandler.h"


QPcxPlugin::QPcxPlugin(QObject *parent) :
    QImageIOPlugin(parent)
{
}

QPcxPlugin::~QPcxPlugin()
{
}

QStringList QPcxPlugin::keys() const
{
    return QStringList() << "pcx" << "dcx";
}

QImageIOPlugin::Capabilities QPcxPlugin::capabilities(
    QIODevice *device, const QByteArray &format) const
{
    Q_UNUSED(device);
    if (format == "pcx")
        return Capabilities(CanRead);
    if (format == "dcx")
        return Capabilities(CanRead|CanReadIncremental);

    return false;
}

QImageIOHandler *QPcxPlugin::create(
    QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new QPcxHandler;
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

#if QT_VERSION < 0x050000
Q_EXPORT_STATIC_PLUGIN(QPcxPlugin)
Q_EXPORT_PLUGIN2(QPcxPlugin, QPcxPlugin)
#endif // QT_VERSION < 0x050000
