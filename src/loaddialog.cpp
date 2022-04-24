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

#include "loaddialog.h"
#include "ui_loaddialog.h"

#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QIntValidator>

LoadDialog::LoadDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoadDialog)
{
    ui->setupUi(this);

    model = new QStandardItemModel(this);
    proxyModel = new QSortFilterProxyModel(this);

    model->setHorizontalHeaderItem(0, new QStandardItem("Name"));
    model->setHorizontalHeaderItem(1, new QStandardItem("Valid"));
    model->setHorizontalHeaderItem(2, new QStandardItem("Length"));
    model->setHorizontalHeaderItem(3, new QStandardItem("Changes"));

    proxyModel->setSourceModel(model);

    ui->tableView->setModel(proxyModel);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setSortingEnabled(true);
    ui->tableView->verticalHeader()->setVisible(false);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);



    QString path = qApp-> applicationDirPath() + "/programs";
    if (!QDir(path).exists()) {
        QDir().mkdir(path);
    }

    QDirIterator it(path, QStringList() << "*.csv", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString filePath = it.next();
        QString fileName = it.fileName();

        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return;
        QIntValidator vT(0, 2147483647, this);

        int previousValue = 0;
        Measurement measurement;
        measurement.length = 0;
        measurement.changes = 0;
        measurement.filename = fileName;
        measurement.valid = true;
        while (!file.atEnd()) {
            QByteArray line = file.readLine();
            QString s(line);
            QRegExp rx("(\\;|\\n)");
            QStringList l = s.split(rx);

            if (l.length() != 3) {
                qDebug() << "Error";
                measurement.valid = false;
                continue;
            }

            int pos = 0;
            QString time = l.at(0);
            QString value = l.at(1);
            int v = value.toInt();

            if (vT.validate(time, pos) == QValidator::State::Invalid) {
                measurement.valid = false;
                qDebug() << "Error1 " << pos;
                continue;
            }

            if (v < 0 || v > 1) {
                measurement.valid = false;
                qDebug() << "Error2 " << value;
                continue;
            }

            int t = time.toInt();

            measurement.length += t; //Increase total length

            if (previousValue != v) {
                previousValue = v;
                measurement.changes++;
            }
            QPair<int, int> state;
            state.first = t;
            state.second = v;

            measurement.states.push_back(state);
        }

        measurements.insert(fileName, measurement);

        QStandardItem *fileItem = new QStandardItem(measurement.filename);
        qDebug() << "Valid: " << measurement.valid;
        QStandardItem *validItem = new QStandardItem(QString::number(measurement.valid));
        QStandardItem *lengthItem = new QStandardItem(QString::number(measurement.length) + " ms");
        QStandardItem *changesItem = new QStandardItem(QString::number(measurement.changes));

        QList<QStandardItem*> items = {fileItem, validItem, lengthItem, changesItem};
        model->appendRow(items);
    }
    if (measurements.size() > 0) {
        ui->tableView->selectRow(0);
    }
}

LoadDialog::~LoadDialog()
{
    delete ui;
    delete model;
    delete proxyModel;
}

Measurement LoadDialog::getMeasurement()
{
    Measurement m;
    m.filename = "";
    QModelIndexList selectedList = ui->tableView->selectionModel()->selectedRows();
    if (selectedList.count() > 0) {
        QModelIndex proxyIndex = selectedList.at(0);
        QStandardItem *item = model->item(proxyModel->mapToSource(proxyIndex).row(), 0);
        QString file = item->data(Qt::DisplayRole).toString();
        return measurements[file];
    }
    return m;
}
