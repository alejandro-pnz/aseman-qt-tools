/*
    Copyright (C) 2017 Aseman Team
    http://aseman.co

    AsemanQtTools is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    AsemanQtTools is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef ASEMANSYSTEMINFO_H
#define ASEMANSYSTEMINFO_H

#include <QObject>

#include "asemantools_global.h"

class AsemanSystemInfoPrivate;
class LIBASEMANTOOLSSHARED_EXPORT AsemanSystemInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(quint64 cpuCores READ cpuCores NOTIFY fakeSignal)
    Q_PROPERTY(quint64 cpuFreq READ cpuFreq NOTIFY fakeSignal)

public:
    AsemanSystemInfo(QObject *parent = 0);
    virtual ~AsemanSystemInfo();

public Q_SLOTS:
    quint64 cpuCores();
    quint64 cpuFreq();

Q_SIGNALS:
    void fakeSignal();

private:
    AsemanSystemInfoPrivate *p;
};

#endif // ASEMANSYSTEMINFO_H
