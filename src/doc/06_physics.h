#pragma once

namespace MINTGGGameEngine
{

/**
 * \page physics Physics
 *
 *
 * \section physics_overview Overview
 *
 * A full physics engine is **not currently implemented** in MINTGGGameEngine, and maybe
 * never will. However, certain physics-related helpers and engine parts are included, which
 * will be described in the following sections.
 *
 *
* \section physics_collision Collision Detection
 *
 * Collisions between GameObjects is automatically checked every frame, and a
 * function is called for each pair of GameObjects that collide each frame. The
 * function can be set with \ref Game::setCollisionCallback().
 *
 * Here's an example of a collision callback function:
 *
 * \code{.cpp}
 *      void onCollision(const GameObjectCollision& coll) {
 *          if (coll.isInvolved(player)) { // If the player GameObject is involved in the collision
 *              GameObject other = coll.getOther(player); // Get the GameObject that the player collided with
 *              if (other.hasTag(TagEnemy)) { // If that other object is an enemy
 *                  game.despawnObject(other); // Despawn the enemy
 *                  playerNumLives--; // Player loses a life
 *                  player.setPosition(playerStartX, playerStartY); // Respawn player
 *              }
 *          }
 *      }
 * \endcode
 *
 *
* \section physics_raycasting Ray Casting
 *
 * The engine supports simple ray casting: It can calculate the intersections
 * between a ray (line segment) and GameObjects, calculating which objects
 * were hit in what order, along with the intersection points.
 *
 * The following example casts a ray from the top-left to the bottom-right
 * corner of the screen, and prints information about all objects that were
 * hit to the console. It only checks intersections with objects that have
 * the TagRayCast tag.
 *
 * \code{.cpp}
 *		Vec2 rayStart(0, 0);
 *		Vec2 rayEnd(screen.getWidth(), screen.getHeight());
 *		RayCastResult res = game.castRay (
 *				rayStart, rayEnd,
 *				game.getGameObjectsWithTag(TagRayCast)
 *				);
 *		for (RayCastHit hit : res.getHits()) {
 *			GameObject hitObject = hit.gameObject; // What was hit?
 *			Serial.printf (
 *					"Ray %s object at (%.1f, %.1f), distance %.1f\r\n",
 *					hit.entering ? "enters" : "exits",
 *					hit.hitPoint.x(), hit.hitPoint.y(),
 *					hit.rayOffset
 *					);
 *		}
 * \endcode
 *
 * A ray can be cast against all spawned objects, or a list of objects. The
 * latter is recommended if there are many GameObjects in the world that don't
 * matter for ray casting, as it reduces the amount of calculation that has to
 * be done and increases performance.
 *
 *
* \section physics_gravity Gravity & Jumping
 *
 * While the engine does not include a physics engine, a simple helper class
 * exists that can simulate a GameObject that has a velocity influenced by
 * gravity.
 *
 * For an example of how to use this feature, take a look at
 * [the DemoPlatformer](https://github.com/alemariusnexus/MINTGGGameEngine_Demos)
 * demo on GitHub.
 */

}
