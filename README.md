# PhysicsBox

A sandbox for general (2D) physics.

---

## Dependencies
This game was built with the help of: 

- [raylib](https://github.com/raysan5/raylib)
- [raygui](https://github.com/raysan5/raygui)

[raylib](https://github.com/raysan5/raylib) is a game development framework.
[raygui](https://github.com/raysan5/raygui) is a GUI tool for **raylib**. 
Both were written by [raysan5](https://github.com/raysan5) on Github.

## Install & Build
First, make sure the above dependencies are installed. Clone the repository wherever you want:
```
git clone https://github.com/BorisZupancic/PhysicsBox.git
```
(Not too sure about the rest of this stuff...)

My current Makefile sucks, so building the game only works on Linux systems.
If you know what you're doing, compile ```main.c``` then run the outputted executable.

#### For Linux
Enter the terminal, navigate to the ```../PhysicsBox``` directory, and run the following command:
```
make && ./main
```
This should compile the source code for the game (```make```), then immediately call
the executable ```./main``` to open the game. After running this command once, you should be able to just call the executable and run the game.
