/*
    Copyright (C) 2020-2021 Andrea Zanellato <redtid3@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    For a full copy of the GNU General Public License see the LICENSE file
*/
#include "dialogprefs.hpp"
#include "ui_dialogprefs.h"

#include <QIcon>
#include <QPushButton>
#include <QSettings>

DialogPrefs::DialogPrefs(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::DialogPrefs)
{
    ui->setupUi(this);
    ui->gbxCursor->setLayout(ui->layCursor);
    setLayout(ui->layout);

    connect(ui->buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, this, &DialogPrefs::saveSettings);

    setWindowIcon(QIcon::fromTheme("preferences-system"));
    setWindowTitle(tr("Preferences"));

    loadSettings();
}

DialogPrefs::~DialogPrefs()
{
    delete ui;
}

void DialogPrefs::onOkClicked()
{
    saveSettings();
}

void DialogPrefs::loadSettings()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       QApplication::organizationName(),
                       QApplication::applicationDisplayName());

    settings.beginGroup("Main");
    int value = settings.value("CursorSize", 1).toUInt();
    if (value < 1 || value > 100)
        value = 1;

    ui->cbxCursorFilled->setChecked(settings.value("CursorFilled", false).toBool());
    ui->spinBox->setValue(value);
    settings.endGroup();
}

void DialogPrefs::saveSettings()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       QApplication::organizationName(),
                       QApplication::applicationDisplayName());

    settings.beginGroup("Main");
    settings.setValue("CursorSize", ui->spinBox->value());
    settings.setValue("CursorFilled", ui->cbxCursorFilled->isChecked());
    settings.endGroup();
    emit sigPrefsChanged();
}
