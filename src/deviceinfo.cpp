#include <MGConfItem>
#include <QDebug>
#include <QFile>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonObject>
#include <QRegularExpression>
#include <QScreen>
#include <QStandardPaths>
#include <QTextStream>
#include <QUuid>

namespace {

QMap<QString, QString> parseReleaseFile(const QString &filename)
{
    // Specification of the format:
    // http://www.freedesktop.org/software/systemd/man/os-release.html

    QFile release(filename);
    if (!release.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }

    QMap<QString, QString> result;

    QTextStream in(&release);
    in.setCodec("UTF-8");

    while (!in.atEnd()) {
        QString line = in.readLine();

        // "Lines beginning with "#" shall be ignored as comments."
        if (line.startsWith('#'))
            continue;

        QString key = line.section('=', 0, 0);
        QString value = line.section('=', 1);

        // Remove trailing whitespace in value
        value = value.trimmed();

        // "Variable assignment values should be enclosed in double or
        // single quotes if they include spaces, semicolons or other
        // special characters outside of A-Z, a-z, 0-9."
        if (((value.at(0) == '\'') || (value.at(0) == '"'))) {
            if (value.at(0) != value.at(value.size() - 1)) {
                qWarning("Quoting error in input line: '%s'", qPrintable(line));
                continue;
            }

            // Remove the quotes
            value = value.mid(1, value.size() - 2);
        }

        // "If double or single quotes or backslashes are to be used within
        // variable assignments, they should be escaped with backslashes,
        // following shell style."
        value = value.replace(QRegularExpression("\\\\(.)"), "\\1");

        result[key] = value;
    }

    return result;
}

QString userAgent()
{
    return QGuiApplication::applicationDisplayName();
}

QString deviceName()
{
    auto hwRelease = parseReleaseFile(QStringLiteral("/etc/hw-release"));
    return hwRelease.value(QStringLiteral("NAME"), userAgent());
}

QString osName()
{
    auto osRelease = parseReleaseFile(QStringLiteral("/etc/os-release"));
    return osRelease.value(QStringLiteral("NAME"));
}

QString uuid()
{
    const auto uuidConfPath = QStringLiteral("/com/dseight/%1/uuid")
                                  .arg(QCoreApplication::applicationName());

    MGConfItem uuidConf(uuidConfPath);
    auto value = uuidConf.value().toString();

    if (value.isEmpty()) {
        qDebug() << "No UUID generated yet";
        auto newUuid = QUuid::createUuid().toString();

        // Remove encolsing curly braces
        newUuid.remove(0, 1);
        newUuid.chop(1);

        value = newUuid;
        uuidConf.set(value);
    }

    return value;
}

} // namespace

QJsonObject deviceInfo()
{
    auto screenSize = QGuiApplication::primaryScreen()->size();

    QJsonObject defaultScreen{
        {"height", qMax(screenSize.width(), screenSize.height())},
        {"width", qMin(screenSize.width(), screenSize.height())},
        {"scale", 1.0},
    };

    QJsonArray screens{defaultScreen};

    QJsonObject content{
        {"display-name", deviceName()},
        {"model", osName()},
        {"user-agent", userAgent()},
        {"uuid", uuid()},
        {"screens", screens},
    };

    QJsonObject info{
        {"type", "device-info"},
        {"content", content},
    };

    return info;
}
