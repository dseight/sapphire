#include "sketchservermodel.h"
#include <qmdnsengine/browser.h>
#include <qmdnsengine/cache.h>
#include <qmdnsengine/resolver.h>
#include <qmdnsengine/server.h>
#include <qmdnsengine/service.h>

namespace {

bool serviceIsSketchMirror(const QMdnsEngine::Service &service)
{
    return service.name().startsWith("Sketch Mirror");
}

} // namespace

class SketchServerModelPrivate : public QObject
{
    Q_OBJECT

public:
    explicit SketchServerModelPrivate(SketchServerModel *parent)
        : QObject(parent)
        , q(parent)
        , m_server(new QMdnsEngine::Server(this))
        , m_cache(new QMdnsEngine::Cache(this))
        , m_browser(new QMdnsEngine::Browser(m_server, "_http._tcp.local.", m_cache, this))
    {
        QObject::connect(m_browser, &QMdnsEngine::Browser::serviceAdded,
                         this, &SketchServerModelPrivate::serviceAdded);
        QObject::connect(m_browser, &QMdnsEngine::Browser::serviceRemoved,
                         this, &SketchServerModelPrivate::serviceRemoved);
    }

private slots:
    void serviceAdded(const QMdnsEngine::Service &service)
    {
        if (!serviceIsSketchMirror(service))
            return;

        qDebug() << "Discovered" << service.name();

        auto resolver = new QMdnsEngine::Resolver(m_server, service.hostname(), m_cache);

        // FIXME: resolved() may be called multiple times
        QObject::connect(resolver, &QMdnsEngine::Resolver::resolved, [=](const QHostAddress &host) {
            resolver->deleteLater();

            auto server = new SketchServer(service, host);
            q->append(server);

            qDebug() << "Resolved" << host << "for" << service.name();
        });
    }

    void serviceRemoved(const QMdnsEngine::Service &service)
    {
        if (!serviceIsSketchMirror(service))
            return;

        const auto servers = q->toList();
        SketchServer *removedServer = nullptr;

        for (const auto &server : servers) {
            if (server->service() == service) {
                removedServer = server;
                break;
            }
        }

        if (removedServer == nullptr)
            return;

        qDebug() << "Removing" << service.name();
        q->remove(removedServer);
    }

private:
    SketchServerModel *const q;
    QMdnsEngine::Server *m_server;
    QMdnsEngine::Cache *m_cache;
    QMdnsEngine::Browser *m_browser;
};

SketchServerModel::SketchServerModel(QObject *parent)
    : QQmlObjectListModel(parent)
    , d(new SketchServerModelPrivate(this))
{
}

#include "sketchservermodel.moc"
