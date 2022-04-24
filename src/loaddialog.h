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

#ifndef LOADDIALOG_H
#define LOADDIALOG_H

#include <QDialog>
#include <QSortFilterProxyModel>
#include <QStandardItem>

namespace Ui {
class LoadDialog;
}

struct Measurement {
    QVector<QPair<int, int>> states;
    int length;
    int changes;
    bool valid;
    QString filename;
};


class LoadDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoadDialog(QWidget *parent = nullptr);
    ~LoadDialog();

    Measurement getMeasurement();

private:
    Ui::LoadDialog *ui;
    QStandardItemModel  *model;
    QSortFilterProxyModel *proxyModel;
    QMap<QString, Measurement> measurements;
};

#endif // LOADDIALOG_H
