/*******************************************************************
 *
 * Copyright 2008 Sascha Peilicke <sasch.pe@gmx.de>
 *
 * This file is part of the KDE project "KGo"
 *
 * KGo is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * KGo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with KReversi; see the file COPYING.  If not, write to
 * the Free Software Foundation, 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 *******************************************************************/

/**
 * @file This file is part of KGO and defines the class ThemeRenderer,
 *       which provides means to get parts of the current selected theme.
 *
 * @author Sascha Peilicke <sasch.pe@gmx.de>
 */
#ifndef KGO_THEMERENDERER_H
#define KGO_THEMERENDERER_H

#include <QString>

class KPixmapCache;
class KSvgRenderer;
class QPixmap;
class QPainter;
class QSize;
class QRectF;

namespace KGo {

/**
 * The class ThemeRenderer loads shapes from a SVG theme file and converts them into
 * pixmaps which can be used by QGraphicsView classes to display the game scene.
 * The class is implemented as a singleton because it is needed only once per
 * application and uses a pixamp cache to efficiently store the pixmaps.
 *
 * ThemeRenderer is implemented as a singleton.
 *
 * @author Sascha Peilicke <sasch.pe@gmx.de>
 * @since 0.1
 */
class ThemeRenderer
{
private:
    ThemeRenderer();
    ~ThemeRenderer();

public:
    /**
     * Enumeration of all possible renderable scene element types.
     */
    enum Element {
        SceneBackground = 1,        ///<
        BoardBackground,            ///<
        BoardHandicapMark,          ///<
        WhiteStone,                 ///<
        WhiteStoneTransparent,      ///<
        BlackStone,                 ///<
        BlackStoneTransparent       //<<
    };

    /**
     * Only one ThemeRenderer is needed per application, this method returns
     * the singleton instance.
     *
     * @return ThemeRenderer instance
     */
    inline static ThemeRenderer* instance()
    {
        static ThemeRenderer instance;
        return &instance;
    }

    /**
     * Load the game theme specified by 'name'. See the correspondig KConfigXT
     * settings file and available themes for details.
     *
     * @param themeName The name of the theme to load
     */
    bool loadTheme(const QString &themeName);

    /**
     * Renders a specific element of the current SVG theme.
     *
     * @param element Determines which part of the theme to render
     * @param painter The QPainter to paint the element with
     * @param rect The desired size of the element to render
     */
    void renderElement(Element element, QPainter *painter, const QRectF &rect) const;
    /**
     * Renders and returns a specific element of the current SVG theme.
     *
     * @param element Determines which part of the theme to render
     * @param size The desired size of the element to render
     */
    QPixmap renderElement(Element element, const QSize &size) const;

    /**
     * Retrieves the default size of a given theme element from the current SVG theme.
     *
     * @param element The theme element to return the size of
     * @return The rounded size of the elements rectangle
     */
    QSize elementSize(Element element) const;

private:
    QString m_currentTheme;         ///< Holds the current seleted theme
    KSvgRenderer *m_renderer;       ///< Converts SVG parts into pixmaps
    KPixmapCache *m_cache;          ///< Stores pixmaps efficiently
};

} // End of namespace KGo

#endif
