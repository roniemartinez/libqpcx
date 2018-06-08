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


#include "qpcxhandler.h"
#include <QDebug>

bool QPcxHandler::readPCX(QByteArray data, QImage *image)
{
    QDataStream input(data);
    if (data.isNull())
        input.setDevice(device());
    input.setByteOrder(QDataStream::LittleEndian);

    quint8 manufacturer, version, encoding, bitsPerPixel,
            reserved, colorPlanes;
    quint16 xMin, yMin, xMax, yMax, verticalDPI,
            horizontalDPI, bytesPerPlaneLine,
            paletteType, horizontalScreenSize, verticalScreenSize;

    input >> manufacturer;
    if (manufacturer != 0x0A)
        return false;

    input >> version;
    input >> encoding;
    if (encoding != 0)
        if (encoding != 1)
            return false;

    //Number of bits per pixel in each colour plane (1, 2, 4, 8, 24)
    input >> bitsPerPixel;
    switch (bitsPerPixel) {
    case 1:
    case 2:
    case 4:
    case 8:
    case 24:
        break;
    default: return false;
        break;
    }

    input >> xMin >> yMin >> xMax >> yMax;
    int width = xMax - xMin + 1;
    int height = yMax - yMin + 1;
    input >> verticalDPI >> horizontalDPI;

    //if bitsPerPixel <= 4 palette is present here
    quint8 byte;
    QByteArray colorMap;
    for (int i = 0; i<48; ++i) {
        input >> byte;
        colorMap.append(byte);
    }

    input >> reserved;
    if (reserved != 0)
        return false;

    //observed that pcx file format was badly designed since
    //the number of color planes was placed here and not before the
    //palette. it is needed for 16 color.
    input >> colorPlanes;
    input >> bytesPerPlaneLine;

    /* encountered uneven bytesPerPlaneLine so I decided not to check
     * it if it is an even number or not
    if (bytesPerPlaneLine%2 != 0)
        return false;
    */

    int fillerCount;
    input >> paletteType;
    if (version >= 4) {
        fillerCount = 54;
        input >> horizontalScreenSize >> verticalScreenSize;
    } else {
        fillerCount = 58;
    }

    input.skipRawData(fillerCount);

    /*DO NOT REMOVE: Intended for testing (for developers ONLY)
    qDebug() << "manufacturer: " << manufacturer << endl
             << "version: " << version << endl
             << "encoding: " << encoding << endl
             << "bitsPerPixel: " << bitsPerPixel << endl
             << "reserved: " << reserved << endl
             << "colorPlanes: " << colorPlanes << endl
             << "xMin: " << xMin << endl
             << "yMin: " << yMin << endl
             << "xMax: " << xMax << endl
             << "yMax: " << yMax << endl
             << "verticalDPI: " << verticalDPI << endl
             << "horizontalDPI: " << horizontalDPI << endl
             << "bytesPerPlaneLine: " << bytesPerPlaneLine << endl
             << "paletteType: " << paletteType << endl
             << "horizontalScreenSize: " << horizontalScreenSize << endl
             << "verticalScreenSize: " << verticalScreenSize << endl;
    */

    QByteArray imageData;
    int scanLineLength = bytesPerPlaneLine * colorPlanes;
    int bytesPerLine;
    quint8 count;
    for(int lineCount=0; lineCount < height; ++lineCount)
    {
        bytesPerLine = scanLineLength;
        switch (encoding) {
        case 0: //uncompressed image
            do {
                input >> byte;
                imageData.append(byte);
                --bytesPerLine;
            } while (bytesPerLine != 0);
            break;
        case 1: //RLE decoding of one line
            do {
                input >> byte;
                if ((byte & 0xC0) == 0xC0) {
                    count = byte & 0x3F;
                    input >> byte;
                } else	 {
                    count = 1;
                }

                //Write the pixel run to the buffer
                for (int j=0; j < count; j++) {
                    imageData.append(byte);
                    --bytesPerLine;
                }

                if (bytesPerLine < 0)
                    return false;
            } while(bytesPerLine != 0);
            break;
        }
    }

    switch (bitsPerPixel*colorPlanes) {
    case 1:
        if (colorPlanes == 1 && bitsPerPixel == 1) {
            m_result = new QImage(width, height, QImage::Format_Mono);
            // ugly implementation because can't put imageData directly to
            // the image since line padding exists at odd width
            quint8* byte = (quint8*)imageData.constData();
            quint8 mask;
            for (int i=0; i < height; ++i) {
                for (int j=0; j < bytesPerPlaneLine*8; j+=8) {
                    mask = 0x80;
                    for (int k = 0; k < 8; ++k) {
                        if (j+k < width)
                            m_result->setPixel(j+k, i, (*byte & mask) == mask);
                        mask >>= 1;
                    }
                    ++byte;
                }
            }
            *image = *m_result;
            return true;
        }
        break;
    case 4:
        if (colorPlanes == 4 && bitsPerPixel == 1) {
            m_result = new QImage(width, height, QImage::Format_Indexed8);

            //color table
            /* http://nemesis.lonestar.org/reference/video/cga.html
                                        Color(s)*                           (I)	(R)	(G)	(B)
            Black                                                           OFF	OFF	OFF	OFF
            Blue                                                            OFF	OFF	OFF	ON
            Green                                                           OFF	OFF	ON	OFF
            Cyan (Blue/Green)                                               OFF	OFF	ON	ON
            Red                                                             OFF	ON	OFF	OFF
            Magenta/Purple (Red/Blue)                                       OFF	ON	OFF	ON
            Brown/Orange/Yellow (Red/Green)                                 OFF	ON	ON	OFF
            White/Gray (Red/Green/Blue)                                     OFF	ON	ON	ON
            Black/Gray (Intensity)                                          ON	OFF	OFF	OFF
            Bright Blue (Blue/Intensity)                                    ON	OFF	OFF	ON
            Bright Green (Green/Intensity)                                  ON	OFF	ON	OFF
            Bright Cyan (Blue/Green/Intensity)                              ON	OFF	ON	ON
            Bright Red (Red/Intensity)                                      ON	ON	OFF	OFF
            Bright Magenta/Pink/Bright Purple (Red/Blue/Intensity)          ON	ON	OFF	ON
            Bright Brown/Bright Orange/ Bright Yellow (Red/Green/Intensity)	ON	ON	ON	OFF
            Bright White (Red/Green/Blue/Intensity)                         ON	ON	ON	ON

            http://en.wikipedia.org/wiki/Color_Graphics_Adapter
            Full CGA 16-color palette
            0	black           8	gray
            #000000             #555555
            1	blue            9	light blue
            #0000AA             #5555FF
            2	green           10	light green
            #00AA00             #55FF55
            3	cyan            11	light cyan
            #00AAAA             #55FFFF
            4	red             12	light red
            #AA0000             #FF5555
            5	magenta         13	light magenta
            #AA00AA             #FF55FF
            6	brown           14	yellow
            #AA5500             #FFFF55
            7	light gray      15	white (high intensity)
            #AAAAAA             #FFFFFF
            */
            m_result->setColor(0, qRgb(0,0,0));
            m_result->setColor(1, qRgb(0,0,170));
            m_result->setColor(2, qRgb(0,170,0));
            m_result->setColor(3, qRgb(0,170,170));
            m_result->setColor(4, qRgb(170,0,0));
            m_result->setColor(5, qRgb(170,0,170));
            m_result->setColor(6, qRgb(170,85,0));
            m_result->setColor(7, qRgb(170,170,0170));
            m_result->setColor(8, qRgb(85,85,85));
            m_result->setColor(9, qRgb(85,85,255));
            m_result->setColor(10, qRgb(85,255,85));
            m_result->setColor(11, qRgb(85,255,255));
            m_result->setColor(12, qRgb(255,85,85));
            m_result->setColor(13, qRgb(255,85,255));
            m_result->setColor(14, qRgb(255,255,85));
            m_result->setColor(15, qRgb(255,255,255));

            //image data
            quint8* red = (quint8*)imageData.constData();
            quint8* green = red + bytesPerPlaneLine;
            quint8* blue = green + bytesPerPlaneLine;
            quint8* intensity = blue + bytesPerPlaneLine;
            quint8 mask;
            quint8 colorIndex;
            for (int i=0; i < height; ++i) {
                for (int j=0; j < bytesPerPlaneLine*8; j+=8) {
                    mask = 0x80;
                    for (int k = 0; k < 8; ++k) {
                        colorIndex = 0;
                        if (*red & mask)
                            colorIndex |= 0x01;
                        if (*green & mask)
                            colorIndex |= 0x02;
                        if (*blue & mask)
                            colorIndex |= 0x04;
                        if (*intensity & mask)
                            colorIndex |= 0x08;
                        if (j+k < width)
                            m_result->setPixel(j+k, i, colorIndex);
                        mask >>= 1;
                    }
                    ++red; ++green; ++blue; ++intensity;
                }
                red += scanLineLength - bytesPerPlaneLine;
                green += scanLineLength - bytesPerPlaneLine;
                blue += scanLineLength - bytesPerPlaneLine;
                intensity += scanLineLength - bytesPerPlaneLine;
            }

            *image = *m_result;
            return true;
        } else if (colorPlanes == 1 && bitsPerPixel == 4) { //untested
            m_result = new QImage(width, height, QImage::Format_Indexed8);
            /* For one plane of four bits (16-colour), each byte will
             * represent two pixels. The bits within the byte are in
             * big-endian order, so the most significant bit belongs
             * to the left-most pixel. In other words, a byte of value
             * 0xE4 (binary 11 10 01 00) will have left-to-right pixel
             * values of 3, 2, 1, 0, assuming two bits per pixel. */

            m_result->setColor(0, qRgb(0,0,0));
            m_result->setColor(1, qRgb(0,0,170));
            m_result->setColor(2, qRgb(0,170,0));
            m_result->setColor(3, qRgb(0,170,170));
            m_result->setColor(4, qRgb(170,0,0));
            m_result->setColor(5, qRgb(170,0,170));
            m_result->setColor(6, qRgb(170,85,0));
            m_result->setColor(7, qRgb(170,170,0170));
            m_result->setColor(8, qRgb(85,85,85));
            m_result->setColor(9, qRgb(85,85,255));
            m_result->setColor(10, qRgb(85,255,85));
            m_result->setColor(11, qRgb(85,255,255));
            m_result->setColor(12, qRgb(255,85,85));
            m_result->setColor(13, qRgb(255,85,255));
            m_result->setColor(14, qRgb(255,255,85));
            m_result->setColor(15, qRgb(255,255,255));

            quint8* byte = (quint8*)imageData.constData();
            quint8 mask;
            quint8 colorIndex;
            for (int i=0; i < height; ++i) {
                for (int j=0; j < bytesPerPlaneLine*2; j+=2) {
                    mask = 0x80;
                    for (int k = 0; k < 2; ++k) {
                        if (k == 0) {
                            colorIndex = *byte & mask;
                            mask >>= 4;
                        } else {
                            colorIndex = *byte & mask;
                        }

                        if (j+k < width)
                            m_result->setPixel(j+k, i, colorIndex);
                        mask >>= 4;
                    }
                    ++byte;
                }
            }

            *image = *m_result;
            return true;
        }
        break;
    case 24:
        if (bitsPerPixel == 8 && colorPlanes == 3) {
            m_result = new QImage(width, height, QImage::Format_RGB32);
            quint8 *red = (quint8*)imageData.constData();
            quint8 *green = red + bytesPerPlaneLine;
            quint8 *blue = green + bytesPerPlaneLine;
            for (int i=0; i < height; ++i) {
                for (int j=0; j < bytesPerPlaneLine; ++j) {
                    if (j < width)
                        m_result->setPixel(j,i,qRgb(*red, *green, *blue));
                    ++red; ++green, ++blue;
                }
                red += scanLineLength - bytesPerPlaneLine;
                green += scanLineLength - bytesPerPlaneLine;
                blue += scanLineLength - bytesPerPlaneLine;
            }

            *image = *m_result;
            return true;
        }
        break;
    case 32:
        if (bitsPerPixel == 8 && colorPlanes == 4) { //untested
            m_result = new QImage(width, height, QImage::Format_ARGB32);
            quint8 *red = (quint8*)imageData.constData();
            quint8 *green = red + bytesPerPlaneLine;
            quint8 *blue = green + bytesPerPlaneLine;
            quint8 *alpha = blue + bytesPerPlaneLine;
            for (int i=0; i < height; ++i) {
                for (int j=0; j < bytesPerPlaneLine; ++j) {
                    if (j < width)
                        m_result->setPixel(j,i,qRgba(*red, *green, *blue, *alpha));
                    ++red; ++green, ++blue; ++alpha;
                }
                red += scanLineLength - bytesPerPlaneLine;
                green += scanLineLength - bytesPerPlaneLine;
                blue += scanLineLength - bytesPerPlaneLine;
                alpha += scanLineLength - bytesPerPlaneLine;
            }

            *image = *m_result;
            return true;
        }
        break;
    default:
        if (input.atEnd())
            return false;
        break;
    }

    // if palette exists at the end of the file
    input >> byte;
    if (version == 2 ||
            (version == 5 && bitsPerPixel == 8 && paletteType == 1))
        if (byte != 12) //should be 12 as described in the documentation
            return false;

    // If palette exists at the end of the file
    QByteArray colorData;
    while (!input.atEnd()) {
        input >> byte;
        colorData.append(byte);
    }

    if (bitsPerPixel == 8 && colorPlanes == 1) {
        switch (paletteType) {
        case 1: // Color/B&W
        {
            m_result = new QImage(width, height, QImage::Format_Indexed8);
            if (colorData.size() != 768)
                return false;
            int indexCount = colorData.size() / 3;
            quint8 *red = (quint8*)colorData.constData();
            quint8 *green = red + 1;
            quint8 *blue = green + 1;
            for (int i=0; i < indexCount; ++i) {
                m_result->setColor(i, qRgb(*red, *green, *blue));
                red+=3; green+=3; blue+=3;
            }
            quint8 *data = (quint8*)imageData.constData();
            for (int i=0; i < height; ++i) {
                for (int j=0; j < bytesPerPlaneLine; ++j) {
                    if (j < width)
                        m_result->setPixel(j,i,*data);
                    ++data;
                }
            }
            *image = *m_result;
            return true;
        } break;
        case 2: // grayscale, no samples found / undocumented

            break;
        default: return false;
            break;
        }
    }

    return false;
}

QPcxHandler::QPcxHandler()
    :m_currentImageNo(0), m_numImages(0)
{
}

QPcxHandler::~QPcxHandler()
{
}

bool QPcxHandler::canRead() const
{
    if (canRead(device())) {
        QByteArray signature = device()->peek(4);
        if (signature == QByteArray("\x3A\xDE\x68\xB1")) { //DCX
            setFormat("dcx");
            return true;
        } else if (signature.startsWith("\x0A")) {//PCX
            setFormat("pcx");
            return true;
        } else return false;
    }
    return false;
}

bool QPcxHandler::canRead(QIODevice *device)
{
    QByteArray signature = device->peek(4);
    if (signature == QByteArray("\x3A\xDE\x68\xB1")) //DCX
        return true;
    else if (signature.startsWith("\x0A")) //PCX
        return true;
    return false;
}

bool QPcxHandler::write(const QImage &image)
{
    Q_UNUSED(image);
    return false;
}

bool QPcxHandler::read(QImage *image)
{
    if (format() == "dcx") {
        QDataStream input(device());
        input.setByteOrder(QDataStream::LittleEndian);
        quint32 signature, offset;
        input >> signature;
        if (signature != 0x3ADE68B1)
            return true;
        QVector<quint32> offsetVector;
        do {
            input >> offset;
            offsetVector << offset;
            if (offset != 0)
                ++m_numImages;
        } while (offset != 0);

        device()->reset();
        input.skipRawData(offsetVector.at(m_currentImageNo));
        QByteArray data;
        quint8 byte;
        int size = offsetVector.at(m_currentImageNo+1) -
                offsetVector.at(m_currentImageNo);
        while (data.size() != size) {
            input >> byte;
            data.append(byte);
        }

        return readPCX(data, image);
        //return input.status() == QDataStream::Ok;
    } else if (format() == "pcx") {
        if (readPCX(QByteArray(), image))
            return true;
    }
    return false;
}

int QPcxHandler::currentImageNumber() const
{
    return m_currentImageNo;
}

int QPcxHandler::imageCount() const
{
    return m_numImages;
}

bool QPcxHandler::jumpToImage(int imageNumber)
{
    if (format() == "pcx")
        return false;
    m_currentImageNo = imageNumber;
    QImage image;
    return read(&image);
}

bool QPcxHandler::jumpToNextImage()
{
    if (m_currentImageNo < imageCount())
        ++m_currentImageNo;
    QImage image;
    return read(&image);
}

QVariant QPcxHandler::option(ImageOption option) const
{
    Q_UNUSED(option);
    return QVariant();
}

bool QPcxHandler::supportsOption(ImageOption option) const
{
    if (format() == "dcx")
        return option == IncrementalReading;
    return false;
}
