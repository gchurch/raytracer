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
vec3 cameraPos(0,0,-3.001);
mat3 cameraRot(vec3(1,0,0),vec3(0,1,0),vec3(0,0,1));
float yaw = 0;

//Light information
vec3 lightPos(0, -0.5, -0.7);
vec3 lightColor = 14.f * vec3(1,1,1);
vec3 indirectLight = 0.5f * vec3(1,1,1);

//Update information
float posDelta = 0.1;
float rotDelta = 0.1;
float lightDelta = 0.1;

//Floating point inaccuracy constant
float epsilon = 0.00001;

/* ----------------------------------------------------------------------------*/
/* FUNCTIONS                                                                   */
bool ClosestIntersection(vec3 start, vec3 dir, const vector<Triangle>& triangles, Intersection& closestIntersection);
void Update();
void Draw();
vec3 DirectLight(const Intersection& i);


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

	//make sure that the direction vector is normalized
	dir = normalize(dir);

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

		//A matrix
		mat3 A(-dir, e1, e2);
		float detA = glm::determinant(A);

		//Cramer's rule to calculate t
		mat3 At(b, e1, e2);
		float t = glm::determinant(At) / detA;
	
		//if the distance is greater than 0, i.e. the triangle is infront of the camera then continue
		if(t > epsilon) {

			//Use Cramer's rule to calculate u
			mat3 Au(-dir, b, e2);
			float u = glm::determinant(Au) / detA;

			//Only continue if u meets the inequality conditions
			if( u > -epsilon && u <= 1 + epsilon) {

				//Use Cramer's rule to calculate v
				mat3 Av(-dir, e1, b);
				float v = glm::determinant(Av) / detA;

				//If ray intersects triangle
				if(v > -epsilon && u + v <= 1 + epsilon) {
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
	//Calcuate new columns for the camera's rotation matrix
	cameraRot[0] = vec3(cos(yaw), 0, -sin(yaw));
	cameraRot[2] = vec3(sin(yaw), 0, cos(yaw));
}

//Calculate the Euclidean distance between the two given vectors
float distanceBetweenPoints(vec3 a, vec3 b) {
	return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2) + pow(a.z - b.z, 2));
}

//Caluclate the dot product between the two given vectors
float dotProduct(vec3 a, vec3 b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

//Calculate the normalised direction vector from the given vector to the lightsource
vec3 unitVectorToLightSource(vec3 a) {
	vec3 v(lightPos.x - a.x, lightPos.y - a.y, lightPos.z - a.z);
	return normalize(v);
}

//Output the illumination of the point in the intersection
vec3 DirectLight(const Intersection& i) {
	//Pi constant
	const float pi = 3.1415926535897;

	//distance from intersection point to light source
	float radius = distanceBetweenPoints(i.position, lightPos);

	//The power per area at this point
	vec3 B = lightColor / ((float) 4 * pi * (float) pow(radius,3));

	//unit vector describing normal of surface
	vec3 n = triangles[i.triangleIndex].normal;

	//unit vector describing direction from surface point to light source
	vec3 r = unitVectorToLightSource(i.position);

	//fraction of the power per area depending on surface's angle from light source
	vec3 D = B * max(dotProduct(r,n),0.0f);

	//trace ray from intersection point to lightsource, if closest intersection distance is less than distance to light
	//source then give give this point no direct illumination. This creates shadow effect
	Intersection closest = {vec3(0,0,0), std::numeric_limits<float>::max(), -1};
	if(ClosestIntersection(i.position, r, triangles, closest)) {
		//If the closest intersection is closer than the light source then set the illumination to 0
		if(closest.distance < radius + epsilon) {
			D = vec3(0,0,0);
		}
	}

	return D;
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
		//calculate the z-axis vector and move camera in the positive direction
		vec3 forward(cameraRot[2][0], cameraRot[2][1], cameraRot[2][2]);
		cameraPos += posDelta * forward;
	}
	if(keystate[SDLK_DOWN])
	{	
		//calculate the z-axis vector and move camera in the negative direction
		vec3 forward(cameraRot[2][0], cameraRot[2][1], cameraRot[2][2]);
		cameraPos -= posDelta * forward;
		
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

	//Move light position depending on key press
	if(keystate[SDLK_w])
	{
		lightPos.z += lightDelta;
	}
	if(keystate[SDLK_s])
	{
		lightPos.z -= lightDelta;
	}
	if(keystate[SDLK_a])
	{
		lightPos.x -= lightDelta;
	}
	if(keystate[SDLK_d])
	{
		lightPos.x += lightDelta;
	}
	if(keystate[SDLK_q])
	{
		lightPos.y -= lightDelta;
	}
	if(keystate[SDLK_e])
	{
		lightPos.y += lightDelta;
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
			vec3 d = normalize(vec3(newX, newY, focalLength));

			//The new ray vector is the rotation matrix multiplied by the old ray vector
			d = cameraRot * d;
			
			//holds information about the closest intersection for this ray
			Intersection closest = {vec3(0,0,0), std::numeric_limits<float>::max(), -1};
	
			//If the ray intersects a triangle then fill the pixel
			//with the color of the closest intersecting triangle
			if(ClosestIntersection(cameraPos, d, triangles, closest) == true) {
				//row
				vec3 color = triangles[closest.triangleIndex].color;
				//D
				vec3 light = DirectLight(closest);

				//Assuming diffuse surface, the light that gets reflected is the color vector * the light vector plus
				//the indirect light vector where the * operator denotes element-wise multiplication between vectors.
				vec3 R = color * (light + indirectLight);
				PutPixelSDL(screen, x, y, R);
			}

		}
	}

	if(SDL_MUSTLOCK(screen)) {
		SDL_UnlockSurface(screen);
	}

	SDL_UpdateRect(screen, 0, 0, 0, 0);

}
