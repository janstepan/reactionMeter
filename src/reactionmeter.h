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

#ifndef REACTIONMETER_H
#define REACTIONMETER_H

#include <QMainWindow>
#include <QTimer>
#include <QFileSystemWatcher>

#include "timedialog.h"
#include "loaddialog.h"
#include "worker.h"

#define USB_PATH "/media/pi"

QT_BEGIN_NAMESPACE
namespace Ui { class ReactionMeter; }
QT_END_NAMESPACE

class ReactionMeter : public QMainWindow
{
    Q_OBJECT

public:
    ReactionMeter(QWidget *parent = nullptr);
    ~ReactionMeter();

private slots:
    void timerTimeout();

    void timeClicked();
    void timeAccepted();

    void loadClicked();
    void loadAccepted();

    void startClicked();
    void stopClicked();

    void directoryChanged(const QString & path);

    void programFinished();

    void importProgramsClicked();
    void exportProgramsClicked();
    void exportDataClicked();

    void shutdownClicked();

    void buttonPressed(int value);

private:
    void checkUSB();
    void enableUI();
    bool usbPresent = false;

    Ui::ReactionMeter *ui;
    TimeDialog *timeDialog;
    LoadDialog *loadDialog;
    QTimer *timer;

    Worker *worker;

    QFileSystemWatcher *watcher;
    QString usbPath;

    Measurement measurement;

};
#endif // REACTIONMETER_H
