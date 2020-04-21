#include "sketchpage.h"
#include "sketchdocument.h"
#include <QGuiApplication>
#include <QJsonArray>
#include <QScreen>

namespace {

int defaultThumbnailWidth()
{
    const auto scaleFactor = 0.35;
    const auto screenSize = QGuiApplication::primaryScreen()->size();
    return screenSize.width() * scaleFactor;
}

float artboardRatio(const SketchArtboard *artboard)
{
    return artboard->height() / float(artboard->width());
}

float maxArtboardRatio(const QList<SketchArtboard *> artboards)
{
    float maxRatio = 0.0;

    for (const auto artboard : artboards) {
        const auto ratio = artboardRatio(artboard);
        if (ratio > maxRatio) {
            maxRatio = ratio;
        }
    }

    return maxRatio;
}

} // namespace

SketchPage::SketchPage(const QJsonObject &data, SketchDocument *document)
    : QQmlObjectListModel(document, QByteArrayLiteral("uid"))
    , m_id(data["id"].toString())
    , m_name(data["name"].toString())
    , m_currentIndex(-1)
    , m_thumbnailWidth(defaultThumbnailWidth())
    , m_thumbnailHeight(m_thumbnailWidth)
    , m_minThumbnailHeight(m_thumbnailHeight)
    , m_maxThumbnailHeight(m_thumbnailHeight)
    , m_document(document)
{
    qDebug() << "Loading page" << m_name;

    QList<SketchArtboard *> sketchArtboards;
    const auto artboards = data["artboards"].toArray();

    for (const auto &artboard : artboards) {
        sketchArtboards.append(new SketchArtboard(artboard.toObject(), this));
    }

    append(sketchArtboards);
    updateThumbnailHeight();
}

void SketchPage::update(const QJsonObject &data)
{
    const auto name = data["name"].toString();
    if (m_name != name) {
        m_name = name;
        emit nameChanged();
    }

    const auto artboards = data["artboards"].toArray();

    QMap<QString, QJsonObject> artboardsMap;
    for (const auto &artboard : artboards) {
        const auto object = artboard.toObject();
        const auto id = object.value("id").toString();
        artboardsMap.insert(id, object);
    }

    // Remove non-existing artboards
    QList<SketchArtboard *> artboardsToRemove;
    for (const auto artboard : toList()) {
        if (!artboardsMap.contains(artboard->id())) {
            artboardsToRemove.append(artboard);
        }
    }
    for (auto artboard : artboardsToRemove) {
        artboard->deleteLater();
        remove(artboard);
    }
    bool artboardsRemoved = !artboardsToRemove.empty();

    // Update existing artboards. Note that only artboard information is updated
    // on manifest change, artboard content updates are handled separately.
    for (const auto artboard : toList()) {
        const auto id = artboard->id();
        artboard->updateInfo(artboardsMap[id]);
        artboardsMap.remove(id);
    }

    // Insert new artboards
    bool artboardsAdded = false;
    for (int i = 0; i < artboards.size(); ++i) {
        const auto artboard = artboards.at(i);
        const auto object = artboard.toObject();
        const auto id = object.value("id").toString();
        if (!contains(id)) {
            insert(i, new SketchArtboard(object, this));
            artboardsAdded = true;
        }
    }

    // FIXME: artboards reordering is not handled

    if (artboardsRemoved || artboardsAdded) {
        updateThumbnailHeight();
    }
}

void SketchPage::updateArtboard(const QString &id, const QJsonObject &data)
{
    const auto artboard = getByUid(id);
    if (artboard == nullptr)
        return;

    // FIXME: thumbnailHeight should be updated as well, but this
    // might be done only after moving to custom artboards cache,
    // as data contains no information about actual artboard size.
    artboard->updateContent(data);
}

void SketchPage::setCurrentArtboard(const QString &id)
{
    const auto artboard = getByUid(id);
    if (artboard == nullptr)
        return;

    m_currentIndex = indexOf(artboard);
    emit currentIndexChanged();
}

const SketchServer *SketchPage::server() const
{
    return m_document->server();
}

QString SketchPage::id() const
{
    return m_id;
}

QString SketchPage::name() const
{
    return m_name;
}

int SketchPage::currentIndex() const
{
    return m_currentIndex;
}

int SketchPage::thumbnailWidth() const
{
    return m_thumbnailWidth;
}

void SketchPage::setThumbnailWidth(int width)
{
    if (m_thumbnailWidth == width)
        return;

    m_thumbnailWidth = width;
    updateThumbnailHeight();
    emit thumbnailWidthChanged();
}

int SketchPage::thumbnailHeight() const
{
    return m_thumbnailHeight;
}

int SketchPage::minThumbnailHeight() const
{
    return m_minThumbnailHeight;
}

void SketchPage::setMinThumbnailHeight(int height)
{
    if (m_minThumbnailHeight == height)
        return;

    m_minThumbnailHeight = height;
    updateThumbnailHeight();
    emit minThumbnailHeightChanged();
}

int SketchPage::maxThumbnailHeight() const
{
    return m_maxThumbnailHeight;
}

void SketchPage::setMaxThumbnailHeight(int height)
{
    if (m_maxThumbnailHeight == height)
        return;

    m_maxThumbnailHeight = height;
    updateThumbnailHeight();
    emit maxThumbnailHeightChanged();
}

void SketchPage::setThumbnailHeightRange(int min, int max)
{
    if (m_minThumbnailHeight == min && m_maxThumbnailHeight == max)
        return;

    m_minThumbnailHeight = min;
    m_maxThumbnailHeight = max;
    updateThumbnailHeight();
    emit minThumbnailHeightChanged();
    emit maxThumbnailHeightChanged();
}

void SketchPage::updateThumbnailHeight()
{
    float minRatio = m_minThumbnailHeight / float(m_thumbnailWidth);
    float maxRatio = m_maxThumbnailHeight / float(m_thumbnailWidth);
    float desiredRatio = maxArtboardRatio(toList());
    float ratio = qBound(minRatio, desiredRatio, maxRatio);
    m_thumbnailHeight = m_thumbnailWidth * ratio;
    emit thumbnailHeightChanged();
}
