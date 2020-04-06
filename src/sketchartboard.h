#ifndef SKETCH_ARTBOARD_H
#define SKETCH_ARTBOARD_H

#include "qqmlobjectlistmodel.h"
#include <QColor>
#include <QJsonObject>
#include <QObject>

class SketchPage;
class SketchServer;

class SketchArtboard : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString uid READ id CONSTANT)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(QString fullscreenSource READ fullscreenSource NOTIFY sourceChanged)
    Q_PROPERTY(QString thumbnailSource READ thumbnailSource NOTIFY sourceChanged)
    Q_PROPERTY(int width READ width NOTIFY widthChanged)
    Q_PROPERTY(int height READ height NOTIFY heightChanged)

public:
    SketchArtboard(const QJsonObject &data, SketchPage *page);

    void updateContent(const QJsonObject &data);
    void updateInfo(const QJsonObject &data);
    const SketchServer *server() const;

    QString id() const;
    QString name() const;
    QString fullscreenSource() const;
    QString thumbnailSource() const;

    int width() const;
    int height() const;

signals:
    void nameChanged();
    void sourceChanged();
    void widthChanged();
    void heightChanged();

private:
    QString m_id;
    QString m_name;
    int m_width;
    int m_height;
    qint64 m_timestamp;
    SketchPage *m_page;
};

#endif // SKETCH_ARTBOARD_H
