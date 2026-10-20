#pragma once

namespace MINTGGGameEngine
{

/**
 * \page tutorial_simple Tutorial: The First Game
 *
 *
 * \section tutorial_simple_overview Overview
 *
 * This tutorial will walk you through the steps necessary to create a first, very
 * simple game using the engine. By the end, you will have a **player** that you can
 * **move using buttons**, as well as pickups and hazards that fall from the top of
 * the screen and which you either have to **collect to gain score points**, or
 * **avoid in order to not lose a life**.
 *
 *
 * \section tutorial_simple_setup Setup
 *
 * Before we can get to programming, you first have to set up your system for
 * development, and get the
 * [skeleton project](https://github.com/alemariusnexus/MINTGGGameEngine_MyGame)
 * to work. Please see \ref setup for the basic steps, as this tutorial will **not**
 * **not** explain them separately. It assumes that you have a working development
 * environment for the platform you're using, can upload or run the code, and are
 * now ready to start writing code.
 *
 *
 * \section tutorial_simple_skelcode Skeleton Code
 *
 * \subsection tutorial_simple_skelcode_overview Overview
 *
 * Before we start writing our own code, **let's take a look at the code** that's
 * already present in the skeleton project. In this tutorial, we will only be
 * looking at **the file _main.cpp_**. All other files are irrelevant for us.
 *
 * The code in main.cpp is the following:
 *
 * \code{.cpp}
 * #include <MINTGGGameEngine.h>
 *
 * LOG_USE_TAG("main")
 *
 *
 *
 * void gameSetup()
 * {
 * }
 *
 * void gameLoop(float dt)
 * {
 * }
 *
 * void onCollision(const GameObjectCollision& coll)
 * {
 * }
 * \endcode
 *
 * All games using MINTGGGameEngine will have this basic structure. The code you see
 * here is written in **the programming language C++**, which is what this engine and all
 * games on it are written in.
 *
 * The first two non-empty lines are irrelevant for us now. We have to keep them there,
 * but can ignore them.
 *
 * The rest of the code defines three **C++ functions**. These are blocks of code
 * with a name. Each function starts with its ***header*** (e.g. `void gameSetup()`), and
 * continues with its ***body***, which is always surrounded by braces (`{}`). We don't
 * care about the header for now, other than the fact that it contains the function's name.
 *
 * The three functions in the file are named *gameSetup*, *gameLoop* and *onCollision*.
 * In documentation like this tutorial, these names are often written with parentheses,
 * to make it clear that they are functions: *gameSetup()*, *gameLoop()* and
 * *onCollision()*.
 *
 * The thing we care about right now is the *body* of these functions. As you can see,
 * **it is currently empty**, as there is no code between the braces (whitespace like
 * empty lines is ignored in C++). If there was code there, it would be executed by the
 * processor when the function is ***called***. The three functions in this file are
 * special functions that are *called* by the engine automatically.
 *
 * \subsection tutorial_simple_skelcode_functions The Main Game Functions
 *
 * As mentioned before, the three functions in this file are called by the engine. Each
 * one is called at different times, and serves different purposes. We will learn more
 * about them in the following sections, but here is a quick overview:
 *
 *  - ***gameSetup()***: Called **once when the game starts** (e.g. when the microcontroller
 *    gets powered on, or when the game application starts). All the setup code like
 *    spawning the player, loading sprites, defining buttons, or loading the first level
 *    will happen here.
 *  - ***gameLoop()***: Called **once every _frame_** that the game is running, after
 *    *gameSetup()* has finished. This function will be called multiple times per second,
 *    until the game is ended. It usually contains the main game logic, like "move the
 *    player two pixels to the right" or "spawn an enemy behind the player 20 seconds
 *    after the game starts".
 *  - ***onCollision()***: Called whenever the engine notices that **two game objects
 *    collide with each other**, e.g. a player with an enemy, a player with a pickup, or
 *    an enemy with a bullet. Can be used to decide what should happen when these objects
 *    collide, like scoring a point, or subtracting a life from the player.
 *
 * Keep in mind that **unless stated otherwise, the code you write must always be inside
 * the body of a function**. So most of your code should be between braces (`{}`). The
 * exceptions (that are currently relevant to us) are *global variables*, which we will see
 * shortly, and the definitions of functions themselves.
 *
 *
 * \section tutorial_simple_playerspawn Spawning a Player
 *
 * Now that we know the basics of the code in front of us, we can write our very first own
 * code. We want to **spawn a player**, i.e. create them and put them into the game world,
 * so that it appears on the screen. For this, we need three lines of code.
 *
 * First, we have to create a **global variable** for the player:
 *
 * \code{.cpp}
 * GameObject player;
 * \endcode
 *
 * **IMPORTANT**: Because it is a ***global*** variable, this code must be placed **outside**
 * any functions. Specifically, it should be placed on its own line, somewhere **before**
 * the first function in the file (e.g. between `LOG_USE_TAG("main")` and `void gameSetup()`),
 *
 * Note that this first line **ends with a semicolon**. This semicolon is required at the end
 * of each *statement* in C++ (unless it is followed by braces, `{}`), and forgetting it is a
 * classic beginner's mistake.
 *
 * That code defines a variable with the name `player`, with its *data type* being GameObject.
 * Almost all objects that are visible on the screen and/or interact with the game are
 * GameObjects. The name `player` can be chosen freely.
 *
 * To actually **create and spawn** the player, we need two more lines of code. The following
 * lines should be written <b>inside the _body_ of the _gameSetup()_</b> function:
 *
 * \code{.cpp}
 * player = GameObject::createCircle(20, 110, 3, Color::BLUE);
 * game.spawnObject(player);
 * \endcode
 *
 * As you might already guess, This creates a player that is a simple blue circle. Its radius
 * is 3 pixels, and it is spawned at the screen at position (20, 110). We'll talk more about
 * coordinates in the next section.
 *
 * This is all that's needed to spawn a player. You can now **upload your code** (or run it,
 * if you're using the desktop port). If you didn't make any mistakes, and all the hardware
 * is connected correctly, you should see your player: a small blue circle on the screen.
 *
 * \image html tutorial_simple_playerspawn_screen.png "The spawned player on the screen. it's not much, but it's three lines of code." width=30%
 *
 * The following is the full code in *main.cpp* that we have now. If you get any errors when
 * building your game, check that your code matches this:
 *
 * \code{.cpp}
 * #include <MINTGGGameEngine.h>
 *
 * LOG_USE_TAG("main")
 *
 * GameObject player;
 *
 * void gameSetup()
 * {
 *     player = GameObject::createCircle(20, 110, 3, Color::BLUE);
 *     game.spawnObject(player);
 * }
 *
 * void gameLoop(float dt)
 * {
 * }
 *
 * void onCollision(const GameObjectCollision& coll)
 * {
 * }
 * \endcode
 *
 *
 * \section tutorial_simple_screen The Game Screen
 *
 * \image html tutorial_simple_screen_layout.svg "Coordinate system of the game screen. All units in pixels." width=50%
 *
 * The image above illustrates how coordinates in the engine work. The black rectangle is
 * the screen, which has a **resolution of 160x128 pixels** in this example (the size of a
 * standard ST7735 screen as supported by the engine). The top-left corner of the screen
 * is the **origin point** with coordinates `(0, 0)`. Note that the y axis goes downward, so
 * the coordinates of the bottom-right pixel are `(159, 127)` (one less than `(160, 128)`,
 * because we start counting at 0).
 *
 * This coordinate system is used to specify where game objects are on screen, or where they
 * should move.
 *
 * In the last example, we wrote the following code:
 *
 * \code{.cpp}
 * player = GameObject::createCircle(20, 110, 3, Color::BLUE);
 * \endcode
 *
 * The first two numbers inside the parentheses (called to first two *parameters* to the
 * function *createCircle()*) are the x and y coordinate of the player, so it is spawned
 * at positon `(20, 110)`. The player is a circle, but note that **these are not the
 * coordinates of the circle's center, but those of the top-left corner of the circle's
 * bounding rectangle**, as you can see in the drawing above. This rule is true for all
 * game objects in the engine: Their position is always the top-left corner of their
 * bounding rectangle.
 *
 *
 * \section tutorial_simple_buttons Adding Buttons
 *
 * Now we have a player, but we can't control them yet (i.e. move them around). For that,
 * we need to add at least **two buttons**: one to move left, and one to move right.
 *
 * First, we need to **connect the physical buttons to the microcontroller** (for the desktop
 * port, we have to map them to keyboard keys in *config/desktop.json*). By default, the
 * engine expects them to be **active-low**. This means that they should be connected such
 * that the pin is at GND (0V) when the button is pressed. When it is not pressed, the engine
 * automatically enables a pull-up resistor by default, which pulls the pin to Vcc (the
 * supply voltage of the microcontroller).
 *
 * Once the buttons are connected, we have to **define them in our code**, inside the
 * *gameLoop()* function (it does not matter if we do this before or after spawning
 * the player):
 *
 * \code{.cpp}
 * game.input().defineButton("left", 16);
 * game.input().defineButton("right", 17);
 * \endcode
 *
 * `"left"` and `"right"` are the internal IDs of the buttons. You can choose them freely,
 * and we use those IDs to identify them leter. The numers `16` and `17` are the pin numbers
 * where the buttons are connected. **Make sure to enter your own pin numbers here**.
 *
 * Then, in the *gameLoop()* function, we can ask the engine whether one of those buttons
 * is pressed. If it is, we can execute some code. The following will move the player one
 * pixel to the right if the `"right"` button is pressed:
 *
 * \code{.cpp}
 * if (game.input().isButtonPressed("right")) {
 *     player.move(1, 0); // Move player 1 pixel in the positive x direction, and 0 pixel in the positive y direction.
 * }
 * \endcode
 *
 * There are a few new C++ constructs in this code:
 *
 *  1. The sequence `//` start a ***single-line comment***. This means that all text
 *     following these slashes until the end of the line is ignored by the compiler, so you
 *     can write anything you want there without changing what your game does. Comments are
 *     used to describe sections of code for humans that read it, the computer doesn't care
 *     about them.
 *  2. The <b>function \ref GameObject::move() "move()"</b> is used to move a GameObject a
 *     certain amount of pixels on the screen. Negative values can be used as well, to move
 *     it along the negative direction of the axis.
 *  3. The syntax `if (...) { ... }` is used to execute a block of code **only when a certain
 *     condition is fulfilled**. The general syntax is `if (CONDITION) { CODE }`, which will
 *     execute the *CODE* only when the *CONDITION* is true. In this case, the condition is
 *     `game.input().isButtonPressed("right")` (<i>is the "right" button pressed?</i>), and only
 *     if the answer is yes (`true`), the code `player.move(1, 0);` will be executed. If it
 *     is no (`false`), no code will be executed.
 *
 * Similar code can be used to make the `"left"` button work. Combined, the code in
 * *gameLoop()* for both buttons might look like this:
 *
 * \code{.cpp}
 * if (game.input().isButtonPressed("left")) {
 *     player.move(-1, 0);
 * }
 * if (game.input().isButtonPressed("right")) {
 *     player.move(1, 0);
 * }
 * \endcode
 *
 * You can now upload your code again and see if the buttons work. You should be able to move
 * the player.
 *
 *
 * \section tutorial_simple_barriers Screen Barriers
 *
 * We can now move the player. But as you might have experienced yourself, **you can move them
 * completely off the screen**. For many games, this is not what you want. Instead, you want
 * the edges of the screen to act as "barriers" that the player can't pass through. Let's
 * try to make that work.
 *
 * The idea is simple: We let the player move freely. But every frame, we **check if the player
 * is currently (partially) outside the screen**. If they are, we just **teleport them back**
 * exactly to the edge. Because we do this every frame, and the player is only shown on the
 * screen at the end of each frame, this brief moment of "being out of the screen" and the
 * teleport will not be visible to the player.
 *
 * The code for the left edge of the screen is the simplest:
 *
 * \code{.cpp}
 * if (player.getX() < 0) {
 *     player.setX(0);
 * }
 * \endcode
 *
 * The functions \ref GameObject::getX() "getX()" and \ref GameObject::setX() "setX()" are
 * used to read and change the x coordinate of the player's position. This is another *if
 * statement*, with the condition `player.getX() < 0` ("is the player's position on the x
 * axis less than 0?"). `0` is the x coordinate of the screen's left edge, so if the player's
 * position is lower than that, they are at least partially outside the screen on the left.
 *
 * Similar code can be written for the right edge:
 *
 * \code{.cpp}
 * if (player.getX() + player.getWidth() > game.getScreen().getWidth()) {
 *     player.setX(game.getScreen().getWidth() - player.getWidth());
 * }
 * \endcode
 *
 * The idea is the same, but as you can see, we have to do some calculations involving the
 * *width of the player* (which is twice its radius for circles) and the *width of the
 * screen*. The reason is that for the right screen edge, **we have to check if the _right_
 * edge** of the player is outside the screen, not the left edge. Similarly, the *setX()*
 * function changes the x coordinate of the player's <b><i>left</i> edge</b> (remember: GameObject
 * coordinates are always for their top-left corner), so we have to calculate the maximum
 * "allowable" x coordinate from the screen's and the player's width.
 *
 * We could have just a well written the last code snippet like this:
 *
 * \code{.cpp}
 * if (player.getX() + 6 > 160) { // Or even: player.getX() > 154
 *     player.setX(128 - 6); // Or just calculate it yourself and use 122
 * }
 * \endcode
 *
 * If our screen is 160 pixels wide, and our player has a diameter of 6 (twice the radius),
 * this code does exactly the same as the one shown before. The downside is that if we
 * ever decide to make the player bigger or smaller in the future, or change to a screen
 * with a different resolution, we would have to adjust these values by hand. The longer
 * snippet **adjust itself automatically to these changes and should thus be preferred**.
 *
 * If you haven't aleady, now would be a good time to **upload and test** the code again.
 * You should not be able to move the player outside the screen anymore.
 *
 *
 * \section tutorial_simple_pickups Pickups
 *
 * \subsection tutorial_simple_pickups_spawnfunction Our Own Spawn Function
 *
 * Now we have a moving player. But there's nothing for the player to do in our game. So
 * next, we will write code that spawns **pickups** at the top of the screen, which then
 * **fall down**. The player has to **collect them to gain score points**, which we will
 * also display on the screen.
 *
 * There are many ways to approach this. What we will do here is create a new function
 * (like the ones we already saw: *gameSetup()*, *gameLoop()* and *onCollision()*, which
 * spawns a pickup whenever it is called. The following will be our first attempt, which
 * we should again put **before any other functions**:
 *
 * \code{.cpp}
 * void spawnPickup()
 * {
 *     GameObject pickup = GameObject::createCircle(50, -10, 2, Color::GREEN);
 *     game.spawnObject(pickup);
 * }
 * \endcode
 *
 * We gave our own function the name *spawnPickup*. The code in its body should mostly be
 * familiar to you. It is basically the same as the one for spawning the player. Pickups
 * are also GameObjects, they just spawn at a different position, are a bit smaller, and
 * are green.
 *
 * One detail is noteworthy: The first line in the *body* starts with
 * `GameObject pickup = ...`. This time, we have the data type in front of out variable
 * name `pickup`. The reason is that this is a <b><i>local</i> variable</b>, i.e. it is only
 * valid within this function. This line just combines the two separate lines
 * `GameObject pickup;` and `pickup = GameObject::createCircle(50, -10, 2, Color::GREEN);`
 * into one line to be more compact.
 *
 * Note also that we spawn the pickup at a negative y position, outside the screen. This
 * is so the player doesn't see the pickup "pop into existence" when it spawns. It spawns
 * outside the screen and will then gradually move downwards into the screen.
 *
 * \subsection tutorial_simple_pickups_spawncall Calling the Function
 *
 * Now we have a function that spawns a pickup. But remember: The body of a function is
 * only executed if the function is *called*. We created this function ourselves, so we
 * are also responsible for calling it.
 *
 * When the game starts, we want to spawn the first pickup immediately. To do this, we
 * <b><i>call</i> the function from gameSetup()</b> like this:
 *
 * \code{.cpp}
 * spawnPickup();
 * \endcode
 *
 * You have seen several of these manual *function calls* already, e.g. when we created
 * and spawned the player, or when we moved the player. This is just the first time we
 * called a function that we created ourselves (the previous ones were all part of the
 * engine).
 * Note that the parentheses behind the function's name are mandatory, even if there's
 * nothing between them. They are what makes this an actual function *call*, instead of
 * just "mentioning the function's name in passing" without any further effect.
 *
 * We will call this function several more times in the following subsections, when we
 * want to spawn another pickup.
 *
 * \subsection tutorial_simple_pickups_move Moving the Pickups
 *
 * The first pickup now spawns, but it's outside the screen and not moving yet, so we
 * won't actually see it. The next step is to make it move. Unlike the player, the
 * pickups are always moving, without us having to press any button. The idea is simple
 * again: Every frame, move all spawned pickups a certain amount of pixels downward.
 *
 * \subsubsection tutorial_simple_pickups_move_tags Tagging the Pickups
 *
 * Before we can do this, we have to figure out a way to get a list of all pickups, so
 * we can do something with them (namely move them). The engine provides a function for
 * that, namely \ref Game::getGameObjectsWithTag() "getGameObjectsWithTag()". It gives
 * us a list of all spawned GameObjects with a certain tag.
 *
 * What's a tag? It's a label that we can attach to a GameObject. Each GameObject can
 * have many tags at the same time, and many GameObjects can have the same tag. Tags
 * can be used to identify **what type of object** it is. In our case, we want to give
 * the pickups the *pickup tag*, which just says "I'm a pickup!". Then we can ask the
 * engine to give us all objects with the *pickup tag*, and don't have to keep a list
 * of them ourselves.
 *
 * First, we have to define the tag. We do this as a global variable again, outside
 * any functions:
 *
 * \code{.cpp}
 * const int TagPickup = 1;
 * \endcode
 *
 * Tags are just integer numbers (thus the data type *int*). This line defines a variable
 * called *TagPickup* with a value of 1. Actually, the keyword `const` at the
 * beginning makes it a constant, not a variable. A constant is just a variable whose
 * value we can't change after first defining it.
 *
 * **Technical side note**: (*Don't worry if you don't get this explanation. It is not
 * necessary to know at this point*) - Why did we choose the value 1? In fact, we could
 * have chosen lots of different values. 1 is just the smallest we could have chosen. We
 * can't just choose any integer though. The important rule is: Tags always have to be
 * powers of two (1, 2, 4, 8, 16, ...). The reason is that the list of tags for an object
 * is stored internally as a [bit field](https://en.wikipedia.org/wiki/Bit_field), and
 * those only work when the individual values have exactly one non-zero bit set, which is
 * true only for powers of two.
 *
 * Lastly, we have to actually give our pickups this tag. For this, we modify the code
 * of our *spawnPickup()* function a bit:
 *
 * \code{.cpp}
 * void spawnPickup()
 * {
 *     GameObject pickup = GameObject::createCircle(50, -10, 2, Color::GREEN);
 *     pickup.setTag(TagPickup);
 *     game.spawnObject(pickup);
 * }
 * \endcode
 *
 * Note the new line calling the function \ref GameObject::setTag() "setTag()". This is
 * used to give a GameObject a certain tag.
 *
 * \subsubsection tutorial_simple_pickups_move_loop The Actual Movement Code
 *
 * Now we're ready to move the pickups. The basic code we want is the following:
 *
 * \code{.cpp}
 * pickup.move(0, 1);
 * \endcode
 *
 * This moves the `pickup` one pixel downwards. The problem is: Which pickup? Right now
 * we only have one, but we might have more than one at the same time in the future. We
 * want to write code that moves **all of the pickups** every frame.
 *
 * To do this, we use a ***loop***. A loop is a construct in C++ that is similar to the
 * *if statement* we already saw, in that it executes a bunch of code only if a condition
 * is true. Unlike the *if statement* however, a loop will **keep executing that code
 * over and over** as long as the condition stays true.
 *
 * We will actually use a special loop here, where the condition is not that clear at
 * first glance. In *gameLoop()*, we do the following:
 *
 * \code{.cpp}
 * for (GameObject pickup : game.getGameObjectsWithTag(TagPickup)) {
 *     pickup.move(0, 1);
 * }
 * \endcode
 *
 * As expected, we see the `move()` call we discussed before. But this time, it appears
 * inside the *body* of a so-called *for loop* (more precisely: a *range-based for loop*).
 * Let's look at this code in more detail:
 *
 *  1. A for loop always has the form `for (VARIABLE_DEFINITION : LIST_OF_VALUES) { CODE }`.
 *  2. The *VARIABLE_DEFINITION* just specifies the data type and name for a new variable
 *     that we use for the loop. Here, we want to loop over pickups, which are GameObjects,
 *     so we create the appropriate variable.
 *  3. The *LIST_OF_VALUES* can be any "list of things", in this case, it's a list of all
 *     game objects that have the *pickup tag*.
 *  4. The *CODE* can be any sequence of code, just like with the earlier *if statement*.
 *
 * Now we know the individual parts of the *for loop*, but what does it actually do? A
 * *for loop* will go through its entire *LIST_OF_VALUES*. For each value in the list, it
 * will assign it to the variable in *VARIABLE_DEFINITION*, and then execute the *CODE*
 * once with that specific value. It then moves on to the next value, assigning the
 * variable, executing the code, and so on - until the list is done. So in short: **A for
 * loop executes a section of code once for every element of a list**. In our example:
 * The *move()* call will be made once for every pickup in the game.
 *
 * This is all the code we need to have our pickups move. You can now upload and test your
 * code again. The pickups should appear, and they should move downward. For now, they will
 * just keep moving downards out of the screen, which we will change next.
 *
 * \subsection tutorial_simple_pickups_respawn Respawning Pickups
 *
 * As mentioned, the pickups currently fall downwards indefinitely, even outside the screen.
 * What we probably want is something else: Once a pickup moved out of the bottom of the
 * screen, it should despawn, and a new one should spawn on top instead.
 *
 * This is simple to do: We use a similar idea as we used for our screen barrier code for
 * the player: Detect when a pickup is outside the screen by its coordinates. We want to
 * check this every frame, for every pickup, so we change the code of our *for loop* to
 * the following:
 *
 * \code{.cpp}
 * for (GameObject pickup : game.getGameObjectsWithTag(TagPickup)) {
 *     pickup.move(0, 1);
 *     if (pickup.getY() > game.getScreen().getHeight()) {
 *         game.despawnObject(pickup);
 *         spawnPickup();
 *     }
 * }
 * \endcode
 *
 * The *if statement* is similar to the one we used for the screen barrier, just using
 * the y coordinate instead of the x coordinate, and the screen's height instead of its
 * width. Also, we want to respawn the object only when it's **fully** outside the screen,
 * which makes the calculations simpler.
 *
 * If the pickup is outside the screen, `game.despawnObject(pickup);` will despawn it, i.e.
 * remove it from the game. `spawnPickup();` then spawns a new one at the top. Here we
 * see why we defined our pickup spawning code as a function before: We now call it from
 * two different places (here in the loop, and in *gameSetup()*), but we don't have to
 * duplicate its code, just the function call.
 *
 * You can test the program again now. The pickup should respawn when it falls out of the
 * screen.
 *
 * \subsection tutorial_simple_pickups_rand Randomizing Pickup Position
 *
 * The respawn works, but the new pickup will always spawn at the same position on the top
 * of the screen. That's a bit boring. Instead, we want to spawn each pickup at a random
 * position instead. We can do this by modifying the first line in *spawnPickup()*, and
 * calculating a random number for its x coordinate:
 *
 * \code{.cpp}
 * GameObject pickup = GameObject::createCircle(game.randInt(0, 150), -10, 2, Color::GREEN);
 * \endcode
 *
 * Instead of the fixed x value of `50`, we now calculate it as `game.randInt(0, 150)`. The
 * \ref Game::randInt() "randInt()" function generates a random integer value between the
 * two parameter values, i.e. between 0 and 150. Instead of 150, we could have calculated
 * a value from the screen width again here, which would have been the cleaner solution,
 * and should be used in actual games.
 *
 * We could do the same for the y coordinate as well, just with only negative numbers. This
 * will make them spawn different distances outside the screen, which creates a random
 * "spawn delay" (the spawn is not actually delayed, but the time that they enter the visible
 * screen area is). Then the line would look something like this:
 *
 * \code{.cpp}
 * GameObject pickup = GameObject::createCircle(game.randInt(0, 150), game.randInt(-60, -10), 2, Color::GREEN);
 * \endcode
 *
 *
 * \section tutorial_simple_collision Collision Detection
 *
 * \subsection tutorial_simple_collision_overview General Idea
 *
 * We now have a moveable player, and an infinite stream of randomly spawning pickups. That's
 * great, but the player still can't interact with the pickups. What we want is for the
 * pickups to despawn when the player "catches" them, and for the player to gain some score
 * points when that happens.
 *
 * For this, we have to use **collision detection**. We have to find out when the player
 * collides with a pickup (i.e. when their shapes overlap). This is a maths problem. The player
 * and the pickup are both circles, and that case is trivial to calculate based on their
 * center positions and radii.
 *
 * **However, we don't actually have to do any collision math** in our game. The engine
 * already handles collision detection for us. It checks all GameObjects for callision with
 * each other, and when it detects a collision, **it calls the function onCollision()**, which
 * is the final function of the original skeleton code we haven't used yet. Now we will.
 *
 * \subsection tutorial_simple_collision_code The Collision Code
 *
 * Let's first look at the function *onCollision()* instead:
 *
 * \code{.cpp}
 * void onCollision(const GameObjectCollision& coll)
 * {
 * }
 * \endcode
 *
 * Its header, i.e. the first line, is a bit longer and more complex than the one for the
 * other functions we used before. Of interest is now the part
 * `const GameObjectCollision& coll`, which is called the ***parameter*** of the function.
 * This parameter is a variable with name *coll* whose value is set when the function is
 * called. This function is called by the engine itself, so it fills the variable with a
 * sensible value. The *coll* variable has the data type *GameObjectCollision*, which is
 * an object that stored information about a collision between two game objects. The
 * exact syntax of this parameter is not important to us right now.
 *
 * We can use this parameter and ask it for various information about the collision that
 * the engine just detected. What we are interested in here is: Which two game objects
 * just collided? We are interested in collisions **between the player and a pickup**.
 * We can check for this specific constellation of objects with the following code:
 *
 * \code{.cpp}
 * if (coll.isInvolved(player)) { // Is the player involved in the collision? If yes, then ...
 *     GameObject other = coll.getOther(player); // ... get the other object involved in the collision (i.e. not the player)
 *     if (other.hasTag(TagPickup)) { // If that other object is a pickup, then ...
 *         // ... do something. We'll look at this code here next.
 *     }
 * }
 * \endcode
 *
 * Here we see two *if statements*. More importantly, they are *nested if statements*,
 * meaning one is inside the body of the other. That means that the second *if statement*
 * is only even evaluated if the first one is already *true*. In other words: The
 * `... do something.` line is only executed if the player is involved in the collision
 * **and** the other invovled object is a pickup, not if only one of these conditions is
 * true.
 *
 * *if statements* can nest as much as you want, and they can also nest with things like
 * *for loops*. This is why the braces around them are so important: You need to know
 * which parts of the code are part of the *if statement*'s body, and which are outside
 * of it. Also take note of the amount of spaces at the start of each line. In C++ (unlike
 * for example the programming language *Python*), this *indentation* is not actually
 * relevant for the meaning of the code to a computer. It is still very important to
 * get it right, because it makes the code much more readable for humans, especially those
 * that are used to the *indentation rules*. Indentation shows the "depth" of a line
 * of code, i.e. far it is nested in things like *functions*, *if statements* or
 * *loops*.
 *
 * Our code isn't quite finished yet. We can now detect collision between the player and
 * a pickup. But what should we do if that happens?
 *
 * Well, first we want to do the same thing as when the pickup moves off the screen at
 * the bottom: respawn it. So our code becomes:
 *
* \code{.cpp}
 * if (coll.isInvolved(player)) { // Is the player involved in the collision? If yes, then ...
 *     GameObject other = coll.getOther(player); // ... get the other object involved in the collision (i.e. not the player)
 *     if (other.hasTag(TagPickup)) { // If that other object is a pickup, then ...
 *         game.despawnObject(other);
 *         spawnPickup();
 *     }
 * }
 * \endcode
 *
 * This is the third (and for now, final) time we called our *spawnPickup()* function.
 *
 * You can test the code again. The player should now be able to collect the pickups,
 * and they should despawn immediately when collected.
 *
 *
 * \section tutorial_simple_score Score Points
 *
 * \subsection tutorial_simple_score_counting Score Counter
 *
 * We almost have a full game now. The player can move, pickups will spawn, and the
 * player can actively "hunt" for them by collecting them. The only thing missing is
 * some incentive for the player to actually go for the pickups. We need a **score
 * counter** that increases when the player collects pickups.
 *
 * The basic score counter is very simple. We define a new variable for the score,
 * and then increase its value whenever the player collects a pickup. First, we
 * **create another global variable** (outside any functions):
 *
 * \code{.cpp}
 * int score = 0;
 * \endcode
 *
 * It's an integer, and it starts at 0 when the game begins.
 *
 * Then, whenever we want to increase the score, we do something like this:
 *
 * \code{.cpp}
 * score += 10; // Short form of: score = score+10;
 * \endcode
 *
 * This increases the score by 10.
 *
 * We put this code inside our *onCollision()* function, at the same point where we
 * respawn the pickup.
 *
 * \subsection tutorial_simple_score_text Displaying the Score
 *
 * You could test this code, and it would count the score, but you won't actually
 * see that happening. That's because the `score` variable is an internal value in
 * the processor. The user doesn't automatically get to see it. For the user to see
 * their score, we have to **add text to the screen** that displays it.
 *
 * We create text in a similar way to how we created the first player. The main
 * difference is that text is one of the few things visible on screen that is
 * **not a GameObject**. Instead, it uses the type \ref Text. We first create
 * another global variable:
 *
 * \code{.cpp}
 * Text scoreText;
 * \endcode
 *
 * And then create the actual text in *gameSetup()*:
 *
 * \code{.cpp}
 * scoreText = Text(2, 2);
 * game.addText(scoreText);
 * \endcode
 *
 * Note the lack of any mention of *GameObject* and the usage of
 * \ref Game::addText() "addText()" instead of
 * \ref Game::spawnObject() "spawnObject()". The text is created at position
 * `(2, 2)`, which is the top-left corner of the screen, with a few pixels of
 * space for good measure.
 *
 * We haven't yet told the game what the text should actually say, so if we were
 * to test the program now, the text would be invisible. The problem is that the
 * text is not fixed. It has to change whenever the player's score changes. For
 * that reason, we actually set the text in *gameLoop()* instead of *gameSetup()*,
 * so we can update it every frame. Here's how:
 *
 * \code{.cpp}
 * scoreText.setText("Score: "s + to_string(score));
 * \endcode
 *
 * The syntax is a bit weird here. First, we have the "fixed text" that will always
 * be displayed (`"Score: "s`). Such fixed text (a *string literal*) must always be
 * inclosed in double quotation marks (`"..."`). Then after it, we have to add the
 * score. To do this, we have to convert the score (which has the data type `int`) to
 * a ***string*** first. *string* is the technical term for a bunch of characters that
 * are arranged one after the other (short for *string of characters*). Or in plain
 * English: text. The conversion from *int* to *string* is done using the *to_string()*
 * function. The `+` operator puts two *strings* together to create a new, longer
 * *string* (*concatenation*). Unlike some scripting languages like Python, **C++ is
 * very particular about the data types involved in string operations**, so the
 * intuitively correct code `"Score" + score` would actually lead to an error. The
 * exact details are out of the scope of this tutorial. Just know that you have to be
 * careful when working with text in C++.
 *
 * Now you can test the program again. The score should be displayed, and it should
 * update when the player collects a coin.
 *
 * **You now have a full game!** Although it is of course very simple, it already
 * showed off a lot of the features and C++ constructs that you will need to develop
 * your own, more complex games. It did of course not show all of the engine's features,
 * nor all of what's to know about C++, *by a long shot*. But it should be a good
 * starting point for playing around with your own ideas for simple games.
 *
 * The following sections will show how to add a few more features to the game. If you
 * want, you can look through them, or you can move on to your own game ideas, try
 * another tutorial, or look through other parts of the engine's documentation to
 * discover what more there is to do.
 *
 *
 * \section tutorial_simple_todo TODO: More game features
 *
 * (Not written yet)
 */

}
