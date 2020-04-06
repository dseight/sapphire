#include "sketchartboard.h"
#include "sketchpage.h"
#include "sketchserver.h"
#include <QGuiApplication>
#include <QScreen>

namespace {

float preserveAspectCropScale(int originalWidth,
                              int originalHeight,
                              int desiredWidth,
                              int desiredHeight)
{
    if (originalWidth == 0 || originalHeight == 0)
        return 1.0;

    float wscale = desiredWidth / float(originalWidth);
    float hscale = desiredHeight / float(originalHeight);

    if (wscale > hscale) {
        return wscale;
    } else {
        return hscale;
    }
}

// FIXME: use QDateTime::currentSecsSinceEpoch() when Sailfish OS will
// finnaly have newer Qt.
qint64 currentSecsSinceEpoch()
{
    return QDateTime::currentMSecsSinceEpoch() / 1000;
}

} // namespace

SketchArtboard::SketchArtboard(const QJsonObject &data, SketchPage *page)
    : QObject(page)
    , m_id(data["id"].toString())
    , m_name(data["name"].toString())
    , m_width(data["width"].toInt())
    , m_height(data["height"].toInt())
    , m_timestamp(currentSecsSinceEpoch())
    , m_page(page)
{
}

void SketchArtboard::updateContent(const QJsonObject &data)
{
    // TODO: use data to determine damage area
    Q_UNUSED(data);

    qDebug() << "Updating artboard" << m_name << "content";

    // FIXME: artboard size may have been updated, but we cannot get this
    // information here. Probably need to implement custom artboard cache with
    // image provider.

    m_timestamp = currentSecsSinceEpoch();
    emit sourceChanged();
}

void SketchArtboard::updateInfo(const QJsonObject &data)
{
    const auto name = data["name"].toString();
    if (m_name != name) {
        m_name = name;
        emit nameChanged();
    }
}

const SketchServer *SketchArtboard::server() const
{
    return m_page->server();
}

QString SketchArtboard::id() const
{
    return m_id;
}

QString SketchArtboard::name() const
{
    return m_name;
}

int SketchArtboard::width() const
{
    return m_width;
}

int SketchArtboard::height() const
{
    return m_height;
}

QString SketchArtboard::fullscreenSource() const
{
    const auto screenSize = QGuiApplication::primaryScreen()->size();

    // Fit to width
    const float scale = screenSize.width() / float(m_width);

    QUrlQuery query;
    query.addQueryItem("token", server()->token());
    query.addQueryItem("scale", QString::number(scale));
    query.addQueryItem("t", QString::number(m_timestamp));
    query.addQueryItem("type", "full");

    QUrl url(server()->baseUrl());
    url.setPath(QStringLiteral("/artboards/%1.png").arg(m_id));
    url.setQuery(query);

    return url.toString();
}

QString SketchArtboard::thumbnailSource() const
{
    // Higher quality will lead to low performance on low-end devices
    const float quality = 0.4;

    const float scale = preserveAspectCropScale(m_width,
                                                m_height,
                                                m_page->thumbnailWidth(),
                                                m_page->thumbnailHeight());

    QUrlQuery query;
    query.addQueryItem("token", server()->token());
    query.addQueryItem("scale", QString::number(scale * quality));
    query.addQueryItem("t", QString::number(m_timestamp));
    query.addQueryItem("type", "full");

    QUrl url(server()->baseUrl());
    url.setPath(QStringLiteral("/artboards/%1.png").arg(m_id));
    url.setQuery(query);

    return url.toString();
}
