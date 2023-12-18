# bomberman
This repository contains the final project for the "Introduction to Robotics" course, taken in my third year at the Faculty of Mathematics and Informatics of the University of Bucharest.

## Table of contents
- [Intro](#Intro)
- [Game Description](#Game-Description)
- [How to Play](#How-to-Play)
- [Components Used](#Components-Used)
- [Photos](#Photos)
- [Video](#Video)

## Intro
Growing up Bomberman was one of my favorite games, and I remember playing countless hours of an online multiplayer version against my brother. (I rarely won🙂) <br><br>
![Original bomberman photo](photos/original_bomberman.png)<br><br>
When I was assigned the task of creating a game of choice on Arduino, I knew it would be a great occasion to rebuild one of my old-school favorites.
It proved to be quite a challenge, as I was constrained by the little memory of the Arduino Uno, but nevertheless a fun one.

## Game Description
The game of Bomberman takes place on a 2D grid, where there is a set of walls that you have to destroy in order to complete the level. Different versions of the game may incorporate obstacles, power-ups, multiplayer modes, and more.
In my version, the player has to destroy all the walls in four different rooms as fast as possible, which implies finding a way to destroy as many walls at once as possible. There is a way to increase the bomb radius, but it might not be immediately obvious to the player.

## How to Play
1. Start the game from the LCD menu
2. Use the joystick to move the player (the blinking dot) around the grid
3. Place bombs by pressing the joystick
4. Destroy all walls in the four rooms as fast as possible
5. Fight for a place on the leaderboard :rocket:

## Components Used
- Arduino Uno board
- medium size breadboard
- small breadboard
- 8 x 8 LED Matrix
- MAX7219 driver
- 10µF electrolytic capacitor
- 0.1µF ceramic capacitor
- 10k ohm resistor
- 16 x 2 LCD Screen
- active buzzer
- 2x 330 ohm resistor
- joystick
- hook-up wires

## Photos
![Photo 1 of my Arduino setup](photos/setup1.jpeg)
![Photo 2 of my Arduino setup](photos/setup2.jpeg)

## Video
https://youtu.be/zTOJoWmgitI
