/**
 * @file main.cpp
 * @author Pål Driveklepp <jalla2000@gmail.com>
 * @author Bernd Wachter <bwachter@lart.info>
 * @date 2009-2011
 *
 * @section license_sec License
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

#include "mainwindow.hpp"
#include "spotworker.hpp"

int main(int argc, char *argv[])
{
    qDebug() << "Juniper starting...";
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("Juniper");
    QCoreApplication::setApplicationName("Juniper");
    app.setWindowIcon(QPixmap("gfx/iconlogo2_32.png"));

    MainWindow mainframe;

    return app.exec();
}
