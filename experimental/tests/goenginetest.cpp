#include "goenginetest.h"

#include "game/goengine.h"

#include <QTest>

void GoEngineTest::initTestCase()
{
    m_engine = new Kigo::GoEngine;
}

void GoEngineTest::cleanupTestCase()
{
    delete m_engine;
}

void GoEngineTest::startStop()
{
    connect(m_engine, SIGNAL(started()), this, SLOT(onStarted()));
    connect(m_engine, SIGNAL(error()), this, SLOT(onError()));
    m_engine->start("gnugo --mode gtp");


    m_engine->stop();
}

void GoEngineTest::nameAndVersion()
{

}

void GoEngineTest::onStarted()
{

}

void GoEngineTest::onStopped()
{

}

void GoEngineTest::onError()
{

}

QTEST_MAIN(GoEngineTest)

//#include "moc_goenginetest.cpp"
