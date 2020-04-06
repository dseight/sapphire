#ifndef QQMLOBJECTLISTMODEL_H
#define QQMLOBJECTLISTMODEL_H

// Based on QQmlObjectListModel by Thomas Boutroue
// http://gitlab.unique-conception.org/qt-qml-tricks/qt-qml-models/blob/master/QQmlObjectListModel.h

#include <QAbstractListModel>
#include <QByteArray>
#include <QChar>
#include <QDebug>
#include <QHash>
#include <QList>
#include <QMetaMethod>
#include <QMetaObject>
#include <QMetaProperty>
#include <QObject>
#include <QString>
#include <QStringBuilder>
#include <QVariant>
#include <QVector>

// custom foreach for QList, which uses no copy and check pointer non-null
#define FOREACH_PTR_IN_QLIST(_type_, _var_, _list_) \
    for (typename QList<_type_ *>::const_iterator it = _list_.constBegin(); \
         it != _list_.constEnd(); \
         ++it) \
        if (_type_ *_var_ = (*it))

class QQmlObjectListModelBase : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    explicit QQmlObjectListModelBase(QObject *parent = nullptr)
        : QAbstractListModel(parent)
    {
    }

public slots: // virtual methods API for QML
    // TODO: write a comment about why this check is false-positive here
    // clazy:excludeall=const-signal-or-slot
    virtual int size() const = 0;
    virtual int count() const = 0;
    virtual bool isEmpty() const = 0;
    virtual bool contains(QObject *item) const = 0;
    virtual int indexOf(QObject *item) const = 0;
    virtual int roleForName(const QByteArray &name) const = 0;
    virtual void clear() = 0;
    virtual void append(QObject *item) = 0;
    virtual void prepend(QObject *item) = 0;
    virtual void insert(int idx, QObject *item) = 0;
    virtual void move(int idx, int pos) = 0;
    virtual void remove(QObject *item) = 0;
    virtual void remove(int idx) = 0;
    virtual QObject *get(int idx) const = 0;
    virtual QObject *getFirst() const = 0;
    virtual QObject *getLast() const = 0;

protected slots:
    virtual void onItemPropertyChanged() = 0;

signals:
    void countChanged();
};

template<class ItemType>
class QQmlObjectListModel : public QQmlObjectListModelBase
{
public:
    explicit QQmlObjectListModel(QObject *parent = nullptr, const QByteArray &uidRole = QByteArray())
        : QQmlObjectListModelBase(parent)
        , m_count(0)
        , m_uidRoleName(uidRole)
    {
        static QSet<QByteArray> roleNamesBlacklist = {
            QByteArrayLiteral("id"),
            QByteArrayLiteral("index"),
            QByteArrayLiteral("class"),
            QByteArrayLiteral("model"),
            QByteArrayLiteral("modelData"),
        };

        auto itemMeta = ItemType::staticMetaObject;

        static const char *HANDLER = "onItemPropertyChanged()";
        m_handler = staticMetaObject.method(staticMetaObject.indexOfMethod(HANDLER));
        m_roles.insert(baseRole(), QByteArrayLiteral("qtObject"));

        const int len = itemMeta.propertyCount();

        for (int property = 0; property < len; property++) {
            QMetaProperty propertyMeta = itemMeta.property(property);
            const QByteArray propertyName = QByteArray(propertyMeta.name());
            const int role = baseRole() + 1 + property;

            if (roleNamesBlacklist.contains(propertyName)) {
                static const QByteArray CLASS_NAME = (QByteArrayLiteral("QQmlObjectListModel<")
                                                      % itemMeta.className() % '>');
                qWarning() << "Can't have" << propertyName << "as a role name in"
                           << qPrintable(CLASS_NAME);
                continue;
            }

            m_roles.insert(role, propertyName);
            if (propertyMeta.hasNotifySignal()) {
                m_signalIdxToRole.insert(propertyMeta.notifySignalIndex(), role);
            }
        }
    }

    bool setData(const QModelIndex &index, const QVariant &value, int role) final
    {
        bool ret = false;
        ItemType *item = at(index.row());
        const QByteArray rolename = m_roles.value(role, emptyBA());
        if (item != nullptr && role != baseRole() && !rolename.isEmpty()) {
            ret = item->setProperty(rolename, value);
        }
        return ret;
    }

    QVariant data(const QModelIndex &index, int role) const final
    {
        QVariant ret;
        ItemType *item = at(index.row());
        const QByteArray rolename = m_roles.value(role, emptyBA());
        if (item != nullptr && !rolename.isEmpty()) {
            ret.setValue(role != baseRole() ? item->property(rolename)
                                            : QVariant::fromValue(static_cast<QObject *>(item)));
        }
        return ret;
    }

    QHash<int, QByteArray> roleNames() const final
    {
        return m_roles;
    }

    typedef typename QList<ItemType *>::const_iterator const_iterator;
    const_iterator begin() const
    {
        return m_items.begin();
    }

    const_iterator end() const
    {
        return m_items.end();
    }

    const_iterator constBegin() const
    {
        return m_items.constBegin();
    }

    const_iterator constEnd() const
    {
        return m_items.constEnd();
    }

public: // C++ API
    ItemType *at(int idx) const
    {
        ItemType *ret = nullptr;
        if (idx >= 0 && idx < m_items.size()) {
            ret = m_items.value(idx);
        }
        return ret;
    }

    ItemType *getByUid(const QString &uid) const
    {
        return m_indexByUid.isEmpty() ? nullptr : m_indexByUid.value(uid, nullptr);
    }

    int roleForName(const QByteArray &name) const final
    {
        return m_roles.key(name, -1);
    }

    int count() const final
    {
        return m_count;
    }

    int size() const final
    {
        return m_count;
    }

    bool isEmpty() const final
    {
        return m_items.isEmpty();
    }

    bool contains(ItemType *item) const
    {
        return m_items.contains(item);
    }

    bool contains(const QString &uid) const
    {
        return m_indexByUid.contains(uid);
    }

    int indexOf(ItemType *item) const
    {
        return m_items.indexOf(item);
    }

    void clear() final
    {
        if (m_items.isEmpty())
            return;

        beginRemoveRows(QModelIndex(), 0, m_items.count() - 1);
        FOREACH_PTR_IN_QLIST (ItemType, item, m_items) {
            dereferenceItem(item);
        }
        m_items.clear();
        updateCounter();
        endRemoveRows();
    }

    void append(ItemType *item)
    {
        if (item == nullptr)
            return;

        const int pos = m_items.count();
        beginInsertRows(QModelIndex(), pos, pos);
        m_items.append(item);
        referenceItem(item);
        updateCounter();
        endInsertRows();
    }

    void prepend(ItemType *item)
    {
        if (item == nullptr)
            return;

        beginInsertRows(QModelIndex(), 0, 0);
        m_items.prepend(item);
        referenceItem(item);
        updateCounter();
        endInsertRows();
    }

    void insert(int idx, ItemType *item)
    {
        if (item == nullptr)
            return;

        beginInsertRows(QModelIndex(), idx, idx);
        m_items.insert(idx, item);
        referenceItem(item);
        updateCounter();
        endInsertRows();
    }

    void append(const QList<ItemType *> &itemList)
    {
        if (itemList.isEmpty())
            return;

        const int pos = m_items.count();
        beginInsertRows(QModelIndex(), pos, pos + itemList.count() - 1);
        m_items.reserve(m_items.count() + itemList.count());
        m_items.append(itemList);
        FOREACH_PTR_IN_QLIST (ItemType, item, itemList) {
            referenceItem(item);
        }
        updateCounter();
        endInsertRows();
    }

    void prepend(const QList<ItemType *> &itemList)
    {
        if (itemList.isEmpty())
            return;

        beginInsertRows(QModelIndex(), 0, itemList.count() - 1);
        m_items.reserve(m_items.count() + itemList.count());
        int offset = 0;
        FOREACH_PTR_IN_QLIST (ItemType, item, itemList) {
            m_items.insert(offset, item);
            referenceItem(item);
            offset++;
        }
        updateCounter();
        endInsertRows();
    }

    void insert(int idx, const QList<ItemType *> &itemList)
    {
        if (itemList.isEmpty())
            return;

        beginInsertRows(QModelIndex(), idx, idx + itemList.count() - 1);
        m_items.reserve(m_items.count() + itemList.count());
        int offset = 0;
        FOREACH_PTR_IN_QLIST (ItemType, item, itemList) {
            m_items.insert(idx + offset, item);
            referenceItem(item);
            offset++;
        }
        updateCounter();
        endInsertRows();
    }

    void move(int idx, int pos) final
    {
        if (idx == pos)
            return;

        // FIXME: use begin/end MoveRows when supported by Repeater, since then use remove/insert pair
        // beginMoveRows(QModelIndex(), idx, idx, QModelIndex(), (idx < pos ? pos + 1 : pos));
        beginRemoveRows(QModelIndex(), idx, idx);
        beginInsertRows(QModelIndex(), pos, pos);
        m_items.move(idx, pos);
        endRemoveRows();
        endInsertRows();
        // endMoveRows();
    }

    void remove(ItemType *item)
    {
        if (item == nullptr)
            return;

        const int idx = m_items.indexOf(item);
        remove(idx);
    }

    void remove(int idx) final
    {
        if (idx >= 0 && idx < m_items.size()) {
            beginRemoveRows(QModelIndex(), idx, idx);
            ItemType *item = m_items.takeAt(idx);
            dereferenceItem(item);
            updateCounter();
            endRemoveRows();
        }
    }

    ItemType *first() const
    {
        return m_items.first();
    }

    ItemType *last() const
    {
        return m_items.last();
    }

    const QList<ItemType *> &toList() const
    {
        return m_items;
    }

public: // QML slots implementation
    void append(QObject *item) final
    {
        append(qobject_cast<ItemType *>(item));
    }

    void prepend(QObject *item) final
    {
        prepend(qobject_cast<ItemType *>(item));
    }

    void insert(int idx, QObject *item) final
    {
        insert(idx, qobject_cast<ItemType *>(item));
    }

    void remove(QObject *item) final
    {
        remove(qobject_cast<ItemType *>(item));
    }

    bool contains(QObject *item) const final
    {
        return contains(qobject_cast<ItemType *>(item));
    }

    int indexOf(QObject *item) const final
    {
        return indexOf(qobject_cast<ItemType *>(item));
    }

    QObject *get(int idx) const final
    {
        return static_cast<QObject *>(at(idx));
    }

    QObject *getFirst() const final
    {
        return static_cast<QObject *>(first());
    }

    QObject *getLast() const final
    {
        return static_cast<QObject *>(last());
    }

protected:
    static const QByteArray &emptyBA()
    {
        static const QByteArray ret = QByteArrayLiteral("");
        return ret;
    }

    static int baseRole()
    {
        return Qt::UserRole;
    }

    int rowCount(const QModelIndex &parent) const final
    {
        return !parent.isValid() ? m_items.count() : 0;
    }

    void referenceItem(ItemType *item)
    {
        if (item == nullptr)
            return;

        if (!item->parent()) {
            item->setParent(this);
        }
        for (QHash<int, int>::const_iterator it = m_signalIdxToRole.constBegin();
             it != m_signalIdxToRole.constEnd();
             ++it) {
            connect(item, item->metaObject()->method(it.key()),
                    this, m_handler, Qt::UniqueConnection);
        }
        if (!m_uidRoleName.isEmpty()) {
            const QString key = m_indexByUid.key(item);
            if (!key.isEmpty()) {
                m_indexByUid.remove(key);
            }
            const QString value = item->property(m_uidRoleName).toString();
            if (!value.isEmpty()) {
                m_indexByUid.insert(value, item);
            }
        }
    }

    void dereferenceItem(ItemType *item)
    {
        if (item == nullptr)
            return;

        disconnect(this, nullptr, item, nullptr);
        disconnect(item, nullptr, this, nullptr);

        if (!m_uidRoleName.isEmpty()) {
            const QString key = m_indexByUid.key(item);
            if (!key.isEmpty()) {
                m_indexByUid.remove(key);
            }
        }

        // FIXME: maybe that's not the best way to test ownership ?
        if (item->parent() == this) {
            item->deleteLater();
        }
    }

    void onItemPropertyChanged() final
    {
        ItemType *item = qobject_cast<ItemType *>(sender());
        const int row = m_items.indexOf(item);
        const int sig = senderSignalIndex();
        const int role = m_signalIdxToRole.value(sig, -1);
        if (row >= 0 && role >= 0) {
            const QModelIndex index = QAbstractListModel::index(row, 0, QModelIndex());
            QVector<int> rolesList;
            rolesList.append(role);
            emit dataChanged(index, index, rolesList);
        }
        if (!m_uidRoleName.isEmpty()) {
            const QByteArray roleName = m_roles.value(role, emptyBA());
            if (!roleName.isEmpty() && roleName == m_uidRoleName) {
                const QString key = m_indexByUid.key(item);
                if (!key.isEmpty()) {
                    m_indexByUid.remove(key);
                }
                const QString value = item->property(m_uidRoleName).toString();
                if (!value.isEmpty()) {
                    m_indexByUid.insert(value, item);
                }
            }
        }
    }

    inline void updateCounter()
    {
        if (m_count != m_items.count()) {
            m_count = m_items.count();
            emit countChanged();
        }
    }

private:
    int m_count;
    QByteArray m_uidRoleName;
    QMetaMethod m_handler;
    QHash<int, QByteArray> m_roles;
    QHash<int, int> m_signalIdxToRole;
    QList<ItemType *> m_items;
    QHash<QString, ItemType *> m_indexByUid;
};

#endif // QQMLOBJECTLISTMODEL_H
