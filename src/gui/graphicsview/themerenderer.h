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

#ifndef KIGO_THEMERENDERER_H
#define KIGO_THEMERENDERER_H

#include <QObject>
#include <QString>

class QSvgRenderer;
class QPixmap;
class QPainter;
class QSize;
class QRectF;

namespace Kigo {

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
class ThemeRenderer : public QObject
{
    Q_OBJECT

private:
    ThemeRenderer();
    ~ThemeRenderer();

public:
    /**
     * Enumeration of all possible renderable scene element types.
     */
    enum class Element {
        Background = 1,
        Board,
        HandicapMark,
        WhiteStone,
        WhiteStoneLast,
        WhiteStoneTransparent,
        WhiteTerritory,
        BlackStone,
        BlackStoneLast,
        BlackStoneTransparent,
        BlackTerritory,
        PlacementMarker
    };

    /**
     * Only one ThemeRenderer is needed per application, this method returns
     * the singleton self.
     *
     * @return ThemeRenderer self
     */
    inline static ThemeRenderer* self()
    {
        static ThemeRenderer self;
        return &self;
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

signals:
    void themeChanged(const QString &);

private:
    QString m_currentTheme;         ///< Holds the current seleted theme
    QSvgRenderer *m_renderer;       ///< Converts SVG parts into pixmaps
};

} // End of namespace Kigo

#endif
