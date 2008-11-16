#ifndef GOENGINETEST_H
#define GOENGINETEST_H

#include <QObject>

namespace Kigo {
    class GoEngine;
}

/**
 * @brief A test class for GoEngine
 */
class GoEngineTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void startStop();
    void nameAndVersion();

public slots:
    void onStarted();
    void onStopped();
    void onError();

private:
    Kigo::GoEngine *m_engine;
};

#endif
