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


#ifndef QPCXPLUGIN_H
#define QPCXPLUGIN_H

#include <QImageIOPlugin>


class QPcxPlugin : public QImageIOPlugin
{
    Q_OBJECT
#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "QPcxPlugin.json")
#endif // QT_VERSION >= 0x050000
    
public:
    QPcxPlugin(QObject *parent = 0);
    ~QPcxPlugin();
    QStringList keys() const;
    Capabilities capabilities(QIODevice *device,
                    const QByteArray &format) const;
    QImageIOHandler *create(QIODevice *device,
                               const QByteArray &format = QByteArray()) const;
};

#endif // QPCXPLUGIN_H
