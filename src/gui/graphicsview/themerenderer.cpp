/***************************************************************************
 *   Copyright (C) 2008 by Sascha Peilicke <sasch.pe@gmx.de>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "themerenderer.h"
#include "preferences.h"

#include <KGameTheme>
#include <KSvgRenderer>
#include <KPixmapCache>
#include <KStandardDirs>
#include <KDebug>

#include <QPixmap>
#include <QPainter>

namespace Kigo {

ThemeRenderer::ThemeRenderer()
    : m_renderer(new KSvgRenderer)
    , m_cache(new KPixmapCache("kigo-cache"))
{
    m_cache->setCacheLimit(3 * 1024);
    if (!loadTheme(Preferences::theme()))
        kDebug() << "Failed to load any game theme!";
}

ThemeRenderer::~ThemeRenderer()
{
    delete m_cache;
    delete m_renderer;
}

bool ThemeRenderer::loadTheme(const QString &themeName)
{
    bool discardCache = !m_currentTheme.isEmpty();

    if (!m_currentTheme.isEmpty() && m_currentTheme == themeName) {
        kDebug() << "Notice: Loading the same theme";
        return true;        // We don't have to do anything
    }

    m_currentTheme = themeName;

    KGameTheme theme;
    if (themeName.isEmpty() || !theme.load(themeName)) {
        kDebug() << "Failed to load theme" << Preferences::theme();
        kDebug() << "Trying to load default";
        if (!theme.loadDefault())
            return true;

        discardCache = true;
        m_currentTheme = "default";
    }

    kDebug() << "Loading" << theme.graphics();
    if (!m_renderer->load(theme.graphics()))
        return false;

    if (discardCache) {
        kDebug() << "Discarding cache";
        m_cache->discard();
    }
    return true;
}

void ThemeRenderer::renderElement(Element element, QPainter *painter, const QRectF &rect) const
{
    if (!m_renderer->isValid() || rect.isEmpty() || painter == 0)
        return;

    QPixmap pixmap = renderElement(element, rect.size().toSize());

    // Draw pixmap with the parameter provided painter
    painter->drawPixmap(static_cast<int>(rect.x()), static_cast<int>(rect.y()), pixmap);
}

QPixmap ThemeRenderer::renderElement(Element element, const QSize &size) const
{
    if (!m_renderer->isValid() || size.isEmpty())
        return QPixmap();

    // Determine which board element we have to render and set the cache item name
    QString cacheName;
    switch (element) {
        case SceneBackground:
            cacheName = QString("background_%1x%2").arg(size.width()).arg(size.height());
            break;
        case BoardBackground:
            cacheName = QString("board_background_%1x%2").arg(size.width()).arg(size.height());
            break;
        case BoardHandicapMark:
            cacheName = QString("board_handicap_mark_%1x%2").arg(size.width()).arg(size.height());
            break;
        case WhiteStone:
            cacheName = QString("board_stone_white_%1x%2").arg(size.width()).arg(size.height());
            break;
        case WhiteStoneTransparent:
            cacheName = QString("board_stone_white_%1x%2_trans").arg(size.width()).arg(size.height());
            break;
        case BlackStone:
            cacheName = QString("board_stone_black_%1x%2").arg(size.width()).arg(size.height());
            break;
        case BlackStoneTransparent:
            cacheName = QString("board_stone_black_%1x%2_trans").arg(size.width()).arg(size.height());
            break;
    }

    // Check if board element is already in cache, if not render it
    QPixmap pixmap;
    if (!m_cache->find(cacheName, pixmap)) {
        pixmap = QPixmap(size);
        pixmap.fill(Qt::transparent);
        QPainter p(&pixmap);
        switch (element) {
            case SceneBackground:
                m_renderer->render(&p, "background");
                break;
            case BoardBackground:
                m_renderer->render(&p, "board_background");
                break;
            case BoardHandicapMark:
                m_renderer->render(&p, "board_handicap_mark");
                break;
            case WhiteStone:
                m_renderer->render(&p, "board_stone_white");
                break;
            case WhiteStoneTransparent:
                p.setOpacity(0.5);
                m_renderer->render(&p, "board_stone_white");
                break;
            case BlackStone:
                m_renderer->render(&p, "board_stone_black");
                break;
            case BlackStoneTransparent:
                p.setOpacity(0.5);
                m_renderer->render(&p, "board_stone_black");
                break;
        }
        m_cache->insert(cacheName, pixmap);
    }
    return pixmap;
}

QSize ThemeRenderer::elementSize(Element element) const
{
    QRectF sizeRect;
    switch(element) {
        case SceneBackground:
            sizeRect = m_renderer->boundsOnElement("background");
            break;
        case BoardBackground:
            sizeRect = m_renderer->boundsOnElement("board_background");
            break;
        case BoardHandicapMark:
            sizeRect = m_renderer->boundsOnElement("board_handicap_mark");
            break;
        case WhiteStone:
            sizeRect = m_renderer->boundsOnElement("board_stone_white");
            break;
        case WhiteStoneTransparent:
            sizeRect = m_renderer->boundsOnElement("board_stone_white");
            break;
        case BlackStone:
            sizeRect = m_renderer->boundsOnElement("board_stone_black");
            break;
        case BlackStoneTransparent:
            sizeRect = m_renderer->boundsOnElement("board_stone_black");
            break;
    }
    return QSize((int)sizeRect.width(), (int)sizeRect.height());
}

} // End of namespace Kigo

#include "moc_themerenderer.cpp"
