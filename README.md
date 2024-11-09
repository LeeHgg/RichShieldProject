# RichShieldProject

## Circle the Cat
### Microprocessor Application

#### 1. Problem description

##### 1.1. Game overview

This project implements the "Circle the Cat" game, where the objective is to trap a cat inside a circle of walls to prevent it from escaping the game board. The
hardware used includes the nrf52840dk board and the Open Smart Rich Shield Two board. The project utilizes the I2C LED matrix 16x8, rotary encoder, joystick, and battery display from the Open Smart Rich Shield. In the original game, the board consists of hexagonal tiles, and the cat moves to one of the six adjacent tiles. In this project, the I2C LED matrix from the Open Smart Rich Shield is not hexagonal but composed of rows and columns, so the cat can move only up, down, left, or right, excluding diagonal movements. This means that diagonal movement of the cat is not allowed. The characteristic of the original game, where the cat moves along the shortest path to escape the board while avoiding walls, is also applied in this project. Additionally, this project aims to add more developed features to the original game. The ultimate goal of this game is to successfully trap the cat attempting to escape by strategically placing walls.

##### 1.2. Game rules

The rules of the original game are as follows:
1. The cat always starts in the same position at the center.
2. Each time the player places a wall, the cat moves to one of the six adjacent
hexagonal tiles.
3. The cat moves along the shortest path to escape the game board.
4. The player loses if the cat escapes the game board, and wins if they successfully
trap the cat.

The rules for this project are as follows:
1. The cat starts at a random position within a specific range in the center of the LED matrix.
2. Each time the player places a wall, the cat moves in one of the four cardinal directions (up, down, left, right).
3. The cat cannot move diagonally.
4. The cat moves along the shortest path to escape the game board.
5. Turning the rotary encoder to the left generates two random walls without
moving the cat.
6. The player is given a total of seven lives. Each time the cat escapes the game
board, one life is lost. If all lives are lost, the player loses.
7. The player wins if they successfully trap the cat.

The key differences between the original game and this project lie in the shape of the tiles (hexagonal vs. a matrix of rows and columns) and the starting position of the cat within the LED matrix's specified central range. In this project, the cat's starting position is randomly chosen within this defined range each time the game starts. Additionally, new functionalities such as the ability to generate two random walls using an item, facilitated by the rotary encoder, have been added. The battery display is used to indicate the player's lives; failing to trap the cat results in decrementing the battery display, effectively reducing lives during gameplay. The player is given a total of seven lives, losing one each time the cat escapes the game board, with the game ending when all lives are lost.
The core rules of Circle the Cat remain consistent between the original game and this project: the cat seeks the shortest path to escape the game board, and the player attempts to encircle the cat with walls to prevent its escape. Failing to do so results in the player losing, while successfully trapping the cat leads to victory, as in the original game.
