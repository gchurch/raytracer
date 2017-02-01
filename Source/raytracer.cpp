#include <iostream>
#include <glm/glm.hpp>
#include <SDL.h>
#include "SDLauxiliary.h"
#include "TestModel.h"
#include "limits.h"

using namespace std;
using glm::vec3;
using glm::mat3;

struct Intersection
{
	vec3 position;
	float distance;
	int triangleIndex;
};

/* ----------------------------------------------------------------------------*/
/* GLOBAL VARIABLES                                                            */

const int SCREEN_WIDTH = 100;
const int SCREEN_HEIGHT = 100;
SDL_Surface* screen;
int t;
vector<Triangle> triangles;
float focalLength = 100;
vec3 cameraPos(0,0,-3);
mat3 cameraRot(vec3(1,0,0),vec3(0,1,0),vec3(0,0,1));
float yaw = 0;

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

		vec3 b = start - v0;

		//Cramer's rule
		mat3 A(-dir, e1, e2);
		mat3 At(b, e1, e2);
		float t = glm::determinant(At) / glm::determinant(A);
	
		if(t >= 0) {

			vec3 x = glm::inverse(A) * b;
		
			float u = x.y;
			float v = x.z;


			//If ray intersects triangle
			if(u > 0 && v > 0 && u + v < 1) {
				//if this triangle is closer than the current closest intersection
				if(t > 0 && t < closestIntersection.distance) {
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

	//return flag indicating whether ray intersects with any triangles
	return intersection;
}

void updateRotationMatrix() {
	vec3 c1(cos(yaw), 0, -sin(yaw));
	vec3 c3(sin(yaw), 0, cos(yaw));
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
		vec3 forward(cameraRot[2][0], cameraRot[2][1], cameraRot[2][2]);
		cameraPos.x += 0.1 * forward.x;
		cameraPos.y += 0.1 * forward.y;
		cameraPos.z += 0.1 * forward.z;
	}
	if(keystate[SDLK_DOWN])
	{	
		vec3 forward(cameraRot[2][0], cameraRot[2][1], cameraRot[2][2]);
		cameraPos.x -= 0.1 * forward.x;
		cameraPos.y -= 0.1 * forward.y;
		cameraPos.z -= 0.1 * forward.z;
	}
	if(keystate[SDLK_LEFT])
	{
		yaw -= 0.1;
		updateRotationMatrix();
	}
	if(keystate[SDLK_RIGHT])
	{
		yaw += 0.1;
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

			//the normalised ray vector
			vec3 d(newX, newY, focalLength);
			d = d/sqrt((pow(newX,2) + pow(newY,2) + pow(focalLength,2)));

			d = cameraRot * d;
			
			//holds information about the closest intersection for this ray
			Intersection closest = {vec3(0,0,0), std::numeric_limits<float>::max(), -1};
	
			if(ClosestIntersection(cameraPos, d, triangles, closest) == true) {
				vec3 color = triangles[closest.triangleIndex].color;
				PutPixelSDL(screen, x, y, color);
				//printf("position: (%d, %d)\n", x, y);
				//printf("distance: %f\n", closest.distance);
			}

		}
	}

	if(SDL_MUSTLOCK(screen)) {
		SDL_UnlockSurface(screen);
	}

	SDL_UpdateRect(screen, 0, 0, 0, 0);

}
