#include <sailfishapp.h>
#include <QCoreApplication>
#include <QTimer>
#include <QtQuick>

#include "sketchdocument.h"
#include "sketchpage.h"
#include "sketchserver.h"
#include "sketchservermodel.h"

#ifndef VERSION_STRING
#define VERSION_STRING "0.0.0"
#endif

namespace {

QObject *serverModelProvider(QQmlEngine *, QJSEngine *)
{
    return new SketchServerModel;
}

} // namespace

int main(int argc, char *argv[])
{
    QScopedPointer<QGuiApplication> app(SailfishApp::application(argc, argv));
    QScopedPointer<QQuickView> view(SailfishApp::createView());

    app->setApplicationDisplayName("Sapphire");
    app->setApplicationVersion(VERSION_STRING);

    const char *uri = "com.dseight.sapphire";

    qmlRegisterSingletonType<SketchServerModel>(uri, 1, 0, "SketchServerModel", serverModelProvider);

    const QString reason = QStringLiteral("Could be created only from C++ side");
    qmlRegisterUncreatableType<SketchServer>(uri, 1, 0, "SketchServer", reason);
    qmlRegisterUncreatableType<SketchDocument>(uri, 1, 0, "SketchDocument", reason);
    qmlRegisterUncreatableType<SketchPage>(uri, 1, 0, "SketchPage", reason);

    view->setSource(SailfishApp::pathTo(QStringLiteral("qml/main.qml")));
    view->show();

    return app->exec();
}
