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

#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ReactionMeter w;

    QFile file("data/style.qss");
    file.open(QFile::ReadOnly);

     QString styleSheet { QLatin1String(file.readAll()) };
     //my custom modifications to stylesheet
     styleSheet += " QWidget {font-size : 28px} "
                          "QPushButton::disabled { background-color:#737676;}"
                          "QSpinBox {font-size : 50px; color : #c2c7d4}"
                          "QProgressBar {text-align: center; color : #c2c7d4; border: 2px solid #0ab19a;}"
                          "QProgressBar::chunk {background-color: #0ab19a;}";     

    //setup stylesheet
    a.setStyleSheet(styleSheet);

    w.setWindowState(Qt::WindowFullScreen);
    w.show();
    return a.exec();
}
