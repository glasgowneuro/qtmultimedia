/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "imagesettings.h"
#include "ui_imagesettings.h"

#include <QComboBox>
#include <QDebug>
#include <QCameraImageCapture>
#include <QCamera>
#include <QMediaCaptureSession>

ImageSettings::ImageSettings(QCameraImageCapture *imageCapture, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImageSettingsUi),
    imagecapture(imageCapture)
{
    ui->setupUi(this);

    //image codecs
    ui->imageCodecBox->addItem(tr("Default image format"), QVariant(QString()));
    const auto supportedImageFormats = QCameraImageCapture::supportedFormats();
    for (const auto &f : supportedImageFormats) {
        QString description = QCameraImageCapture::fileFormatDescription(f);
        ui->imageCodecBox->addItem(QCameraImageCapture::fileFormatName(f) + ": " + description, QVariant::fromValue(f));
    }

    ui->imageQualitySlider->setRange(0, int(QCameraImageCapture::VeryHighQuality));

    ui->imageResolutionBox->addItem(tr("Default Resolution"));
    const QList<QSize> supportedResolutions = imagecapture->captureSession()->camera()->cameraDevice().photoResolutions();
    for (const QSize &resolution : supportedResolutions) {
        ui->imageResolutionBox->addItem(QString("%1x%2").arg(resolution.width()).arg(resolution.height()),
                                        QVariant(resolution));
    }

    selectComboBoxItem(ui->imageCodecBox, QVariant::fromValue(imagecapture->fileFormat()));
    selectComboBoxItem(ui->imageResolutionBox, QVariant(imagecapture->resolution()));
    ui->imageQualitySlider->setValue(imagecapture->quality());
}

ImageSettings::~ImageSettings()
{
    delete ui;
}

void ImageSettings::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void ImageSettings::applyImageSettings() const
{
    imagecapture->setFileFormat(boxValue(ui->imageCodecBox).value<QCameraImageCapture::FileFormat>());
    imagecapture->setQuality(QCameraImageCapture::Quality(ui->imageQualitySlider->value()));
    imagecapture->setResolution(boxValue(ui->imageResolutionBox).toSize());
}

QVariant ImageSettings::boxValue(const QComboBox *box) const
{
    int idx = box->currentIndex();
    if (idx == -1)
        return QVariant();

    return box->itemData(idx);
}

void ImageSettings::selectComboBoxItem(QComboBox *box, const QVariant &value)
{
    for (int i = 0; i < box->count(); ++i) {
        if (box->itemData(i) == value) {
            box->setCurrentIndex(i);
            break;
        }
    }
}
