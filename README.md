# C++ Raytracer

This is a raytracer graphics program created as part of the Computer Graphics unit at the Unversity of Bristol. 

The program is written in C++ and makes use of GLM and SDL.

![Original](https://github.com/gchurch/raytracer/blob/master/screenshots/soft_shadows.bmp)

## Linux Setup

Install SDL

```
sudo apt-get install libsdl1.2-dev
```

Clone the GLM github repository

```
$ git clone https://github.com/g-truc/glm
```

Set the GLMDIR environment variable to your GLM directory's path

e.g.

```
$ export GLMDIR=/home/user/glm
```

## Installation

Clone the raytracer github repository

```
$ git clone https://github.com/gchurch/raytracer
```

## Compilation

Navigate to the raytracer repository

To compile the code, run the command:

```
$ make
```

## Execution

Make sure you are in the raytracer repository and issue the command:

```
$ make run
```

## Controls

You can move the camera's view by using the up, down, left and right keys

You can also move the light source's position by using the w, s, a and d keys
