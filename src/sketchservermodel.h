#ifndef SKETCH_SERVER_MODEL_H
#define SKETCH_SERVER_MODEL_H

#include "qqmlobjectlistmodel.h"
#include "sketchserver.h"

class SketchServerModelPrivate;

class SketchServerModel : public QQmlObjectListModel<SketchServer>
{
    Q_OBJECT

public:
    explicit SketchServerModel(QObject *parent = nullptr);

private:
    SketchServerModelPrivate *const d;
};

#endif // SKETCH_SERVER_MODEL_H
