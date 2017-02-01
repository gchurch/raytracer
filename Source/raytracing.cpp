#include <iostream>
#include <glm/glm.hpp>
#include <SDL.h>
#include "SDLauxiliary.h"
#include "TestModel.h"
#include "limits.h"

using namespace std;
using glm::vec3;
using glm::mat3;

//structure used to hold information about the intersection of a ray and a triangle
struct Intersection
{
	vec3 position;
	float distance;
	int triangleIndex;
};

/* ----------------------------------------------------------------------------*/
/* GLOBAL VARIABLES                                                            */

//Screen information
const int SCREEN_WIDTH = 500;
const int SCREEN_HEIGHT = 500;
SDL_Surface* screen;
int t;

//Scene information
vector<Triangle> triangles;

//Camera information
float focalLength = 500;
vec3 cameraPos(0,0,-3);
mat3 cameraRot(vec3(1,0,0),vec3(0,1,0),vec3(0,0,1));
float yaw = 0;

//Update information
float posDelta = 0.1;
float rotDelta = 0.1;


/* ----------------------------------------------------------------------------*/
/* FUNCTIONS                                                                   */
bool ClosestIntersection(vec3 start, vec3 dir, const vector<Triangle>& triangles, Intersection& closestIntersection);
void Update();
void Draw();


int main() {
	LoadTestModel(triangles);
	screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT );

	while( NoQuitMessageSDL() )
	{
		Draw();
		Update();
	}

	SDL_SaveBMP( screen, "screenshot.bmp" );
	return 0;
}

bool ClosestIntersection(vec3 start, vec3 dir, const vector<Triangle>& triangles, Intersection& closestIntersection) {
	//bool stating whether or not this ray intersects a triangle
	bool intersection = false;

	//iterates through all triangles
	for(unsigned int i = 0; i < triangles.size(); i++) {
		//triangles vertices
		vec3 v0 = triangles[i].v0;
		vec3 v1 = triangles[i].v1;
		vec3 v2 = triangles[i].v2;
		
		//basis vectors
		vec3 e1 = v1 - v0;
		vec3 e2 = v2 - v0;

		//b vector
		vec3 b = start - v0;

		//Cramer's rule to calculate t
		mat3 A(-dir, e1, e2);
		float detA = glm::determinant(A);
		mat3 At(b, e1, e2);
		float t = glm::determinant(At) / detA;
	
		//if the distance is greater than 0, i.e. the triangle is infront of the camera then continue
		if(t >= 0) {

			//Use Cramer's rule to calculate u
			mat3 Au(-dir, b, e2);
			float u = glm::determinant(Au) / detA;

			//Only continue if u meets the inequality conditions
			if( u > 0 && u < 1) {

				//Use Cramer's rule to calculate v
				mat3 Av(-dir, e1, b);
				float v = glm::determinant(Av) / detA;

				//If ray intersects triangle
				if(v > 0 && u + v < 1) {
					//if this triangle is closer than the current closest intersection
					if(t < closestIntersection.distance) {
						//set intersection flag to true
						intersection = true;
						//update closestIntersection flag
						closestIntersection.position = v0 + u * e1 + v * e2;
						closestIntersection.distance = t;
						closestIntersection.triangleIndex = i;
					}
				}
			}
		}
	}

	//return flag indicating whether ray intersects with any triangles
	return intersection;
}

void updateRotationMatrix() {
	//Calcuate new columns for the rotation matrix of the camera
	vec3 c1(cos(yaw), 0, -sin(yaw));
	vec3 c3(sin(yaw), 0, cos(yaw));
	//update rotation matrix columns
	cameraRot[0] = c1;
	cameraRot[2] = c3;
}

void Update()
{
	// Compute frame time:
	int t2 = SDL_GetTicks();
	float dt = float(t2-t);
	t = t2;
	cout << "Render time: " << dt << " ms." << endl;

	//get key presses and update camera position
	Uint8* keystate = SDL_GetKeyState(0);
	if(keystate[SDLK_UP])
	{
		//calculate the z-axis vector and move in the positive direction
		vec3 forward(cameraRot[2][0], cameraRot[2][1], cameraRot[2][2]);
		cameraPos.x += posDelta * forward.x;
		cameraPos.y += posDelta * forward.y;
		cameraPos.z += posDelta * forward.z;
	}
	if(keystate[SDLK_DOWN])
	{	
		//calculate the z-axis vector and move in the negative direction
		vec3 forward(cameraRot[2][0], cameraRot[2][1], cameraRot[2][2]);
		cameraPos.x -= posDelta * forward.x;
		cameraPos.y -= posDelta * forward.y;
		cameraPos.z -= posDelta * forward.z;
	}
	if(keystate[SDLK_LEFT])
	{
		//decrease the rotation angle and update rotation matrix 
		yaw -= rotDelta;
		updateRotationMatrix();
	}
	if(keystate[SDLK_RIGHT])
	{
		//increase the rotation angle and update the rotation matrix
		yaw += rotDelta;
		updateRotationMatrix();
	}
	
}

void Draw() {
	SDL_FillRect(screen, 0, 0);

	if(SDL_MUSTLOCK(screen)) {
		SDL_LockSurface(screen);
	}


	//Iterate through all pixels in window
	for(int y = 0; y < SCREEN_HEIGHT; y++) {
		for(int x = 0; x < SCREEN_WIDTH; x++) {
		
			//Calculate relative x and y positions of the pixel to the camera position
			float newX = (float) x - (float) SCREEN_WIDTH / 2;
			float newY = (float) y - (float) SCREEN_HEIGHT / 2;

			//the normalised ray vector (assuming no rotation of the camera)
			vec3 d(newX, newY, focalLength);
			d = d/sqrt((pow(newX,2) + pow(newY,2) + pow(focalLength,2)));

			//The new ray vector is the rotation matrix multiplied by the old ray vector
			d = cameraRot * d;
			
			//holds information about the closest intersection for this ray
			Intersection closest = {vec3(0,0,0), std::numeric_limits<float>::max(), -1};
	
			//If the ray intersects a triangle then fill the pixel
			//with the color of the closest intersecting triangle
			if(ClosestIntersection(cameraPos, d, triangles, closest) == true) {
				vec3 color = triangles[closest.triangleIndex].color;
				PutPixelSDL(screen, x, y, color);
			}

		}
	}

	if(SDL_MUSTLOCK(screen)) {
		SDL_UnlockSurface(screen);
	}

	SDL_UpdateRect(screen, 0, 0, 0, 0);

}
