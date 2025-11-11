#pragma once

#include "Globals.h"

#include "AudioClip.h"
#include "Bitmap.h"
#include "Collider.h"
#include "Color.h"
#include "Game.h"
#include "GameObjectCollision.h"
#include "GameObject.h"
#include "InputEngine.h"
#include "Screen.h"
#include "ScreenST7735.h"
#include "Sprite.h"
#include "Vec2.h"


/**
 * \mainpage MINTGGGameEngine - A Simple 2D Game Engine for Microcontrollers
 *
 * 
 * \section sec_overview Overview
 *
 * This is a simple 2D game engine as an Arduino library, intended for larger
 * microcontrollers, color displays and simple peripheral components like
 * buttons, piezo speakers or analog joysticks.
 *
 * It is mainly developed for ESP32 microcontrollers and their variants, and
 * targets very specific components. It is not intended to be highly portable,
 * and relies on relatively specific components for some of its functionality.
 * Its focus is on simplicity, with a relatively simple API useful for
 * not-quite-beginners. It is not intended as a fully-featured game engine, but
 * more for quick and simple learning projects.
 *
 * It was developed by me, David Lerch, as a teacher for my "Microcontrollers"
 * student courses at the Berufliche Schulen Groß-Gerau and the MINT-Zentrum
 * Groß-Gerau in Germany.
 *
 * 
 * \section sec_links Links
 *
 * This documentation is for the engine itself. You can find the source code of
 * [the engine on Github](https://github.com/alemariusnexus/MINTGGGameEngine).
 * There is also a
 * [skeleton Arduino project](https://github.com/alemariusnexus/MINTGGGameEngine_MyGame)
 * for an empty game, and a
 * [Shoot 'em up](https://github.com/alemariusnexus/MINTGGGameEngine_ShootEmUp)
 * that was developed as a proof-of-concept.
 *
 *
 * \section sec_features Features
 *
 * The engine broadly provides the following features:
 *  - Spawning, positioning and movement of **GameObjects** in a 2D scene
 *  - **Rendering of 2D sprites** (circles, rectangles, bitmaps) on a simple screen (currently support ST7735 controllers in RGB565 mode)
 *  - **Collision detection** with circles and axis-aligned rectangles as basic shapes
 *  - Simple single-channel, single-voice **audio playback** on a piezo speaker
 *  - Handling of simple **input devices**. Currently only buttons, with Joystick support planned. Supports MCP23009 IO expanders.
 *
 *
 * \section sec_concepts Basic Concepts
 *
 * The engine is based around the \ref MINTGGGameEngine::Game "Game" class. Each
 * game project should create a single instance of this class, and call its
 * \ref MINTGGGameEngine::Game::begin() "begin()" method for setup. The skeleton game
 * project on Github already does most of the initialization in the engine.cpp
 * file, hiding it from the user.
 *
 * Otherwise, the most important class is \ref MINTGGGameEngine::GameObject "GameObject".
 * It represents any object that is visible on the screen and/or participates in
 * collision detection (e.g. players, enemies, pickups, obstacles, backgrounds,
 * ...). Each GameObject has a \ref MINTGGGameEngine::Sprite "Sprite" object
 * that determines its visual properties, and a \ref MINTGGGameEngine::Collider "Collider"
 * object that describes its collision shape. GameObjects need to be
 * \ref MINTGGGameEngine::Game::spawnObject() "spawned" into the game to be
 * visible and/or collidable.
 *
 * Here's a simple example for creating and spawning a GameObject for the player
 * character (a simple blue circle).
 *
 * \code{.cpp}
 *      GameObject player = GameObject::createCircle(40, 40, 10, Color(0, 0, 255)); // x, y, radius, color
 *      player.setTag(TagPlayer); // TagPlayer must be defined by the user (for later finding a GameObject by its tag)
 *      game.spawnObject(player);
 * \endcode
 *
 * Such a GameObject can then be moved depending on user input every frame like
 * so:
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
 * \section sec_tags Tags
 *
 * Each GameObject can have one or more tags associated with it. A tag is like
 * a marker used for categorizing the "type" of object that it is. For example,
 * all enemies in the game might have the tag TagEnemy, and all bullets shot by
 * the player might have the tag TagPlayerBullet. Tagging is useful to later
 * identify what a certain GameObject is, or to fetch all GameObjects of a given
 * type that are currently spawned.
 * 
 * Tags are bit flags that can be freely defined by the user. It's a good idea
 * to define all tags in a global enum like this:
 *
 * \code{.cpp}
 *      enum GameObjectTag {
 *          TagPlayer       = (1 << 0),
 *          TagEnemy        = (1 << 1),
 *          TagPlayerBullet = (1 << 2)
 *      };
 * \endcode
 *
 * A bullet shot by the player can then be tagged accordingly:
 *
 * \code{.cpp}
 *      GameObject bullet = GameObject::createCircle(player.getX(), player.getY()-5, 3); // x, y, radius
 *      bullet.setTag(TagPlayerBullet); // Tag the GameObject as a player bullet
 *      game.spawnObject(bullet);
 * \endcode
 *
 * Using these tags, you can write code like to following to despawn all player
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
 * \section sec_sprites Sprites
 *
 * Each GameObject has an associated \ref MINTGGGameEngine::Sprite "Sprite" object.
 * The sprite determines how the GameObject is rendered on the screen. Sprites
 * can be simple geometric shapes like circles and rectangles, or they can be
 * arbitrary color bitmaps. Bitmap sprites can be loaded from raw RGB565 data,
 * or from simple BMP image files on an SD card (if any is attached).
 *
 * If an SD card is not available, the RGB data can be included in the code by
 * using a converter like [image2cpp](https://javl.github.io/image2cpp/). Make
 * sure to use a draw mode of "Horizontal - 2 bytes per pixel (565)".
 *
 * Here's an example for how to load a Bitmap sprite from a BMP file on an
 * SD card.
 *
 * \code{.cpp}
 *      Bitmap bmp = Bitmap::loadBMP("/enemy.bmp"); // Load file from SD card
 *      GameObject enemy = GameObject::createBitmap(0, 0, bmp); // x, y, bitmap
 *      enemy.setTag(TagEnemy); // Must be defined by the user
 *      game.spawnObject(enemy);
 * \endcode
 *
 * Alternatively, to load an image from code generated by image2cpp linked
 * above, the Bitmap can be created like this (assuming a 16x16 pixel image):
 *
 * \code{.cpp}
 *      Bitmap bmp(16, 16, epd_bitmap_player); // width, height, data
 * \endcode
 *
 *
 * \section sec_collision Collision Detection
 *
 * Collisions between GameObjects is automatically checked every frame, and a
 * function is called for each pair of GameObjects that collide each frame. The
 * function can be set with \ref MINTGGGameEngine::Game::setCollisionCallback() "Game::setCollisionCallback()".
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
 * \section sec_input Input
 *
 * Input is handled by the \ref MINTGGGameEngine::InputEngine "InputEngine",
 * which must be accessed through the \ref MINTGGGameEngine::Game::input() "Game::input()"
 * method. At game startup, buttons must be defined using \ref MINTGGGameEngine::InputEngine::defineButton() "defineButton()"
 * and its variants. Each button is assigned a string ID, through which it is
 * later identified.
 *
 * Example for defining simple buttons (directly connected to microcontroller):
 *
 * \code{.cpp}
 *      game.input().defineButton("left", 16); // "left" button on pin 16 (active-low, pull-up)
 *      game.input().defineButton("right", 17); // "right" button on pin 16 (active-low, pull-up)
 * \endcode
 *
 * The current state of a button can be checked at any time, and button
 * combinations can be registered to call a function whenever they occur. To
 * check whether a button is pressed:
 *
 * \code{.cpp}
 *      if (game.input().isButtonPressed("left")) {
 *          // "left" is pressed
 *      }
 *      if (game.input().isButtonPressed("right")) {
 *          // "right" is pressed
 *      }
 * \endcode
 *
 * To call a function when a button combination is pressed:
 *
 * \code{.cpp}
 *      // On the global level:
 *      void onUltraBlast() {
 *          game.despawnObjects(game.getGameObjectsWithTag(TagEnemy)); // Despawn all enemies (BOOM!)
 *      }
 * 
 *      ...
 *
 *      // Somewhere during game setup (assumes "a", "b" and "start" buttons have been defined):
 *      game.input().defineButtonCombo({"a", "b", "start"}, onUltraBlast); // Combo: a+b+start
 * \endcode
 *
 *
 * \section sec_text Text
 *
 * Simple on-screen text rendering is provided by the \ref MINTGGGameEngine::Text "Text" class.
 * A Text object can be added to the game by calling \ref MINTGGGameEngine::Game::addText() "Game::addText()".
 * The text rendering is currently very basic and highly dependent on the
 * display-specific graphics library used in the backend.
 *
 * Example of simple text:
 *
 * \code{.cpp}
 *      Text scoreText = Text(100, 2); // x, y
 *      scoreText.setColor(Color(0, 255, 0)); // Green text
 *      scoreText.setSize(2);
 *      scoreText.setText("Score: 0"); // Initial text
 *      game.addText(scoreText);
 * \endcode
 *
 *
 * \section sec_audio Audio
 *
 * A simple \ref MINTGGGameEngine::AudioEngine "AudioEngine" is included. It
 * plays back \ref MINTGGGameEngine::AudioClip "AudioClips" in real-time,
 * supporting both background music and effect clips. The engine is very basic,
 * outputting only one frequency at a time. It is targeted towards simple piezo
 * speakers as output devices.
 *
 * Example of defining and later playing a simple sound effect:
 *
 * \code{.cpp}
 *      // On global level
 *      AudioClip deathClip;
 *
 *      // Somewhere during game setup
 *      deathClip.setTempo(200); // Clip tempo: 200 time units per minute
 *      deathClip.note(NOTE_G4, 1); // Play note G4 for one time unit
 *      deathClip.note(NOTE_Ds4, 1); // Play note D sharp 4 for one time unit
 *      deathClip.note(NOTE_C4, 1); // Play note C4 for one time unit
 *      deathClip.pause(3); // Final pause for 3 time units (before background music resumes, if any)
 *
 *      // When player is hit by enemy
 *      game.audio().playClip(deathClip);
 * \endcode
 *
 * For background music, a clip can be looped endlessly, and made to be
 * interrupted whenever a higher-priority sound effect occurs:
 *
 * \code{.cpp}
 *      // Assumes an AudioClip object called backgroundAudio was previously created.
 *      game.audio().playClip(backgroundAudio, AudioEngine::Priority::Background, true, true); // clip, priority, loop, advanceInBackground
 * \endcode
 */
