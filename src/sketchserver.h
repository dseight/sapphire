#ifndef SKETCH_SERVER_H
#define SKETCH_SERVER_H

#include "qqmlobjectlistmodel.h"
#include <QHostAddress>
#include <QJsonObject>
#include <QObject>
#include <QtWebSockets/QtWebSockets>
#include <qmdnsengine/service.h>

class SketchDocument;

class SketchServer : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(bool connected READ connected NOTIFY connectionStatusChanged)
    Q_PROPERTY(SketchDocument *document READ document NOTIFY documentChanged)

public:
    explicit SketchServer(const QMdnsEngine::Service &service,
                          const QHostAddress &host,
                          QObject *parent = nullptr);

    QString name() const;
    bool connected() const;
    const QMdnsEngine::Service &service() const;

    SketchDocument *document();

    Q_INVOKABLE void connect();

    QUrl baseUrl() const;
    QString token() const;

signals:
    void connectionStatusChanged();
    void documentChanged();

private slots:
    void textMessageReceived(const QString &message);

private:
    QMdnsEngine::Service m_service;
    QHostAddress m_host;
    bool m_connected;
    bool m_connecting;
    QWebSocket m_ws;

    QString m_token;

    SketchDocument *m_document;
};

#endif // SKETCH_SERVER_H
