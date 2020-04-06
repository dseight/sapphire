#ifndef SKETCH_DOCUMENT_H
#define SKETCH_DOCUMENT_H

#include "qqmlobjectlistmodel.h"
#include "sketchpage.h"
#include <QColor>
#include <QJsonObject>
#include <QObject>

class SketchServer;

class SketchDocument : public QQmlObjectListModel<SketchPage>
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(int currentIndex READ currentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(QColor flowColor READ flowColor NOTIFY flowColorChanged)

public:
    explicit SketchDocument(QObject *parent = nullptr);
    SketchDocument(const QJsonObject &data, SketchServer *server);

    void update(const QJsonObject &data);
    void updateArtboard(const QString &id, const QJsonObject &data);
    void setCurrentArtboard(const QString &id);
    const SketchServer *server() const;

    QString id() const;
    QString name() const;
    QColor flowColor() const;

    // Index of page currently selected in Sketch
    int currentIndex() const;

signals:
    void nameChanged();
    void flowColorChanged();
    void currentIndexChanged();

private:
    QString m_id;
    QString m_name;
    QColor m_flowColor;
    int m_currentIndex;
    SketchServer *m_server;
};

#endif // SKETCH_DOCUMENT_H
