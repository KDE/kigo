/*
    SPDX-FileCopyrightText: 2008 Sascha Peilicke <sasch.pe@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "themerenderer.h"
#include "preferences.h"

#include <QSvgRenderer>
#include <QPixmapCache>

#define USE_UNSTABLE_LIBKDEGAMESPRIVATE_API
#include <libkdegamesprivate/kgametheme.h>

#include <QPixmap>
#include <QPainter>

namespace Kigo {

ThemeRenderer::ThemeRenderer()
    : m_currentTheme()
    , m_renderer(new QSvgRenderer)
{
    QPixmapCache::setCacheLimit(3 * 1024);
    if (!loadTheme(Preferences::theme())) {
        //qCDebug(KIGO_LOG) << "Failed to load any game theme!";
    }
}

ThemeRenderer::~ThemeRenderer()
{
    delete m_renderer;
}

bool ThemeRenderer::loadTheme(const QString &themeName)
{
    bool discardCache = !m_currentTheme.isEmpty();

    if (!m_currentTheme.isEmpty() && m_currentTheme == themeName) {
        //qCDebug(KIGO_LOG) << "Notice: Loading the same theme";
        return true;        // We don't have to do anything
    }

    m_currentTheme = themeName;

    KGameTheme theme;
    if (themeName.isEmpty() || !theme.load(themeName)) {
        //qCDebug(KIGO_LOG) << "Failed to load theme" << Preferences::theme();
        //qCDebug(KIGO_LOG) << "Trying to load default";
        if (!theme.loadDefault()) {
            return true;
        }

        discardCache = true;
        m_currentTheme = QStringLiteral("default");
    }

    //qCDebug(KIGO_LOG) << "Loading" << theme.graphics();
    if (!m_renderer->load(theme.graphics())) {
        return false;
    }

    if (discardCache) {
        //qCDebug(KIGO_LOG) << "Discarding cache";
        QPixmapCache::clear();
    }
    Q_EMIT themeChanged(m_currentTheme);
    return true;
}

void ThemeRenderer::renderElement(Element element, QPainter *painter, const QRectF &rect) const
{
    if (!m_renderer->isValid() || rect.isEmpty() || painter == nullptr) {
        return;
    }

    const QPixmap pixmap = renderElement(element, rect.size().toSize());

    // Draw pixmap with the parameter provided painter
    painter->drawPixmap(static_cast<int>(rect.x()), static_cast<int>(rect.y()), pixmap);
}

QPixmap ThemeRenderer::renderElement(Element element, const QSize &size) const
{
    if (!m_renderer->isValid() || size.isEmpty()) {
        return QPixmap();
    }

    // Determine which board element we have to render and set the cache item name
    QString cacheName;
    switch (element) {
        case Element::Background:
            cacheName = QStringLiteral("background_%1x%2").arg(size.width()).arg(size.height());
            break;
        case Element::Board:
            cacheName = QStringLiteral("board_%1x%2").arg(size.width()).arg(size.height());
            break;
        case Element::HandicapMark:
            cacheName = QStringLiteral("handicap_mark_%1x%2").arg(size.width()).arg(size.height());
            break;
        case Element::WhiteStone:
            cacheName = QStringLiteral("white_stone_%1x%2").arg(size.width()).arg(size.height());
            break;
        case Element::WhiteStoneLast:
            cacheName = QStringLiteral("white_stone_last_%1x%2").arg(size.width()).arg(size.height());
            break;
        case Element::WhiteStoneTransparent:
            cacheName = QStringLiteral("white_stone_%1x%2_trans").arg(size.width()).arg(size.height());
            break;
        case Element::WhiteTerritory:
            cacheName = QStringLiteral("white_territory_%1x%2").arg(size.width()).arg(size.height());
            break;
        case Element::BlackStone:
            cacheName = QStringLiteral("black_stone_%1x%2").arg(size.width()).arg(size.height());
            break;
        case Element::BlackStoneLast:
            cacheName = QStringLiteral("black_stone_last_%1x%2").arg(size.width()).arg(size.height());
            break;
        case Element::BlackStoneTransparent:
            cacheName = QStringLiteral("black_stone_%1x%2_trans").arg(size.width()).arg(size.height());
            break;
        case Element::BlackTerritory:
            cacheName = QStringLiteral("black_territory_%1x%2").arg(size.width()).arg(size.height());
            break;
        case Element::PlacementMarker:
            cacheName = QStringLiteral("placement_marker_%1x%2").arg(size.width()).arg(size.height());
            break;
    }

    // Check if board element is already in cache, if not render it
    QPixmap pixmap;
    if (!QPixmapCache::find(cacheName, &pixmap)) {
        pixmap = QPixmap(size);
        pixmap.fill(Qt::transparent);
        QPainter p(&pixmap);
        switch (element) {
            case Element::Background:
                m_renderer->render(&p, QStringLiteral("background"));
                break;
            case Element::Board:
                m_renderer->render(&p, QStringLiteral("board"));
                break;
            case Element::HandicapMark:
                m_renderer->render(&p, QStringLiteral("handicap_mark"));
                break;
            case Element::WhiteStone:
                m_renderer->render(&p, QStringLiteral("white_stone"));
                break;
            case Element::WhiteStoneLast:
                m_renderer->render(&p, QStringLiteral("white_stone_last"));
                break;
            case Element::WhiteStoneTransparent:
                p.setOpacity(0.5);
                m_renderer->render(&p, QStringLiteral("white_stone"));
                break;
            case Element::WhiteTerritory:
                m_renderer->render(&p, QStringLiteral("white_territory"));
                break;
            case Element::BlackStone:
                m_renderer->render(&p, QStringLiteral("black_stone"));
                break;
            case Element::BlackStoneLast:
                m_renderer->render(&p, QStringLiteral("black_stone_last"));
                break;
            case Element::BlackStoneTransparent:
                p.setOpacity(0.5);
                m_renderer->render(&p, QStringLiteral("black_stone"));
                break;
            case Element::BlackTerritory:
                m_renderer->render(&p, QStringLiteral("black_territory"));
                break;
            case Element::PlacementMarker:
                m_renderer->render(&p, QStringLiteral("placement_marker"));
                break;
        }
        QPixmapCache::insert(cacheName, pixmap);
    }
    return pixmap;
}

QSize ThemeRenderer::elementSize(Element element) const
{
    QRectF sizeRect;
    switch(element) {
        case Element::Background:
            sizeRect = m_renderer->boundsOnElement(QStringLiteral("background"));
            break;
        case Element::Board:
            sizeRect = m_renderer->boundsOnElement(QStringLiteral("board"));
            break;
        case Element::HandicapMark:
            sizeRect = m_renderer->boundsOnElement(QStringLiteral("handicap_mark"));
            break;
        case Element::WhiteStone:
            sizeRect = m_renderer->boundsOnElement(QStringLiteral("white_stone"));
            break;
        case Element::WhiteStoneLast:
            sizeRect = m_renderer->boundsOnElement(QStringLiteral("white_stone"));
            break;
        case Element::WhiteStoneTransparent:
            sizeRect = m_renderer->boundsOnElement(QStringLiteral("white_stone"));
            break;
        case Element::WhiteTerritory:
            sizeRect = m_renderer->boundsOnElement(QStringLiteral("white_territory"));
            break;
        case Element::BlackStone:
            sizeRect = m_renderer->boundsOnElement(QStringLiteral("black_stone"));
            break;
        case Element::BlackStoneLast:
            sizeRect = m_renderer->boundsOnElement(QStringLiteral("black_stone"));
            break;
        case Element::BlackStoneTransparent:
            sizeRect = m_renderer->boundsOnElement(QStringLiteral("black_stone"));
            break;
        case Element::BlackTerritory:
            sizeRect = m_renderer->boundsOnElement(QStringLiteral("black_territory"));
            break;
        case Element::PlacementMarker:
            sizeRect = m_renderer->boundsOnElement(QStringLiteral("placement_marker"));
            break;
    }
    return QSize(static_cast<int>(sizeRect.width()), static_cast<int>(sizeRect.height()));
}

} // End of namespace Kigo
