#include "sketchdocument.h"
#include "sketchserver.h"
#include <QJsonArray>

namespace {

QColor jsonArrayToColor(const QJsonArray &color)
{
    const auto r = color[0].toDouble();
    const auto g = color[1].toDouble();
    const auto b = color[2].toDouble();
    const auto a = color[3].toDouble();
    return QColor::fromRgbF(r, g, b, a);
}

} // namespace

SketchDocument::SketchDocument(QObject *parent)
    : QQmlObjectListModel(parent, QByteArrayLiteral("uid"))
    , m_id(QStringLiteral("00000000-0000-0000-0000-000000000000"))
    , m_name(QStringLiteral("Empty document"))
{
}

SketchDocument::SketchDocument(const QJsonObject &data, SketchServer *server)
    : QQmlObjectListModel(server, QByteArrayLiteral("uid"))
    , m_id(data["id"].toString())
    , m_name(data["name"].toString())
    , m_flowColor(jsonArrayToColor(data["flowColor"].toArray()))
    , m_currentIndex(-1)
    , m_server(server)
{
    qDebug() << "Loading document" << m_name;

    QList<SketchPage *> sketchPages;

    const auto contents = data["contents"].toObject();
    const auto pages = contents["pages"].toArray();

    for (const auto &page : pages) {
        sketchPages.append(new SketchPage(page.toObject(), this));
    }

    append(sketchPages);
}

void SketchDocument::update(const QJsonObject &data)
{
    const auto name = data["name"].toString();
    if (m_name != name) {
        m_name = name;
        emit nameChanged();
    }

    const auto flowColor = jsonArrayToColor(data["flowColor"].toArray());
    if (m_flowColor != flowColor) {
        m_flowColor = flowColor;
        emit flowColorChanged();
    }

    const auto contents = data["contents"].toObject();
    const auto pages = contents["pages"].toArray();

    QMap<QString, QJsonObject> pagesMap;
    for (const auto &page : pages) {
        const auto object = page.toObject();
        const auto id = object.value("id").toString();
        pagesMap.insert(id, object);
    }

    // Remove non-existing pages
    QList<SketchPage *> pagesToRemove;
    for (const auto page : toList()) {
        if (!pagesMap.contains(page->id())) {
            pagesToRemove.append(page);
        }
    }
    for (auto page : pagesToRemove) {
        page->deleteLater();
        remove(page);
    }

    // Update existing pages
    for (const auto page : toList()) {
        const auto id = page->id();
        page->update(pagesMap[id]);
        pagesMap.remove(id);
    }

    // Insert new pages
    for (int i = 0; i < pages.size(); ++i) {
        const auto page = pages.at(i);
        const auto object = page.toObject();
        const auto id = object.value("id").toString();
        if (!contains(id)) {
            insert(i, new SketchPage(object, this));
        }
    }

    // FIXME: pages reordering is not handled
}

void SketchDocument::updateArtboard(const QString &id, const QJsonObject &data)
{
    for (const auto page : toList()) {
        page->updateArtboard(id, data);
    }
}

void SketchDocument::setCurrentArtboard(const QString &id)
{
    for (const auto page : toList()) {
        if (page->contains(id)) {
            page->setCurrentArtboard(id);
            m_currentIndex = indexOf(page);
            emit currentIndexChanged();
            break;
        }
    }
}

const SketchServer *SketchDocument::server() const
{
    return m_server;
}

QString SketchDocument::id() const
{
    return m_id;
}

QString SketchDocument::name() const
{
    return m_name;
}

QColor SketchDocument::flowColor() const
{
    return m_flowColor;
}

int SketchDocument::currentIndex() const
{
    return m_currentIndex;
}
