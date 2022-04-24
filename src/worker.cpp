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

#include "worker.h"

#include <QDebug>
#include <QFile>
#include <QApplication>

#include <pigpio.h>

void Worker::callback(int, int level, uint32_t, void *userdata)
{

    Worker *thiz = reinterpret_cast<Worker*>(userdata);
    thiz->gpio(level);
}

static inline unsigned long long getTimestampMicroseconds() {
return std::chrono::duration_cast<std::chrono::microseconds>(
    std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

void Worker::run()   
{
    gpioInitialise();

    gpioSetMode(INPUT_PIN, PI_INPUT);
    gpioSetAlertFuncEx(INPUT_PIN, callback, this);

    gpioSetMode(OUTPUT_PIN, PI_OUTPUT);
    gpioSetPWMfrequency(OUTPUT_PIN, TONE_FEQUENCY);
    msleep(5);
    gpioPWM(OUTPUT_PIN, 0);

    while (true) {
        msleep(20);
        if (processProgram) {
            running = true;
            qDebug() << "Provadim program o " << measurement.states.size() << " krocich";
            startTimestamp = getTimestampMicroseconds();
            date = QDate::currentDate();
            time = QTime::currentTime();

            int progressUpdate = 0;
            int totalElapsedTime = 0;

            int previousValue = measurement.states.at(0).second;
            gpioPWM(20, previousValue*127); //Set PWM to either 0 or 50%
            for (int i = 0; i < measurement.states.size(); i++) {
                QPair<int, int> pair = measurement.states.at(i);

                unsigned long long waitTime = pair.first * 1000; //Convert millisecond to microseconds                
                unsigned long long elapsedTime = 0;
                unsigned long long previousTimestamp = getTimestampMicroseconds();
                unsigned long long current;

                if (previousValue != pair.second) {
                    gpioPWM(OUTPUT_PIN, pair.second*127);
                    previousValue = pair.second;
                }

                QString log = QString::number(previousTimestamp) + ";" + QString::number(totalElapsedTime) + ";PROGRAM;" + QString::number(pair.second) + "\n";
                logs.append(log);

                while (elapsedTime < waitTime) {
                    usleep(5);
                    if (running == false) {
                        break;
                    }
                    current = getTimestampMicroseconds();
                    elapsedTime += (current - previousTimestamp);
                    totalElapsedTime += (current - previousTimestamp);
                    progressUpdate += (current - previousTimestamp);

                    previousTimestamp = current;

                    if (progressUpdate >= 100000) {
                        progressUpdate = 0;
                        float progress = ((totalElapsedTime/1000.0f) / measurement.length) * 100;                        
                        emit updateProgress(progress);
                    }
                }

                if (running == false) {
                    processProgram = false;
                    break;
                }

            }
            //stop program
            if (running) {
                emit updateProgress(100);
                dumpLog(true);
            } else {
                dumpLog(false);
            }
            running = false;
            processProgram = false;            
            emit programFinished();
        }
    }
}

void Worker::dumpLog(bool complete)
{
    QString filename = qApp->applicationDirPath() + "/results/";
    if (!complete) {
        filename += "partial_";
    }

    filename = filename + date.toString("yyyy-MM-dd") + "_" + time.toString("hh-mm-ss") + "_" + measurement.filename;

    qDebug() << "CSV dump to " << filename;

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }
    QTextStream out(&file);
    for (QString rec : logs) {
        out << rec;
    }
    file.close();
    logs.clear();
}

void Worker::startProgram(Measurement m)
{
    this->measurement = m;
    processProgram = true;
}

void Worker::stopProgram()
{
    running = false;
}

void Worker::gpio(int value)
{
    if (running) {
        unsigned long long current = getTimestampMicroseconds();
        QString log = QString::number(current) + ";" + QString::number(current - startTimestamp) + ";USER;" + QString::number(value) + "\n";
        logs.append(log);
    }
    emit buttonPressed(value);
}
