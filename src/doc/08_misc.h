#pragma once

namespace MINTGGGameEngine
{

/**
 * \page misc Miscellaneous
 *
 *
 * \section misc_overview Overview
 *
 * This page will describe miscellaneous features of the engine, i.e. features that do not
 * currently fit into other categories. They may be moved to more relevant sections in the
 * future.
 *
 *
* \section misc_camera Camera & Scrolling
 *
 * The engine knows the concept of a camera. This camera defines what part
 * of the scene is actually drawn on the screen. The camera's position can be
 * changed, which is useful for creating scrolling effects. The following shows
 * how to implement side-scrolling using the left and right buttons:
 *
 * \code{.cpp}
 *		if (game.input().isButtonPressed("left")) {
 *			game.scroll(1, 0);
 *		}
 *		if (game.input().isButtonPressed("right")) {
 *			game.scroll(-1, 0);
 *		}
 * \endcode
 *
 * Similarly, the position of the camera can also be directly set. The following
 * code shows how to make the camera follow a player GameObject, such that the
 * player always appears in the middle of the screen:
 *
 * \code{.cpp}
 *		// Executed every frame, at the end of gameLoop():
 *		Vec2 playerCenter = player.getCenterPosition();
 *		game.setCameraOffset (
 *				playerCenter.x() - screen.getWidth()/2,
 *				playerCenter.y() - screen.getHeight()/2
 *				);
 * \endcode
 */

}
