/*
 * (C) Copyright 2009 Pål Driveklepp
 *
 * Written by: Pål Driveklepp <jalla2000@gmail.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdio.h>
#include <spotify/api.h>
#include <QApplication>
#include <QPushButton>
#include <QWidget>
#include <QObject>
#include <QString>
#include <QCleanlooksStyle>
#include <QGtkStyle>

#include "mainwindow.hpp"
#include "spotworker.hpp"

int main(int argc, char *argv[])
{
    printf("Juniper starting!\n");
    QApplication app(argc, argv);
    QCleanlooksStyle *style = new QGtkStyle();
    QApplication::setStyle(style);
    app.setWindowIcon(QPixmap("gfx/iconlogo2_32.png"));

    //    printf("Username found: %s\n", MainWindow::getUsername().toAscii().constData());
    MainWindow mainframe;

    return app.exec();

}

