/*
    SPDX-FileCopyrightText: 2008 Sascha Peilicke <sasch.pe@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KIGO_THEMERENDERER_H
#define KIGO_THEMERENDERER_H

#include <QObject>
#include <QString>

class KGameThemeProvider;
class KGameTheme;

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
    ~ThemeRenderer() override;

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
     * @param theme the theme to load
     */
    void loadTheme(const KGameTheme *theme);

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

    KGameThemeProvider *themeProvider() const;

Q_SIGNALS:
    void themeChanged();

private:
    KGameThemeProvider *m_themeProvider;
    QSvgRenderer *m_renderer;       ///< Converts SVG parts into pixmaps
};

} // End of namespace Kigo

#endif
