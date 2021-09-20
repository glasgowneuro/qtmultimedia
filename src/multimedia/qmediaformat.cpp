/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
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

#include "qmediaformat.h"
#include "private/qplatformmediaintegration_p.h"
#include "private/qplatformmediaformatinfo_p.h"
#include <QtCore/qmimedatabase.h>

QT_BEGIN_NAMESPACE

/*!
    \class QMediaFormat
    \ingroup multimedia
    \inmodule QtMultimedia
    \brief Describes an encoding format for a multimedia file or stream.
    \since 6.2

    QMediaFormat describes an encoding format for a multimedia file or stream.

    You can check whether a certain media format can be used for encoding
    or decoding using QMediaFormat.
*/

/*!
    \qmltype MediaFormat
    \since 6.2
    \instantiates QMediaFormat
    \brief MediaFormat describes the format of a media file.
    \inqmlmodule QtMultimedia
    \ingroup multimedia_qml

    The MediaFormat type describes the format of a media file. It contains
    three properties that describe the file type and the audio and video codecs
    that are being used.

    MediaFormat can be used to specify the type of file that should be created
    by a MediaRecorder. The snippet below shows an example that sets up the
    recorder to create an mpeg4 video with AAC encoded audio and H265 video:

    \qml
    CaptureSession {
        ... // setup inputs
        MediaRecorder {
            mediaFormat: MediaFormat {
                fileFormat: mediaFormat.FileFormat.MPEG4
                audioCodec: mediaFormat.AudioCodec.AAC
                videoCodec: mediaFormat.VideoCodec.H265
            }
        }
    }
    \endqml

    If the specified MediaFormat is not supported, the MediaRecorder will automatically try
    to find the best possible replacement format and use that instead.

    \sa MediaRecorder, CaptureSession
*/

namespace {

const char *mimeTypeForFormat[QMediaFormat::LastFileFormat + 2] =
{
    "",
    "video/x-ms-wmv",
    "video/x-msvideo",
    "video/x-matroska",
    "video/mp4",
    "video/ogg",
    "video/quicktime",
    "video/webm",
    // Audio Formats
    "audio/mp4",
    "audio/aac",
    "audio/x-ms-wma",
    "audio/flac",
    "audio/mpeg",
    "audio/wav",
};

constexpr QMediaFormat::FileFormat videoFormatPriorityList[] =
{
    QMediaFormat::MPEG4,
    QMediaFormat::QuickTime,
    QMediaFormat::AVI,
    QMediaFormat::WebM,
    QMediaFormat::WMV,
    QMediaFormat::Matroska,
    QMediaFormat::Ogg,
    QMediaFormat::UnspecifiedFormat
};

constexpr QMediaFormat::FileFormat audioFormatPriorityList[] =
{
    QMediaFormat::Mpeg4Audio,
    QMediaFormat::MP3,
    QMediaFormat::WMA,
    QMediaFormat::FLAC,
    QMediaFormat::Wave,
    QMediaFormat::UnspecifiedFormat
};

constexpr QMediaFormat::AudioCodec audioPriorityList[] =
{
    QMediaFormat::AudioCodec::AAC,
    QMediaFormat::AudioCodec::MP3,
    QMediaFormat::AudioCodec::AC3,
    QMediaFormat::AudioCodec::Opus,
    QMediaFormat::AudioCodec::EAC3,
    QMediaFormat::AudioCodec::DolbyTrueHD,
    QMediaFormat::AudioCodec::WMA,
    QMediaFormat::AudioCodec::FLAC,
    QMediaFormat::AudioCodec::Vorbis,
    QMediaFormat::AudioCodec::Wave,
    QMediaFormat::AudioCodec::Unspecified
};

constexpr QMediaFormat::VideoCodec videoPriorityList[] =
{
    QMediaFormat::VideoCodec::H265,
    QMediaFormat::VideoCodec::VP9,
    QMediaFormat::VideoCodec::H264,
    QMediaFormat::VideoCodec::AV1,
    QMediaFormat::VideoCodec::VP8,
    QMediaFormat::VideoCodec::WMV,
    QMediaFormat::VideoCodec::Theora,
    QMediaFormat::VideoCodec::MPEG4,
    QMediaFormat::VideoCodec::MPEG2,
    QMediaFormat::VideoCodec::MPEG1,
    QMediaFormat::VideoCodec::MotionJPEG,
};

}

class QMediaFormatPrivate : public QSharedData
{};

QT_DEFINE_QESDP_SPECIALIZATION_DTOR(QMediaFormatPrivate);

/*! \enum QMediaFormat::FileFormat

    Describes the container format used in a multimedia file or stream.

    \value WMA
        \l {Windows Media Audio}
    \value AAC
        \l{Advanced Audio Coding}
    \value Matroska
        \l{Matroska (MKV)}
    \value WMV
        \l{Windows Media Video}
    \value MP3
        \l{MPEG-1 Audio Layer III or MPEG-2 Audio Layer III}
    \value Wave
        \l{Waveform Audio File Format}
    \value Ogg
        \l{Ogg}
    \value MPEG4
        \l{MPEG-4}
    \value AVI
        \l{Audio Video Interleave}
    \value QuickTime
        \l{QuickTime}
    \value WebM
        \l{WebM}
    \value Mpeg4Audio
        \l{MPEG-4 Part 3 or MPEG-4 Audio (formally ISO/IEC 14496-3)}
    \value FLAC
        \l{Free Lossless Audio Codec}
    \value UnspecifiedFormat
        The format is unspecified.

    \omitvalue LastFileFormat
*/

/*! \qmlproperty enumeration QtMultiMedia::MediaFormat::fileFormat

    Describes the container format used in a multimedia file or stream.
    It can take one of the following values:

    \table
    \header \li Property value
            \li Description
    \row \li WMA
        \li \l{Windows Media Audio}
    \row \li AAC
        \li \l{Advanced Audio Coding}
    \row \li Matroska
        \li \l{Matroska (MKV)}
    \row \li WMV
        \li \l{Windows Media Video}
    \row \li MP3
        \li \l{MPEG-1 Audio Layer III or MPEG-2 Audio Layer III}
    \row \li Wave
        \li \l{Waveform Audio File Format}
    \row \li Ogg
        \li \l{Ogg}
    \row \li MPEG4
        \li \l{MPEG-4}
    \row \li AVI
        \li \l{Audio Video Interleave}
    \row \li QuickTime
        \li \l{QuickTime}
    \row \li WebM
        \li \l{WebM}
    \row \li Mpeg4Audio
        \li \l{MPEG-4 Part 3 or MPEG-4 Audio (formally ISO/IEC 14496-3)}
    \row \li FLAC
        \li \l{Free Lossless Audio Codec}
    \row \li UnspecifiedFormat
        \li The format is unspecified.
    \endtable
*/

/*! \enum QMediaFormat::AudioCodec

    Describes the audio codec used in multimedia file or stream.

    \value WMA
        \l {Windows Media Audio}
    \value AC3
        \l {Dolby Digital}
    \value AAC
        \l{Advanced Audio Coding}
    \value ALAC
        \l{Apple Lossless Audio Codec}
    \value DolbyTrueHD
        \l{Dolby TrueHD}
    \value EAC3
        \l {Dolby Digital Plus (EAC3)}
    \value MP3
        \l{MPEG-1 Audio Layer III or MPEG-2 Audio Layer III}
    \value Wave
        \l{Waveform Audio File Format}
    \value Vorbis
        \l{Ogg Vorbis}
    \value FLAC
        \l{Free Lossless Audio Codec}
    \value Opus
        \l{Opus Audio Format}
    \value Unspecified
        Unspecified codec

    \omitvalue LastAudioCodec
*/

/*! \qmlproperty enumeration QMediaFormat::audioCodec

    Describes the audio codec used in multimedia file or stream.
    It can take one of the following values:

    \table
    \header \li Property value
            \li Description
    \row \li WMA
        \li \l {Windows Media Audio}
    \row \li AC3
        \li \l {Dolby Digital}
    \row \li AAC
        \li \l{Advanced Audio Coding}
    \row \li ALAC
        \li \l{Apple Lossless Audio Codec}
    \row \li DolbyTrueHD
        \li \l{Dolby TrueHD}
    \row \li EAC3
        \li \l {Dolby Digital Plus (EAC3)}
    \row \li MP3
        \li \l{MPEG-1 Audio Layer III or MPEG-2 Audio Layer III}
    \row \li Wave
        \li \l{Waveform Audio File Format}
    \row \li Vorbis
        \li \l{Ogg Vorbis}
    \row \li FLAC
        \li \l{Free Lossless Audio Codec}
    \row \li Opus
        \li \l{Opus Audio Format}
    \row \li Unspecified
        \li Unspecified codec
    \endtable
*/

/*! \enum QMediaFormat::VideoCodec

    Describes the video coded used in multimedia file or stream.

    \value VP8
        \l{VP8}
    \value MPEG2
        \l{MPEG-2}
    \value MPEG1
        \l{MPEG-1}
    \value WMV
        \l{Windows Media Video}
    \value H265
        \l{High Efficiency Video Coding (HEVC)}
    \value H264
        \l{Advanced Video Coding}
    \value MPEG4
        \l{MPEG-4}
    \value AV1
        \l{AOMedia Video 1}
    \value MotionJPEG
        \l{MotionJPEG}
    \value VP9
        \l{VP9}
    \value Theora
        \l{Theora}
    \value Unspecified
        Video codec not specified

    \omitvalue LastVideoCodec
*/

/*! \qmlproperty QtMultimedia::MediaFormat::videoCodec

    Describes the video codec used in multimedia file or stream.
    It can take one of the following values:

    \table
    \header \li Property value
            \li Description
    \row \li VP8
        \li \l{VP8}
    \row \li MPEG2
        \li \l{MPEG-2}
    \row \li MPEG1
        \li \l{MPEG-1}
    \row \li WMV
        \li \l{Windows Media Video}
    \row \li H265
        \li \l{High Efficiency Video Coding (HEVC)}
    \row \li H264
        \li \l{Advanced Video Coding}
    \row \li MPEG4
        \li \l{MPEG-4}
    \row \li AV1
        \li \l{AOMedia Video 1}
    \row \li MotionJPEG
        \li \l{MotionJPEG}
    \row \li VP9
        \li \l{VP9}
    \row \li Theora
        \li \l{Theora}
    \row \li Unspecified
        \li Video codec not specified
    \endtable
*/

// these are non inline to make a possible future addition of a d pointer binary compatible

/*!
    Constructs a QMediaFormat object for \a format.
*/
QMediaFormat::QMediaFormat(FileFormat format)
    : fmt(format)
{
}

/*!
    Destroys the QMediaFormat object.
*/
QMediaFormat::~QMediaFormat() = default;

/*!
    Constructs a QMediaFormat object by copying from \a other.
*/
QMediaFormat::QMediaFormat(const QMediaFormat &other) noexcept = default;

/*!
    Copies \a other into this QMediaFormat object.
*/
QMediaFormat &QMediaFormat::operator=(const QMediaFormat &other) noexcept = default;

/*! \fn QMediaFormat::QMediaFormat(QMediaFormat &&other)

    Constructs a QMediaFormat objects by moving from \a other.
*/

/*! \fn QMediaFormat &QMediaFormat::operator=(QMediaFormat &&other)

    Moves \a other into this QMediaFormat objects.
*/

// Properties
/*! \qmlproperty QtMultimedia::MediaFormat::fileFormat

    The file (container) format of the media.
*/

/*! \property QMediaFormat::fileFormat

    \brief The file (container) format of the media.

    \sa QMediaFormat::FileFormat
*/

/*! \qmlproperty QtMultimedia::MediaFormat::audioCodec

    The audio codec of the media.
*/

/*! \property QMediaFormat::audioCodec

    \brief The audio codec of the media.

    \sa QMediaFormat::AudioCodec
*/

/*! \qmlproperty QtMultimedia::MediaFormat::videoCodec

    The video codec of the media.
*/

/*! \property QMediaFormat::videoCodec

    \brief The video codec of the media.

    \sa QMediaFormat::VideoCodec
*/

/*! \fn void QMediaFormat::setVideoCodec(VideoCodec codec)

    Sets the video codec to \a codec.

    \sa videoCodec(), QMediaFormat::VideoCodec
*/

/*! \fn QMediaFormat::VideoCodec QMediaFormat::videoCodec() const

    Returns the video codec used in this format.

    \sa setVideoCodec(), QMediaFormat::VideoCodec
*/

/*! \fn void QMediaFormat::setAudioCodec(AudioCodec codec)

    Sets the audio codec to \a codec.

    \sa audioCodec(), QMediaFormat::AudioCodec
*/

/*! \fn QMediaFormat::AudioCodec QMediaFormat::audioCodec() const

    Returns the audio codec used in this format.

    \sa setAudioCodec(), QMediaFormat::AudioCodec
*/

/*!
    Returns \c true if Qt Multimedia can encode or decode this format,
    depending on \a mode.
*/

bool QMediaFormat::isSupported(ConversionMode mode) const
{
    return QPlatformMediaIntegration::instance()->formatInfo()->isSupported(*this, mode);
}

/*!
    Returns the \l{MIME type} for the file format used in this media format.
*/

QMimeType QMediaFormat::mimeType() const
{
    return QMimeDatabase().mimeTypeForName(QString::fromLatin1(mimeTypeForFormat[fmt + 1]));
}

static QPlatformMediaFormatInfo *formatInfo()
{
    QPlatformMediaFormatInfo *result = nullptr;
    if (auto *pi = QPlatformMediaIntegration::instance())
        result = pi->formatInfo();
    return result;
}

/*!
    \enum QMediaFormat::ConversionMode

    In many cases, systems have asymmetric capabilities and can often decode more formats
    or codecs than can be encoded. This enum describes the requested conversion mode to
    be used when checking whether a certain file format or codec is supported.

    \value Encode
        Used to check whether a certain file format or codec can be encoded.
    \value Decode
        Used to check whether a certain file format or codec can be decoded.

    \sa supportedFileFormats, supportedAudioCodecs, supportedVideoCodecs
*/

/*!
    \qmlmethod list<FileFormat> QtMultimedia::MediaFormat::supportedFileFormats(conversionMode)
    Returns a list of file formats for the audio and video
    codec indicated by \a{conversionMode}.

    To get all supported file formats, run this query on a default constructed MediaFormat. To
    get a list of file formats supporting a specific combination of an audio and video codec,
    you can set the audioCodec and videoCodec properties before running this query.

    \sa QMediaFormat::ConversionMode
*/

/*!
    Returns a list of file formats for the audio and video
    codec indicated by \a{m}.

    To get all supported file formats, run this query on a default constructed
    QMediaFormat.

    \sa QMediaFormat::ConversionMode
*/
QList<QMediaFormat::FileFormat> QMediaFormat::supportedFileFormats(QMediaFormat::ConversionMode m)
{
    auto *fi = formatInfo();
    return fi != nullptr ? fi->supportedFileFormats(*this, m) : QList<QMediaFormat::FileFormat>{};
}

/*!
    \qmlmethod list<VideoCodec> QtMultimedia::MediaFormat::supportedVideoCodecs(conversionMode)
    Returns a list of video codecs for the chosen file format and
    audio codec (\a conversionMode).

    To get all supported video codecs, run this query on a default constructed MediaFormat. To
    get a list of supported video codecs for a specific combination of a file format and an audio
    codec, you can set the fileFormat and audioCodec properties before running this query.

    \sa QMediaFormat::ConversionMode
*/

/*!
    Returns a list of video codecs for the chosen file format and
    audio codec (\a m).

    To get all supported video codecs, run this query on a default constructed
    MediaFormat.

    \sa QMediaFormat::ConversionMode
*/
QList<QMediaFormat::VideoCodec> QMediaFormat::supportedVideoCodecs(QMediaFormat::ConversionMode m)
{
    auto *fi = formatInfo();
    return fi != nullptr ? fi->supportedVideoCodecs(*this, m) : QList<QMediaFormat::VideoCodec>{};
}

/*!
    \qmlmethods list<AudioCodec> QtMultimedia::MediaFormat::supportedAudioFormats(conversionMode)
    Returns a list of audio codecs for the chosen file format and
    video codec (\a conversionMode).

    To get all supported audio codecs, run this query on a default constructed MediaFormat. To get
    a list of supported audio codecs for a specific combination of a file format and a video codec,
    you can set the fileFormat and videoCodec properties before running this query.

    \sa QMediaFormat::ConversionMode
*/

/*!
    Returns a list of audio codecs for the chosen file format and
    video codec (\a m).

    To get all supported audio codecs, run this query on a default constructed
    QMediaFormat.

    \sa QMediaFormat::ConversionMode
*/
QList<QMediaFormat::AudioCodec> QMediaFormat::supportedAudioCodecs(QMediaFormat::ConversionMode m)
{
    auto *fi = formatInfo();
    return fi != nullptr ? fi->supportedAudioCodecs(*this, m) : QList<QMediaFormat::AudioCodec>{};
}

/*!
    \qmlmethods QtMultimedia::MediaFormat::fileFormatName(fileFormat)
    Returns a string based name for \a fileFormat.
*/

/*!
    Returns a string based name for \a fileFormat.
*/
QString QMediaFormat::fileFormatName(QMediaFormat::FileFormat fileFormat)
{
    constexpr const char *descriptions[QMediaFormat::LastFileFormat + 2] = {
        "Unspecified",
        "WMV",
        "AVI",
        "Matroska",
        "MPEG-4",
        "Ogg",
        "QuickTime",
        "WebM",
        // Audio Formats
        "MPEG-4 Audio",
        "AAC",
        "WMA",
        "MP3",
        "FLAC",
        "Wave"
    };
    return QString::fromUtf8(descriptions[int(fileFormat) + 1]);
}

/*!
    \qmlmethods QtMultimedia::MediaFormat::audioCodecName(codec)
    Returns a string based name for \a codec.
*/

/*!
    Returns a string based name for \a codec.
*/
QString QMediaFormat::audioCodecName(QMediaFormat::AudioCodec codec)
{
    constexpr const char *descriptions[] = {
        "Invalid",
        "MP3",
        "AAC",
        "AC3",
        "EAC3",
        "FLAC",
        "DolbyTrueHD",
        "Opus",
        "Vorbis",
        "Wave",
        "WMA",
        "ALAC",
    };
    return QString::fromUtf8(descriptions[int(codec) + 1]);
}

/*!
    \qmlmethods QtMultimedia::MediaFormat::videoCodecName(codec)
    Returns a string based name for \a codec.
*/

/*!
    Returns a string based name for \a codec.
*/
QString QMediaFormat::videoCodecName(QMediaFormat::VideoCodec c)
{
    constexpr const char *descriptions[] = {
        "Invalid",
        "MPEG1",
        "MPEG2",
        "MPEG4",
        "H264",
        "H265",
        "VP8",
        "VP9",
        "AV1",
        "Theora",
        "WMV",
        "MotionJPEG"
    };
    return QString::fromUtf8(descriptions[int(c) + 1]);
}

/*!
    \qmlmethods QtMultimedia::MediaFormat::fileFormatDescription(fileFormat)
    Returns a description for \a fileFormat.
*/

/*!
    Returns a description for \a fileFormat.
*/
QString QMediaFormat::fileFormatDescription(QMediaFormat::FileFormat fileFormat)
{
    constexpr const char *descriptions[QMediaFormat::LastFileFormat + 2] = {
        "Unspecified File Format",
        "Windows Media Video",
        "Audio Video Interleave",
        "Matroska Multimedia Container",
        "MPEG-4 Video Container",
        "Ogg",
        "QuickTime Container",
        "WebM",
        // Audio Formats
        "MPEG-4 Audio",
        "AAC",
        "Windows Media Audio",
        "MP3",
        "Free Lossless Audio Codec (FLAC)",
        "Wave File"
    };
    return QString::fromUtf8(descriptions[int(fileFormat) + 1]);
}

/*!
    \qmlmethods QtMultimedia::MediaFormat::audioCodecDescription(codec)
    Returns a description for \a codec.
*/

/*!
    Returns a description for \a codec.
*/
QString QMediaFormat::audioCodecDescription(QMediaFormat::AudioCodec codec)
{
    constexpr const char *descriptions[] = {
        "Unspecified Audio Codec",
        "MP3",
        "Advanced Audio Codec (AAC)",
        "Dolby Digital (AC3)",
        "Dolby Digital Plus (E-AC3)",
        "Free Lossless Audio Codec (FLAC)",
        "Dolby True HD",
        "Opus",
        "Vorbis",
        "Wave",
        "Windows Media Audio",
        "Apple Lossless Audio Codec (ALAC)",
    };
    return QString::fromUtf8(descriptions[int(codec) + 1]);
}

/*!
    \qmlmethods QtMultimedia::MediaFormat::videoCodecDescription(codec)
    Returns a description for \a codec.
*/

/*!
    Returns a description for \a codec.
*/
QString QMediaFormat::videoCodecDescription(QMediaFormat::VideoCodec codec)
{
    constexpr const char *descriptions[] = {
        "Unspecified Video Codec",
        "MPEG-1 Video",
        "MPEG-2 Video",
        "MPEG-4 Video",
        "H.264",
        "H.265",
        "VP8",
        "VP9",
        "AV1",
        "Theora",
        "Windows Media Video",
        "MotionJPEG"
    };
    return QString::fromUtf8(descriptions[int(codec) + 1]);
}

bool QMediaFormat::operator==(const QMediaFormat &other) const
{
    Q_ASSERT(!d);
    return fmt == other.fmt &&
            audio == other.audio &&
           video == other.video;
}

/*!
    \enum QMediaFormat::ResolveFlags

    Describes the requirements for resolving a suitable format for
    QMediaRecorder.

    \value NoFlags
           No requirements
    \value RequiresVideo
           A video codec is required

    \sa resolveForEncoding()
*/

/*!
    Resolves the format, based on \a flags, to a format that is supported by
    QMediaRecorder.

    This method tries to find the best possible match for unspecified settings.
    Settings that are not supported by the recorder will be modified to the closest
    match that is supported.

    When resolving, priority is given in the following order:
    \list 1
    \li File format
    \li Video codec
    \li Audio codec
    \endlist
*/
void QMediaFormat::resolveForEncoding(ResolveFlags flags)
{
    const bool requiresVideo = (flags & ResolveFlags::RequiresVideo) != 0;

    if (!requiresVideo)
        video = VideoCodec::Unspecified;

    // need to adjust the format. Priority is given first to file format, then video codec, then audio codec

    QMediaFormat nullFormat;
    auto supportedFormats = nullFormat.supportedFileFormats(QMediaFormat::Encode);
    auto supportedAudioCodecs = nullFormat.supportedAudioCodecs(QMediaFormat::Encode);
    auto supportedVideoCodecs = nullFormat.supportedVideoCodecs(QMediaFormat::Encode);

    auto bestSupportedFileFormat = [&](QMediaFormat::AudioCodec audio = QMediaFormat::AudioCodec::Unspecified,
                                       QMediaFormat::VideoCodec video = QMediaFormat::VideoCodec::Unspecified)
    {
        QMediaFormat f;
        f.setAudioCodec(audio);
        f.setVideoCodec(video);
        auto supportedFormats = f.supportedFileFormats(QMediaFormat::Encode);
        auto *list = (flags == NoFlags) ? audioFormatPriorityList : videoFormatPriorityList;
        while (*list != QMediaFormat::UnspecifiedFormat) {
            if (supportedFormats.contains(*list))
                break;
            ++list;
        }
        return *list;
    };

    // reset format if it does not support video when video is required
    if (requiresVideo && this->supportedVideoCodecs(QMediaFormat::Encode).isEmpty())
        fmt = QMediaFormat::UnspecifiedFormat;

    // reset non supported formats and codecs
    if (!supportedFormats.contains(fmt))
        fmt = QMediaFormat::UnspecifiedFormat;
    if (!supportedAudioCodecs.contains(audio))
        audio = QMediaFormat::AudioCodec::Unspecified;
    if (!requiresVideo || !supportedVideoCodecs.contains(video))
        video = QMediaFormat::VideoCodec::Unspecified;

    if (requiresVideo) {
        // try finding a file format that is supported
        if (fmt == QMediaFormat::UnspecifiedFormat)
            fmt = bestSupportedFileFormat(audio, video);
        // try without the audio codec
        if (fmt == QMediaFormat::UnspecifiedFormat)
            fmt = bestSupportedFileFormat(QMediaFormat::AudioCodec::Unspecified, video);
    }
    // try without the video codec
    if (fmt == QMediaFormat::UnspecifiedFormat)
        fmt = bestSupportedFileFormat(audio);
    // give me a format that's supported
    if (fmt == QMediaFormat::UnspecifiedFormat)
        fmt = bestSupportedFileFormat();
    // still nothing? Give up
    if (fmt == QMediaFormat::UnspecifiedFormat) {
        *this = {};
        return;
    }

    // find a working video codec
    if (requiresVideo) {
        // reset the audio codec, so that we won't throw away the video codec
        // if it is supported (choosing the specified video codec has higher
        // priority than the specified audio codec)
        auto a = audio;
        audio = QMediaFormat::AudioCodec::Unspecified;
        auto videoCodecs = this->supportedVideoCodecs(QMediaFormat::Encode);
        if (!videoCodecs.contains(video)) {
            // not supported, try to find a replacement
            auto *list = videoPriorityList;
            while (*list != QMediaFormat::VideoCodec::Unspecified) {
                if (videoCodecs.contains(*list))
                    break;
                ++list;
            }
            video = *list;
        }
        audio = a;
    } else {
        video = QMediaFormat::VideoCodec::Unspecified;
    }

    // and a working audio codec
    auto audioCodecs = this->supportedAudioCodecs(QMediaFormat::Encode);
    if (!audioCodecs.contains(audio)) {
        auto *list = audioPriorityList;
        while (*list != QMediaFormat::AudioCodec::Unspecified) {
            if (audioCodecs.contains(*list))
                break;
            ++list;
        }
        audio = *list;
    }
}

QT_END_NAMESPACE
