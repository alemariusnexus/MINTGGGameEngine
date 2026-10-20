#pragma once

namespace MINTGGGameEngine
{

/**
 * \page core Core Concepts
 *
 *
 * \section core_overview Overview
 *
 * This page will describe some of the core and most central concepts of the engine.
 * More specialized parts of the engine will be described on their own pages, like
 * \ref input, \ref audio or \ref physics.
 *
 * Note that this page is **not intended as a tutorial** on how to get started with
 * the engine. If you haven't yet, it is best to look at \ref setup and
 * \ref tutorial_simple first, and then return here afterwards if you're interested
 * to learn more about the concepts you need to create your own game.
 *
 *
 * \section core_startup Startup Sequence
 *
 * By default, the engine itself will handle the entire sequence necessary for a
 * game to start. This means that **the engine will define the application entry
 * point(s)** (e.g. `setup()` and `loop()` for Arduino, `app_main()` on ESP-IDF, or
 * `main()` on desktop). It will also initialize and start all relevant engine
 * components like the screen, storage, input, audio, networking and others
 * automatically.
 *
 * It is possible for advanced game projects to change this startup sequence, though
 * this is currently not documented, and the API for it is not currently stable. If
 * you're interested in the details, take a look at the files *src/core/Startup.cpp*
 * and *src/core/DefaultEngine.cpp*.
 *
 *
 * \section core_config Configuration
 *
 * The engine uses **C++ header files** for certain parts of its configuration, especially
 * things like pin configurations, screen setup, and other hardware- and
 * platform-specific settings.
 *
 * For this reason, every game project must have a file called **_mgg_config.h_**, which
 * the engine itself will include. This file (or files included by it) should setup all
 * the necessary configuration values. For most configuration options, a sensible default
 * value is provided, so you only have to add your own configuration values if you want
 * to deviate from this default (which you very often will though, especially for the
 * hardware configuration like pin numbers or screen type).
 *
 * A more detailed guide on this configuration will hopefully be created in the future.
 * In the mean time, you can take a look at the configuration options defined in the
 * *src/config* directory of the engine's source code.
 *
 *
 * \section core_game The Game Class
 *
 * The engine is based around the \ref Game class. Each game project will currently
 * have a single instance of this class. Unless the game deviates from the default
 * startup sequence described in \ref core_startup, **this object is available as a
 * global variable called `game`**.
 *
 * The Game class serves various functions in the engine, which will be described
 * in separate sections and tutorials. Its most central functions are the following:
 *
 *  - It provides access to the objects that handle the various **engine subsystems**
 *    through functions like \ref Game::input(), \ref Game::storage(),
 *    \ref Game::audio() or \ref Game::screen().
 *  - It holds the **list of all game objects** (see \ref core_gameobject below) that
 *    are currently spawned in the game, and is responsible for spawning and
 *    despawning them.
 *  - It handles the **high-level game loop** (e.g. calling gameLoop(), rendering game
 *    objects, drawing to the screen, detecting collision, ...), though it does not
 *    implement the details of any of these operations.
 *
 * Note that this list is far from exhaustive. See the API documentation of \ref Game
 * for more details.
 *
 *
 * \section core_gameobject Game Objects (and their parts)
 *
 * \subsection core_gameobject_class The GameObject Class
 *
 * Other than \ref Game, the most important class of the engine is \ref GameObject (also
 * spelled *game object* in this documentation). It represents **almost any object that can
 * be visible on the screen or participate in physics** (e.g. for collision detection).
 * Players, enemies, pickups, bullets, obstacles, platforms, and backgrounds are all game
 * objects, to name just a few.
 *
 * Each game object has a set of properties that defines it, including but not limited to
 * the following:
 *
 *  - A **position** in the game world, along with other transforms (e.g. rotation, scale
 *    factor, flip direction)
 *  - A **sprite** (optional), which defines the visual appearance of the object on the screen
 *    (e.g. a rectangle, circle, or bitmap)
 *  - A **collider** (optional), which defines the *hit box* of the object for collision
 *    detection or ray casting.
 *  - A set of **tags** to classify the object
 *
 * These various parts of a game object will be described below, or on separate documentation
 * pages.
 *
 * To actually become active in the game (e.g. be displayed on screen or participate in the
 * physics calculations), a game object must be **_spawned_** into the game using
 * \ref Game::spawnObject(). When it is no longer needed, it can be **despawned** using
 * \ref Game::despawnObject().
 *
 * Here's a simple example for creating and spawning a GameObject for the player
 * character (a simple blue circle).
 *
 * \code{.cpp}
 *      GameObject player = GameObject::createCircle(40, 40, 10, Color(0, 0, 255)); // x, y, radius, color
 *      game.spawnObject(player);
 * \endcode
 *
 * See the \ref GameObject class for various ways to create game objects.
 *
 * Such a GameObject could then be moved depending on user input every frame like so:
 *
 * \code{.cpp}
 *      if (game.input().isButtonPressed("left")) {
 *          player.move(-2, 0); // Move left 2 pixels
 *      } else if (game.input().isButtonPressed("right")) {
 *          player.move(2, 0); // Move right 2 pixels
 *      }
 * \endcode
 *
 *
 * \subsection core_gameobject_sprite Sprites
 *
 * The \ref Sprite class controls a game object's **appearance on the screen**. It can
 * be manipulated using \ref GameObject::setSprite() and \ref GameObject::getSprite().
 * A GameObject's sprite can be changed at any point, even while it is spawned.
 *
 * Not every game object needs to have a sprite, e.g. an object that merely acts as
 * an invisible wall.
 *
 * In addition to simple geometric shapes like rectangles or circles, sprites can
 * also use **arbitrary bitmaps** to display a game object. Bitmaps will be discussed
 * later in \ref core_bitmaps.
 *
 *
 * \subsection core_gameobject_collider Colliders
 *
 * The \ref Collider class controls a game object's **_hit box_** for the physics parts
 * of the engine, like **collision detection or ray casting**, and is optimized for
 * fast calculations. It is separate from the object's sprite. Currently, only very
 * simple colliders are supported (axis-aligned rectangles and circles). See the
 * \ref Collider class for more details.
 *
 * Not every game object needs to have a collider. A game object that just acts as
 * the game's background image will usually not need one, for example.
 *
 *
 * \subsection sec_gameobject_tags Tags
 *
 * Each GameObject can have one or more tags associated with it. A tag is like
 * a **marker used for categorizing the "type" of object** that it is. For example,
 * all enemies in the game might have a tag `TagEnemy`, and all bullets shot by
 * the player might have a tag `TagPlayerBullet`. Tagging is useful to later
 * identify *what* a certain GameObject is, or to fetch all GameObjects of a given
 * type that are currently spawned.
 *
 * Tags are **_bit flags_** that can be freely defined by the user. It's a good idea
 * to define all tags in a global enum like this:
 *
 * \code{.cpp}
 *      enum GameObjectTag {
 *          TagPlayer       = 1,
 *          TagEnemy        = 2,
 *          TagPlayerBullet = 4
 *      };
 * \endcode
 *
 * Note that because they are bit flags, an individual tag's value **must always be
 * a unique integer power of two**, though its value otherwise does not matter.
 *
 * A bullet shot by the player can then be tagged accordingly:
 *
 * \code{.cpp}
 *      GameObject bullet = GameObject::createCircle(player.getX(), player.getY()-5, 3); // x, y, radius
 *      bullet.setTag(TagPlayerBullet); // Tag the GameObject as a player bullet
 *      game.spawnObject(bullet);
 * \endcode
 *
 * Using these tags, you can write code like the following to despawn all player
 * bullets currently in the game:
 *
 * \code{.cpp}
 *      game.despawnObjects(game.getGameObjectsWithTag(TagPlayerBullet)); // Despawn all player bullets
 * \endcode
 *
 * Or you could move all enemies a few pixels down the screen every frame like
 * this:
 *
 * \code{.cpp}
 *      // Do this every frame
 *      for (GameObject enemy : game.getGameObjectsWithTag(TagEnemy)) { // Loop over all enemies
 *          enemy.move(0, 2); // Move 2 pixels down
 *      }
 * \endcode
 *
 *
 * \section core_bitmaps Bitmaps and Colors
 *
 * A bitmap is a **rectangular two-dimensional image**, made up of individual pixels.
 * Each pixel in turn has a color.
 *
 * Colors are represented by the \ref Color class. Each object of the Color class
 * stores a single RGB color, and possibly an alpha value (usually used to define
 * transparency, but sometimes used for other purposes as well). The Color class
 * provides helper functions for converting to and from different color formats,
 * like RGB888 and RGB565.
 *
 * Bitmaps can store their pixel data internally in several different formats. This
 * is to allow for quicker drawing of bitmaps to a screen (e.g. every frame) by
 * storing the pixel data in the display's native format. In addition to RGB color,
 * bitmaps also support transparency by way of an alpha channel. Currently, the
 * engine only supports binary transparency (either fully transparent or fully
 * opaque, nothing in between).
 *
 * There are many ways to create or load a bitmap. The following example shows how
 * to load a bitmap from a BMP image file on an SD card:
 *
* \code{.cpp}
 *      Bitmap bmp = Bitmap::loadBMP(game.screen(), "/sdcard/enemy.bmp"); // Load file from SD card
 *      GameObject enemy = GameObject::createBitmap(0, 0, bmp); // x, y, bitmap
 *      enemy.setTag(TagEnemy); // Must be defined by the user
 *      game.spawnObject(enemy);
 * \endcode
 *
 * The game's screen can be passed when loading the bitmap, to convert to bitmap
 * into the screen's native pixel format. This is not necessary, but makes drawing
 * the bitmap significantly faster, and is thus **_strongly_ recommended** for
 * bitmaps that may be drawn every frame.
 *
 * If an SD card or similar file storage medium is not available, a bitmap can also
 * be created from a C/C++ array of raw pixel data, which can be generated by tools
 * like [image2cpp](https://javl.github.io/image2cpp/):
 *
 * \code{.cpp}
 *      Bitmap bmp(16, 16, epd_bitmap_player); // width, height, data
 * \endcode
 *
 * This code assumes that the pixel data is stored in RGB565 format without an alpha
 * channel. This can be changes by passing additional parameters to the
 * \ref Bitmap::Bitmap() "Bitmap()" constructors.
 *
 *
 * \section core_implicitsharing Implicit Sharing
 *
 * To simplify game code and shield game programmers from the notoriously
 * beginner-unfriendly topic of C/C++ pointers, many (but not all!) of the core game
 * classes use **_Implicit Sharing_**. This means that **copies of such objects will
 * still refer to the same object**, and will not create new objects. Consider the
 * following code:
 *
 * \code{.cpp}
 * GameObject player = GameObject::createCircle(10, 10, 3, Color::BLUE);
 * GameObject player2 = player;
 * \endcode
 *
 * In C++ jargon, the second line create a *copy* of the object from `player`, and
 * stores this object in the new variable `player2`. However, since GameObject uses
 * *implicit sharing*, `player` and `player2` still both refer to the **_same_**
 * game object. So the code above does **not** actually create a second player, but
 * just makes the same player object known under two different "names", `player`
 * and `player2`.
 *
 * *Implicit sharing* also means that **copying such objects is cheap**, so you do
 * not have to worry about passing them as pointers or references for increased
 * performance. Implicit sharing uses C++ shared pointers, so memory management
 * is automatic in most cases as well.
 *
 * Other classes using *implicit sharing* include (but are not limited to):
 * \ref Sprite, \ref Bitmap, \ref Text and \ref AudioClip.
 *
 *
 * \section core_text Text
 *
 * The engine provides support for displaying simple text on the screen. Such text is
 * represented by the \ref Text class.
 *
 * Text is the most prominent example of a "thing" that is visible on the screen, but
 * **isn't a GameObject**. As such, text does not have all the features that game
 * objects have. For example, it can not currently be rotated, mirrored, or have a hit
 * box.
 *
 * Example of simple text:
 *
 * \code{.cpp}
 *      Text scoreText = Text(100, 2); // x, y
 *      scoreText.setColor(Color(0, 255, 0)); // Green text
 *      scoreText.setScale(2); // Make all characters twice as large as the font specifies them
 *      scoreText.setText("Score: 0"); // Initial text
 *      game.addText(scoreText);
 * \endcode
 *
 * More details about text rendering will be documented in the future.
 *
 *
 * \section core_epilog Further Information
 *
 * This concludes the overview of the most important core concepts and classes of the
 * engine. For further information, take a look at the other documentation chapters,
 * depending on the specific topics or engine subsystems you need for your game.
 */

}
