// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#ifndef QFFMPEGENCODER_P_H
#define QFFMPEGENCODER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists purely as an
// implementation detail. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qffmpegthread_p.h"
#include "qffmpeg_p.h"
#include "qffmpeghwaccel_p.h"

#include <private/qplatformmediarecorder_p.h>
#include <qaudioformat.h>
#include <qaudiobuffer.h>

#include <qqueue.h>

QT_BEGIN_NAMESPACE

class QFFmpegAudioInput;
class QVideoFrame;
class QPlatformVideoSource;

namespace QFFmpeg
{

class Encoder;
class Muxer;
class AudioEncoder;
class VideoEncoder;
class VideoFrameEncoder;

class EncodingFinalizer : public QThread
{
public:
    EncodingFinalizer(Encoder *e);

    void run() override;

private:
    Encoder *encoder = nullptr;
};

class Encoder : public QObject
{
    Q_OBJECT
public:
    Encoder(const QMediaEncoderSettings &settings, const QUrl &url);
    ~Encoder();

    void addAudioInput(QFFmpegAudioInput *input);
    void addVideoSource(QPlatformVideoSource *source);

    void start();
    void finalize();

    void setPaused(bool p);

    void setMetaData(const QMediaMetaData &metaData);

public Q_SLOTS:
    void newAudioBuffer(const QAudioBuffer &buffer);
    void newTimeStamp(qint64 time);

Q_SIGNALS:
    void durationChanged(qint64 duration);
    void error(QMediaRecorder::Error code, const QString &description);
    void finalizationDone();

private:
    // TODO: improve the encasulation
    friend class EncodingFinalizer;
    friend class AudioEncoder;
    friend class VideoEncoder;
    friend class Muxer;

    QMediaEncoderSettings settings;
    QMediaMetaData metaData;
    AVFormatContext *formatContext = nullptr;
    Muxer *muxer = nullptr;
    bool isRecording = false;

    AudioEncoder *audioEncode = nullptr;
    QList<VideoEncoder *> videoEncoders;
    QList<QMetaObject::Connection> connections;

    QMutex timeMutex;
    qint64 timeRecorded = 0;
};


class Muxer : public Thread
{
    mutable QMutex queueMutex;
    QQueue<AVPacket *> packetQueue;
public:
    Muxer(Encoder *encoder);

    void addPacket(AVPacket *);

private:
    AVPacket *takePacket();

    void init() override;
    void cleanup() override;
    bool shouldWait() const override;
    void loop() override;

    Encoder *encoder;
};

class EncoderThread : public Thread
{
public:
    virtual void setPaused(bool b)
    {
        paused.storeRelease(b);
    }

protected:
    QAtomicInteger<bool> paused = false;
    Encoder *encoder = nullptr;
};

class AudioEncoder : public EncoderThread
{
    mutable QMutex queueMutex;
    QQueue<QAudioBuffer> audioBufferQueue;
public:
    AudioEncoder(Encoder *encoder, QFFmpegAudioInput *input, const QMediaEncoderSettings &settings);

    void open();
    void addBuffer(const QAudioBuffer &buffer);

    QFFmpegAudioInput *audioInput() const { return input; }

private:
    QAudioBuffer takeBuffer();
    void retrievePackets();

    void init() override;
    void cleanup() override;
    bool shouldWait() const override;
    void loop() override;

    AVStream *stream = nullptr;
    AVCodecContext *codec = nullptr;
    QFFmpegAudioInput *input;
    QAudioFormat format;

    SwrContext *resampler = nullptr;
    qint64 samplesWritten = 0;
    const AVCodec *avCodec = nullptr;
    QMediaEncoderSettings settings;
};


class VideoEncoder : public EncoderThread
{
    mutable QMutex queueMutex;
    QQueue<QVideoFrame> videoFrameQueue;
    const qsizetype maxQueueSize = 10; // Arbitrarily chosen to limit memory usage (332 MB @ 4K)

public:
    VideoEncoder(Encoder *encoder, const QMediaEncoderSettings &settings,
                 const QVideoFrameFormat &format, std::optional<AVPixelFormat> hwFormat);
    ~VideoEncoder() override;

    void addFrame(const QVideoFrame &frame);

    bool isValid() const;

    void setPaused(bool b) override
    {
        EncoderThread::setPaused(b);
        if (b)
            baseTime.storeRelease(-1);
    }

private:
    QVideoFrame takeFrame();
    void retrievePackets();

    void init() override;
    void cleanup() override;
    bool shouldWait() const override;
    void loop() override;

    VideoFrameEncoder *frameEncoder = nullptr;

    QAtomicInteger<qint64> baseTime = std::numeric_limits<qint64>::min();
    qint64 lastFrameTime = 0;
};

}

QT_END_NAMESPACE

#endif
