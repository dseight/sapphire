#include "sketchserver.h"
#include "deviceinfo.h"
#include "sketchdocument.h"
#include <QRegularExpression>
#include <QtWebSockets/QtWebSockets>

namespace {

QString extractDisplayName(const QString &fullName)
{
    QRegularExpression re("Sketch Mirror \\((.*)\\)");
    return re.match(fullName).captured(1);
}

} // namespace

SketchServer::SketchServer(const QMdnsEngine::Service &service,
                           const QHostAddress &host,
                           QObject *parent)
    : QObject(parent)
    , m_service(service)
    , m_host(host)
    , m_connected(false)
    , m_connecting(false)
    , m_document(new SketchDocument(this))
{
}

QString SketchServer::name() const
{
    return extractDisplayName(m_service.name());
}

bool SketchServer::connected() const
{
    return m_connected;
}

const QMdnsEngine::Service &SketchServer::service() const
{
    return m_service;
}

SketchDocument *SketchServer::document()
{
    return m_document;
}

QUrl SketchServer::baseUrl() const
{
    QUrl url;
    url.setScheme(QStringLiteral("http"));
    url.setHost(m_host.toString());
    url.setPort(m_service.port());
    return url;
}

QString SketchServer::token() const
{
    return m_token;
}

void SketchServer::connect()
{
    if (m_connected || m_connecting)
        return;

    m_connecting = true;

    QUrl url;
    url.setScheme(QStringLiteral("ws"));
    url.setHost(m_host.toString());
    url.setPort(m_service.port() + 1);

    QObject::connect(&m_ws, &QWebSocket::connected, this, [=]() {
        QObject::connect(&m_ws, &QWebSocket::textMessageReceived,
                         this, &SketchServer::textMessageReceived);

        m_ws.sendTextMessage(QJsonDocument(deviceInfo()).toJson(QJsonDocument::Compact));
    });

    QObject::connect(&m_ws, &QWebSocket::disconnected, this, [=]() {
        if (m_connected) {
            m_connected = false;
            emit connectionStatusChanged();
        }
    });

    m_ws.open(url);
}

void SketchServer::textMessageReceived(const QString &message)
{
    auto doc = QJsonDocument::fromJson(message.toUtf8());
    auto object = doc.object();
    auto type = object["type"].toString();
    auto content = object["content"].toObject();

    if (type == QStringLiteral("connected")) {
        m_connected = true;
        m_token = content["token"].toString();
        emit connectionStatusChanged();
    } else if (type == QStringLiteral("manifest")) {
        const auto empty = content.isEmpty();
        const auto id = content["id"].toString();

        if (id == m_document->id()) {
            m_document->update(content);
        } else {
            m_document->deleteLater();
            if (empty) {
                m_document = new SketchDocument(this);
            } else {
                m_document = new SketchDocument(content, this);
            }
        }
        emit documentChanged();
    } else if (type == QStringLiteral("artboard")) {
        const auto id = content["identifier"].toString();
        m_document->updateArtboard(id, content);
    } else if (type == QStringLiteral("current-artboard")) {
        const auto id = content["identifier"].toString();
        m_document->setCurrentArtboard(id);
    } else {
        qWarning() << "Unhandled event of type" << type;
        qDebug() << content;
    }
}
