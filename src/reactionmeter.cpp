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

#include "reactionmeter.h"
#include "ui_reactionmeter.h"

#include <QDate>
#include <QTime>
#include <QErrorMessage>
#include <QMessageBox>
#include <QDebug>
#include <QDirIterator>

#include <time.h>
#include <sys/time.h>

ReactionMeter::ReactionMeter(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ReactionMeter)
{
    ui->setupUi(this);
    timeDialog = nullptr;
    loadDialog = nullptr;

    connect(ui->setupTimeButton, &QPushButton::clicked, this, &ReactionMeter::timeClicked);
    connect(ui->loadProgramButton, &QPushButton::clicked, this, &ReactionMeter::loadClicked);
    connect(ui->startButton, &QPushButton::clicked, this, &ReactionMeter::startClicked);
    connect(ui->stopButton, &QPushButton::clicked, this, &ReactionMeter::stopClicked);
    connect(ui->exportResultsButton, &QPushButton::clicked, this, &ReactionMeter::exportDataClicked);
    connect(ui->exportProgramsButton, &QPushButton::clicked, this, &ReactionMeter::exportProgramsClicked);
    connect(ui->importProgramsButton, &QPushButton::clicked, this, &ReactionMeter::importProgramsClicked);
    connect(ui->shutdownButton, &QPushButton::clicked, this, &ReactionMeter::shutdownClicked);

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timerTimeout()));
    timer->start(500);
    worker = new Worker();
    worker->start(QThread::TimeCriticalPriority);

    connect(worker, &Worker::updateProgress, ui->progressBar, &QProgressBar::setValue);
    connect(worker, &Worker::programFinished, this, &ReactionMeter::programFinished);
    connect(worker, &Worker::buttonPressed, this, &ReactionMeter::buttonPressed);

    watcher = new QFileSystemWatcher(this);
    watcher->addPath(USB_PATH);
    connect(watcher, &QFileSystemWatcher::directoryChanged, this, &ReactionMeter::directoryChanged);
    checkUSB();
}

ReactionMeter::~ReactionMeter()
{
    delete ui;
}

void ReactionMeter::timeClicked()
{
    if (timeDialog != nullptr) {
        delete timeDialog;
    }
    timeDialog = new TimeDialog();
    connect(timeDialog, &TimeDialog::accepted, this, &ReactionMeter::timeAccepted);
    timeDialog->showFullScreen();
}

void ReactionMeter::timerTimeout()
{
    QDate date = QDate::currentDate();
    QTime time = QTime::currentTime();
    QString dateTime = date.toString("dd.MM.yyyy") + "  " + time.toString("hh:mm:ss");
    ui->timeLabel->setText(dateTime);
}

void ReactionMeter::timeAccepted()
{
    DateTime dt = timeDialog->getDateTime();

    struct tm time;
    memset(&time, 0, sizeof(tm));

    time.tm_year = dt.year - 1900;
    time.tm_mon  = dt.month - 1;
    time.tm_mday = dt.day;
    time.tm_hour = dt.hour;
    time.tm_min  = dt.minute;
    time.tm_sec  = 0;

    if (time.tm_year < 0)
    {
        time.tm_year = 0;
    }

    time_t t = mktime(&time);

    if (t != (time_t) -1)
    {
        const struct timeval tv = {t, 0};
        settimeofday(&tv, 0);
    }
}

void ReactionMeter::loadClicked()
{
    if (loadDialog != nullptr) {
        delete loadDialog;
    }
    loadDialog = new LoadDialog();
    connect(loadDialog, &LoadDialog::accepted, this, &ReactionMeter::loadAccepted);
    loadDialog->showFullScreen();
}

void ReactionMeter::loadAccepted()
{
    measurement = loadDialog->getMeasurement();
    if (measurement.filename == "") {
        QMessageBox msgBox;
        msgBox.setText("Unable to load program");
        msgBox.exec();
        ui->startButton->setEnabled(false);
        ui->progressBar->setValue(0);
        ui->progressBar->setEnabled(false);
        ui->programLabel->setText("Program:");
    } else {
        if (measurement.valid == 0) {
            QMessageBox msgBox;
            msgBox.setText("Caution, program is not valid");
            msgBox.exec();
        }
        ui->startButton->setEnabled(true);
        ui->progressBar->setValue(0);
        ui->progressBar->setEnabled(true);
        ui->programLabel->setText("Program: " + measurement.filename);
    }
}

void ReactionMeter::enableUI()
{
    ui->stopButton->setEnabled(false);
    ui->startButton->setEnabled(true);
    ui->loadProgramButton->setEnabled(true);
    if (usbPresent) {
        ui->importProgramsButton->setEnabled(true);
        ui->exportResultsButton->setEnabled(true);
        ui->exportProgramsButton->setEnabled(true);        
    }
}

void ReactionMeter::startClicked()
{
    worker->startProgram(measurement);
    ui->startButton->setEnabled(false);
    ui->stopButton->setEnabled(true);
    ui->importProgramsButton->setEnabled(false);
    ui->exportResultsButton->setEnabled(false);
    ui->exportProgramsButton->setEnabled(false);
    ui->loadProgramButton->setEnabled(false);
    ui->progressBar->setValue(0);
}

void ReactionMeter::stopClicked()
{
    worker->stopProgram();
    enableUI();
}

void ReactionMeter::directoryChanged(const QString &)
{
    checkUSB();
}

void ReactionMeter::programFinished()
{
    enableUI();
}

void ReactionMeter::importProgramsClicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Import programs", "Overwrite local programs?", QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);
    if (reply == QMessageBox::Yes || reply == QMessageBox::No) {       
        QString path = usbPath + "/programs";
        QDirIterator it(path, QStringList() << "*.csv", QDir::Files, QDirIterator::Subdirectories);
        int counter = 0;
        while (it.hasNext()) {
            counter++;
            QString filePath = it.next();
            QString fileName = it.fileName();
            if (QFile::exists(qApp->applicationDirPath() + "/programs/" + fileName)) {
                if (reply == QMessageBox::Yes) {
                    qDebug() << "Mazu " << filePath;
                    QFile::remove(qApp->applicationDirPath() + "/programs/" + fileName);
                }
                continue;
            }
            bool ok = QFile::copy(filePath, qApp->applicationDirPath() + "/programs/" + fileName);

            if(!ok) {
                qDebug() << "Error while moving/copying file to device";
                return;
            }
        }
        system("sudo sync");
        QMessageBox msgBox;
        msgBox.setText(QString::number(counter) + " programs imported");
        msgBox.exec();
    }

}

void ReactionMeter::exportProgramsClicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Export programs", "Overwrite programs on USB?", QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);
    if (reply == QMessageBox::Yes || reply == QMessageBox::No) {
        int counter = 0;
        QString path = usbPath + "/programs";
        if (!QDir(path).exists()) {
            if (!QDir().mkdir(path)) {
                qDebug() << "Error while creating directory on USB";
                return;
            }
        }

        path = qApp-> applicationDirPath() + "/programs";
        QDirIterator it(path, QStringList() << "*.csv", QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            counter++;
            QString filePath = it.next();
            QString fileName = it.fileName();
            if (QFile::exists(usbPath + "/programs/" + fileName)) {
                if (reply == QMessageBox::Yes ) {
                    qDebug() << "Mazu " << filePath;
                    QFile::remove(filePath);
                }
                continue;
            }
            bool ok = QFile::copy(filePath, usbPath + "/programs/" + fileName);

            if(!ok) {
                qDebug() << "Error while moving/copying file to USB";
                return;
            }
        }
        system("sudo sync");
        QMessageBox msgBox;
        msgBox.setText(QString::number(counter) + " programs exported");
        msgBox.exec();
    }
}

void ReactionMeter::exportDataClicked()
{
    QMessageBox msgBox;
    msgBox.setText(tr("Move or copy?"));
    QAbstractButton* pButtonMove = msgBox.addButton(tr("Move"), QMessageBox::YesRole);
    QAbstractButton* pButtonCopy = msgBox.addButton(tr("Copy"), QMessageBox::NoRole);
    msgBox.addButton(tr("Cancel"), QMessageBox::RejectRole);

    msgBox.exec();

    if (msgBox.clickedButton() == pButtonMove || msgBox.clickedButton() == pButtonCopy) {
        int counter = 0;
        QString path = usbPath + "/results";
        if (!QDir(path).exists()) {
            if (!QDir().mkdir(path)) {
                qDebug() << "Error while creating directory on USB";
                return;
            }
        }

        path = qApp-> applicationDirPath() + "/results";
        QDirIterator it(path, QStringList() << "*.csv", QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            counter++;
            QString filePath = it.next();
            QString fileName = it.fileName();
            if (QFile::exists(usbPath + "/results/" + fileName)) {
                if (msgBox.clickedButton() == pButtonMove) {
                    qDebug() << "Mazu " << filePath;
                    QFile::remove(filePath);
                }
                continue;
            }
            bool ok = QFile::copy(filePath, usbPath + "/results/" + fileName);
            if (msgBox.clickedButton() == pButtonMove) {
                qDebug() << "Mazu " << filePath;
                ok = QFile::remove(filePath);
            }

            if(!ok) {
                qDebug() << "Error while moving/copying file to USB";
                return;
            }
        }
        system("sudo sync");
        QMessageBox msgBox;
        msgBox.setText(QString::number(counter) + " results exported");
        msgBox.exec();
    }
}

void ReactionMeter::shutdownClicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Shutdown", "Do you wish to shutdown system?", QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        system("sudo shutdown -h now");
    }
}

void ReactionMeter::buttonPressed(int value)
{
    if (value == 0) {
        ui->timeLabel->setStyleSheet("QLabel { background-color : transparent; }");
    } else {
        ui->timeLabel->setStyleSheet("QLabel { background-color : #0ab19a; }");
    }
}

void ReactionMeter::checkUSB()
{
    QDirIterator it(USB_PATH);
    while (it.hasNext()) {
        QString dir = it.next();
        qDebug() << dir;
        if (dir.contains(".")) {
            continue;
        }

        usbPresent = true;
        usbPath = dir;
        if (ui->startButton->isEnabled() || ui->loadProgramButton->isEnabled()) {
            ui->exportResultsButton->setEnabled(true);
            ui->exportProgramsButton->setEnabled(true);
            ui->importProgramsButton->setEnabled(true);
        }
        return;
    }
    usbPresent = false;
    ui->exportResultsButton->setEnabled(false);
    ui->exportProgramsButton->setEnabled(false);
    ui->importProgramsButton->setEnabled(false);
}

