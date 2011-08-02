/**
 * @file settingsdialog.h
 * @author Bernd Wachter <bwachter@lart.info>
 * @date 2011
 */

#ifndef _SETTINGSDIALOG_H
#define _SETTINGSDIALOG_H

#include <QtGui>
#include "ui_settingsdialog.h"

class SettingsDialog: public QDialog, private Ui::SettingsDialog {
    Q_OBJECT

        public:
    SettingsDialog();

  private:
    QSettings settings;

    public slots:

    private slots:
    void accept();

  signals:
    void configurationChanged();
};


#endif
