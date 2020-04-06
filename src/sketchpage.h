#ifndef SKETCH_PAGE_H
#define SKETCH_PAGE_H

#include "qqmlobjectlistmodel.h"
#include "sketchartboard.h"
#include <QColor>
#include <QJsonObject>
#include <QObject>

class SketchDocument;

class SketchPage : public QQmlObjectListModel<SketchArtboard>
{
    Q_OBJECT
    Q_PROPERTY(QString uid READ id CONSTANT)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(int currentIndex READ currentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(int thumbnailWidth READ thumbnailWidth WRITE setThumbnailWidth
               NOTIFY thumbnailWidthChanged)
    Q_PROPERTY(int thumbnailHeight READ thumbnailHeight NOTIFY thumbnailHeightChanged)
    Q_PROPERTY(int minThumbnailHeight READ minThumbnailHeight WRITE setMinThumbnailHeight
               NOTIFY minThumbnailHeightChanged)
    Q_PROPERTY(int maxThumbnailHeight READ maxThumbnailHeight WRITE setMaxThumbnailHeight
               NOTIFY maxThumbnailHeightChanged)

public:
    SketchPage(const QJsonObject &data, SketchDocument *document);

    void update(const QJsonObject &data);
    void updateArtboard(const QString &id, const QJsonObject &data);
    void setCurrentArtboard(const QString &id);
    const SketchServer *server() const;

    QString id() const;
    QString name() const;

    // Index of artboard currently selected in Sketch
    int currentIndex() const;

    int thumbnailWidth() const;
    void setThumbnailWidth(int width);

    int thumbnailHeight() const;

    int minThumbnailHeight() const;
    void setMinThumbnailHeight(int height);

    int maxThumbnailHeight() const;
    void setMaxThumbnailHeight(int height);

    Q_INVOKABLE void setThumbnailHeightRange(int min, int max);

signals:
    void nameChanged();
    void currentIndexChanged();
    void thumbnailWidthChanged();
    void thumbnailHeightChanged();
    void minThumbnailHeightChanged();
    void maxThumbnailHeightChanged();

private:
    void updateThumbnailHeight();

private:
    QString m_id;
    QString m_name;
    int m_currentIndex;
    int m_thumbnailWidth;
    int m_thumbnailHeight;
    int m_minThumbnailHeight;
    int m_maxThumbnailHeight;
    SketchDocument *m_document;
};

#endif // SKETCH_PAGE_H
