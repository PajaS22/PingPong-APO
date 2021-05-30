---
title: 'README PingPong'
disqus: pingpong game
---
# Ping Pong

The Ping Pong game was created as a semestral project for a subject *Arcitecture of computers* at CTU Prague in May 2021.
The authors are Šimon Lehký and Pavel Svoboda.  
(c) 2021 All trademarks are the property of their respective owners.

To see the full version click *[here](https://hackmd.io/@APOsem/HyhP2kJqd)*.

## Table of Contents

[TOC]

## How to play

If you are a total beginner to the game, read this:

### Main menu control
* You can control the menu by rotating green knob.
* Select an option by clicking the green button on the green knob.
* The menu gives you three options:
    * Easy ... starts the game with an easy mode
    * Hard ... starts the game with a hard mode
    * Exit ... shutdowns the application

### In-game control
* Both players have their own knob.
* Players control their paddle by rotating the red or blue knob.
* When catched **tean bonus point**, the player can enlarge his paddle by clicking the button on the knob.

### What's the goal?
* Goal of the game is to score 16 points.
* The game ends when one of the players has 16 points at the end of the round.
* And how to get a point? Both players try to rebound the ball, when they fail to do so, the other player gets a point As we say: *he scores a goal!*
* When scoring a goal, a blue or red LED starts blinking to indicate that one of the players has hit the net.
* Another possibility how to get a point is by catching **yellow bonus balls**.

### Pause menu control
* To open the pause menu, press the green knob.
* The pause menu the gives you two options:
    * Resume ... continues the game
    * Exit ... ends the game instantly and returns the players back to the main menu

### Bonus balls
* Every 5 seconds a bonus ball spawns on the screen. Its position and color are randomly chosen and the ball can be either yellow or tean (*tean is light blue*).
* There can be up to five bonus balls at the same time in game.
* A **yellow bonus ball** gives player a point.
* A **tean bonus ball** gives the player a possibility to enlarge his paddle for the next 5 seconds. If not used, the advantage is passed to the following round and can be used then.


## Compilation and run process for dummies
1. Connect the MicroZed board to your local internet network.
2. Connect your PC to your local internet network.
3. Change *TARGET_IP* on line 18 in our Makefile to your IP adress.
4. Run this command in your terminal while being in the directory of our pingpong game (it will compile the project and send it to the MicroZed board).  
Then you must change the IP address in the command as well:  
`TARGET_IP=192.168.202.214 make run`
5. Enjoy the game!

## Preview of the MicroZed board

![](https://i.imgur.com/qwI8ZB1.png)  

## Preview of the main menu
{%youtube WaOxzRp3u4E%}


## Preview of the game

{%youtube CNlWv_n-qHY%}


## Appendix

:::info
**Find this document incomplete?** Leave us a comment!  
* Pavel Svoboda: <svobop60@fel.cvut.cz>   
* Šimon Lehký: <lehkysim@fel.cvut.cz>
:::

###### tags: `Documentation` `Game` `PingPong` `MicroZed` `Sixty hours of hard work`

