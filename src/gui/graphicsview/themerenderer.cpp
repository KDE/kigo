/*
    Copyright 2008 Sascha Peilicke <sasch.pe@gmx.de>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "themerenderer.h"
#include "preferences.h"

#include <QSvgRenderer>
#include <KPixmapCache>
#include <KDebug>

#define USE_UNSTABLE_LIBKDEGAMESPRIVATE_API
#include <libkdegamesprivate/kgametheme.h>

#include <QPixmap>
#include <QPainter>

namespace Kigo {

ThemeRenderer::ThemeRenderer()
    : m_renderer(new QSvgRenderer)
    , m_cache(new KPixmapCache("kigo-cache"))
{
    m_cache->setCacheLimit(3 * 1024);
    if (!loadTheme(Preferences::theme())) {
        kDebug() << "Failed to load any game theme!";
    }
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
        // kDebug() << "Notice: Loading the same theme";
        return true;        // We don't have to do anything
    }

    m_currentTheme = themeName;

    KGameTheme theme;
    if (themeName.isEmpty() || !theme.load(themeName)) {
        // kDebug() << "Failed to load theme" << Preferences::theme();
        // kDebug() << "Trying to load default";
        if (!theme.loadDefault())
            return true;

        discardCache = true;
        m_currentTheme = "default";
    }

    // kDebug() << "Loading" << theme.graphics();
    if (!m_renderer->load(theme.graphics()))
        return false;

    if (discardCache) {
        // kDebug() << "Discarding cache";
        m_cache->discard();
    }
    emit themeChanged(m_currentTheme);
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
        case Background:
            cacheName = QString("background_%1x%2").arg(size.width()).arg(size.height());
            break;
        case Board:
            cacheName = QString("board_%1x%2").arg(size.width()).arg(size.height());
            break;
        case HandicapMark:
            cacheName = QString("handicap_mark_%1x%2").arg(size.width()).arg(size.height());
            break;
        case WhiteStone:
            cacheName = QString("white_stone_%1x%2").arg(size.width()).arg(size.height());
            break;
        case WhiteStoneTransparent:
            cacheName = QString("white_stone_%1x%2_trans").arg(size.width()).arg(size.height());
            break;
        case WhiteTerritory:
            cacheName = QString("white_territory_%1x%2").arg(size.width()).arg(size.height());
            break;
        case BlackStone:
            cacheName = QString("black_stone_%1x%2").arg(size.width()).arg(size.height());
            break;
        case BlackStoneTransparent:
            cacheName = QString("black_stone_%1x%2_trans").arg(size.width()).arg(size.height());
            break;
        case BlackTerritory:
            cacheName = QString("black_territory_%1x%2").arg(size.width()).arg(size.height());
            break;
        case PlacementMarker:
            cacheName = QString("placement_marker_%1x%2").arg(size.width()).arg(size.height());
            break;
    }

    // Check if board element is already in cache, if not render it
    QPixmap pixmap;
    if (!m_cache->find(cacheName, pixmap)) {
        pixmap = QPixmap(size);
        pixmap.fill(Qt::transparent);
        QPainter p(&pixmap);
        switch (element) {
            case Background:
                m_renderer->render(&p, "background");
                break;
            case Board:
                m_renderer->render(&p, "board");
                break;
            case HandicapMark:
                m_renderer->render(&p, "handicap_mark");
                break;
            case WhiteStone:
                m_renderer->render(&p, "white_stone");
                break;
            case WhiteStoneTransparent:
                p.setOpacity(0.5);
                m_renderer->render(&p, "white_stone");
                break;
            case WhiteTerritory:
                m_renderer->render(&p, "white_territory");
                break;
            case BlackStone:
                m_renderer->render(&p, "black_stone");
                break;
            case BlackStoneTransparent:
                p.setOpacity(0.5);
                m_renderer->render(&p, "black_stone");
                break;
            case BlackTerritory:
                m_renderer->render(&p, "black_territory");
                break;
            case PlacementMarker:
                m_renderer->render(&p, "placement_marker");
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
        case Background:
            sizeRect = m_renderer->boundsOnElement("background");
            break;
        case Board:
            sizeRect = m_renderer->boundsOnElement("board");
            break;
        case HandicapMark:
            sizeRect = m_renderer->boundsOnElement("handicap_mark");
            break;
        case WhiteStone:
            sizeRect = m_renderer->boundsOnElement("white_stone");
            break;
        case WhiteStoneTransparent:
            sizeRect = m_renderer->boundsOnElement("white_stone");
            break;
        case WhiteTerritory:
            sizeRect = m_renderer->boundsOnElement("white_territory");
            break;
        case BlackStone:
            sizeRect = m_renderer->boundsOnElement("black_stone");
            break;
        case BlackStoneTransparent:
            sizeRect = m_renderer->boundsOnElement("black_stone");
            break;
        case BlackTerritory:
            sizeRect = m_renderer->boundsOnElement("black_territory");
            break;
        case PlacementMarker:
            sizeRect = m_renderer->boundsOnElement("placement_marker");
            break;
    }
    return QSize((int)sizeRect.width(), (int)sizeRect.height());
}

} // End of namespace Kigo

#include "moc_themerenderer.cpp"
