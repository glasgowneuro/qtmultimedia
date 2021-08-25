/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef AVFCAMERARENDERER_H
#define AVFCAMERARENDERER_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qobject.h>
#include <QtMultimedia/qvideoframe.h>
#include <QtCore/qmutex.h>
#include <private/avfvideosink_p.h>

#include <CoreVideo/CVBase.h>
#include <CoreVideo/CVPixelBuffer.h>
#include <CoreVideo/CVImageBuffer.h>
#ifdef Q_OS_IOS
#include <CoreVideo/CVOpenGLESTexture.h>
#include <CoreVideo/CVOpenGLESTextureCache.h>
#endif

#include <dispatch/dispatch.h>

Q_FORWARD_DECLARE_OBJC_CLASS(AVFCaptureFramesDelegate);
Q_FORWARD_DECLARE_OBJC_CLASS(AVCaptureVideoDataOutput);

QT_BEGIN_NAMESPACE

class AVFCameraSession;
class AVFCameraService;
class AVFCameraRenderer;
class AVFVideoSink;

class AVFCameraRenderer : public QObject, public AVFVideoSinkInterface
{
Q_OBJECT
public:
    AVFCameraRenderer(QObject *parent = nullptr);
    ~AVFCameraRenderer();

    void reconfigure() override;
    void setRhi(QRhi *rhi) override;

    void configureAVCaptureSession(AVFCameraSession *cameraSession);
    void syncHandleViewfinderFrame(const QVideoFrame &frame);

    AVCaptureVideoDataOutput *videoDataOutput() const;

    AVFCaptureFramesDelegate *captureDelegate() const;
    void resetCaptureDelegate() const;

    QRhi *rhi() const { return m_rhi; }

    void setPixelFormat(const QVideoFrameFormat::PixelFormat format);

Q_SIGNALS:
    void newViewfinderFrame(const QVideoFrame &frame);

private Q_SLOTS:
    void handleViewfinderFrame();
    void updateCaptureConnection();

private:
    AVFCaptureFramesDelegate *m_viewfinderFramesDelegate = nullptr;
    AVFCameraSession *m_cameraSession = nullptr;
    AVCaptureVideoDataOutput *m_videoDataOutput = nullptr;

    bool m_needsHorizontalMirroring = false;

#ifdef Q_OS_IOS
    CVOpenGLESTextureCacheRef m_textureCache = nullptr;
#endif

    QVideoFrame m_lastViewfinderFrame;
    QMutex m_vfMutex;
    dispatch_queue_t m_delegateQueue;
    QRhi *m_rhi = nullptr;

    friend class CVImageVideoBuffer;
};

QT_END_NAMESPACE

#endif
