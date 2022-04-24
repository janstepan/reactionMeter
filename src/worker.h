/*
# Copyright (C) 2022 Jan Stepan
# This file is part of reactionMeter <https://github.com/janstepan/reactionMeter>.
#
# reactionMeter is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# reactionMeter is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with reactionMeter. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef WORKER_H
#define WORKER_H

#include <QThread>
#include <QDate>
#include <QTime>
#include "loaddialog.h"

#define INPUT_PIN 16
#define OUTPUT_PIN 20
#define TONE_FEQUENCY 400


class Worker : public QThread
{
    Q_OBJECT
public:    
    void run() override;

public slots:
    void startProgram(Measurement m);
    void stopProgram();

signals:
    void updateProgress(int);
    void programFinished();
    void buttonPressed(int);

private:
    static void callback(int gpio, int level, uint32_t tick, void *userdata);

    void gpio(int value);

    void dumpLog(bool complete);
    Measurement measurement;
    bool running = false;
    bool processProgram = false;

    QVector<QString> logs;
    unsigned long long startTimestamp;
    QDate date;
    QTime time;
};

#endif // WORKER_H
