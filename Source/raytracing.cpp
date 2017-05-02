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
	int objectIndex;
	int triangleIndex;
};

/* ----------------------------------------------------------------------------*/
/* GLOBAL VARIABLES                                                            */

float PI = 3.1415926535897;

//Screen information
const int SCREEN_WIDTH = 500;
const int SCREEN_HEIGHT = 500;
SDL_Surface* screen;
int t;

//Scene information
vector<Object> objects;

//Camera information
float focalLength = 500;
vec3 cameraPos(0,0,-3.001);
mat3 cameraRot(vec3(1,0,0),vec3(0,1,0),vec3(0,0,1));
float yaw = 0;

//Light information
vec3 lightCentre(0, -0.5, -0.7);
float lightRadius = 0.1;
vec3 lightColor = 10.f * vec3(1,1,1);
vec3 indirectLight = 0.25f * vec3(1,1,1);

//Update information
float posDelta = 0.1;
float rotDelta = 0.1;
float lightDelta = 0.1;

//The number of samples taken in antialiasing
int antiAliasingCells = 4;

//Floating point inaccuracy constant
float epsilon = 0.00001;

//Statistics
int numRayBoxTests = 0;
int numRayTrianglesTests = 0;
int numRayTrianglesIntersections = 0;
int numPrimaryRays = 0;

/* ----------------------------------------------------------------------------*/
/* FUNCTIONS                                                                  */
void Draw();
void Update();
bool ObjectIntersection(vec3 start, vec3 dir, const Object& object);
bool ClosestIntersection(vec3 start, vec3 dir, const vector<Object>& objects, Intersection& closestIntersection);
bool PointInShadow(vec3 start, vec3 dir, const vector<Object>& objects, float radius);
void updateRotationMatrix();
vec3 DirectLight(const Intersection& i);
vector<vec3> CalculateLightPoints();
void getArrayOfDirectionVectors(float x, float y, int n, vec3 dir[]);
void raycasting();

const vector<vec3> lightPoints = CalculateLightPoints();

int main() {
	LoadTestModel(objects);
	screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT );

	while( NoQuitMessageSDL() )
	{
		Draw();
		Update();
	}

	SDL_SaveBMP( screen, "screenshot.bmp" );	

	return 0;
}

void Draw() {
	SDL_FillRect(screen, 0, 0);

	if(SDL_MUSTLOCK(screen)) {
		SDL_LockSurface(screen);
	}

	raycasting();

	if(SDL_MUSTLOCK(screen)) {
		SDL_UnlockSurface(screen);
	}

	SDL_UpdateRect(screen, 0, 0, 0, 0);

}

void Update()
{
	// Compute frame time:
	int t2 = SDL_GetTicks();
	float dt = float(t2-t);
	t = t2;
	printf("Render time:                                   %f ms.\n", dt);
	printf("Total number of triangles:                     %d\n", (int) (objects[0].triangles.size() + objects[1].triangles.size() + objects[2].triangles.size()));
	printf("Total number of primary rays:                  %d\n", numPrimaryRays);
	printf("Total number of bounding box tests:            %d\n", numRayBoxTests);
	printf("Total number of ray-triangles tests:           %d\n", numRayTrianglesTests);
	printf("Total number of ray-triangles intersections:   %d\n", numRayTrianglesIntersections);
	printf("\n");

	numPrimaryRays = 0;
	numRayBoxTests = 0;
	numRayTrianglesTests = 0;
	numRayTrianglesIntersections = 0;


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
		lightCentre.z += lightDelta;
	}
	if(keystate[SDLK_s])
	{
		lightCentre.z -= lightDelta;
	}
	if(keystate[SDLK_a])
	{
		lightCentre.x -= lightDelta;
	}
	if(keystate[SDLK_d])
	{
		lightCentre.x += lightDelta;
	}
	if(keystate[SDLK_q])
	{
		lightCentre.y -= lightDelta;
	}
	if(keystate[SDLK_e])
	{
		lightCentre.y += lightDelta;
	}
}


bool ObjectIntersection(vec3 start, vec3 dir, const Object& object) {

	numRayBoxTests++;

	//Bounding box
	vec3 Pmin = object.Pmin;
	vec3 Pmax = object.Pmax;
	
	float txmin = (Pmin.x - start.x) / dir.x;
	float txmax = (Pmax.x - start.x) / dir.x;

	if(txmin > txmax) {
		float tmp = txmin;
		txmin = txmax;
		txmax = tmp;
	}

	float tymin = (Pmin.y - start.y) / dir.y;
	float tymax = (Pmax.y - start.y) / dir.y;

	if(tymin > tymax) {
		float tmp = tymin;
		tymin = tymax;
		tymax = tmp;
	}

	if((txmin > tymax) || (tymin > txmax)) {
		return false;
	}

	if(tymin > txmin) {
		txmin = tymin;
	}

	if(tymax < txmax) {
		txmax = tymax;
	}

	float tzmin = (Pmin.z - start.z) / dir.z;
	float tzmax = (Pmax.z - start.z) / dir.z;

	if(tzmin > tzmax) {
		float tmp = tzmin;
		tzmin = tzmax;
		tzmax = tmp;
	}

	if((txmin > tzmax) || (tzmin > txmax)) {
		return false;
	}

	if(tzmin > txmin) {
		txmin = tzmin;
	}

	if(tzmax < txmax) {
		txmax = tzmax;
	}
	
	return true;
}

bool ClosestIntersection(vec3 start, vec3 dir, const vector<Object>& objects, Intersection& closestIntersection) {

	//Increment the variable holding the total number of primary rays
	numPrimaryRays++;

	//bool stating whether or not this ray intersects a triangle
	bool intersection = false;

	//make sure that the direction vector is normalized
	dir = normalize(dir);

	for(unsigned int j = 0; j < objects.size(); j++) {
	
	
		if(ObjectIntersection(start, dir, objects[j])) {

			//iterates through all triangles
			for(unsigned int i = 0; i < objects[j].triangles.size(); i++) {
				//increment the variable counting the number of triangle ray intersection tests
				numRayTrianglesTests++;
	
				//triangles vertices
				vec3 v0 = objects[j].triangles[i].v0;
				vec3 v1 = objects[j].triangles[i].v1;
				vec3 v2 = objects[j].triangles[i].v2;
		
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

							//Increment the variable containing the total number of triangle ray intersections
							numRayTrianglesIntersections++;
	
							//if this triangle is closer than the current closest intersection
							if(t < closestIntersection.distance) {
								//set intersection flag to true
								intersection = true;
								//update closestIntersection flag
								closestIntersection.position = v0 + u * e1 + v * e2;
								closestIntersection.distance = t;
								closestIntersection.objectIndex = j;
								closestIntersection.triangleIndex = i;
							}
						}
					}
				}
			}
		}
	}

	//return flag indicating whether ray intersects with any triangles
	return intersection;
}

bool PointInShadow(vec3 start, vec3 dir, const vector<Object>& objects, float radius) {

	//Increment the variable holding the total number of primary rays
	numPrimaryRays++;

	//make sure that the direction vector is normalized
	dir = normalize(dir);

	for(unsigned int j = 0; j < objects.size(); j++) {
	
	
		if(ObjectIntersection(start, dir, objects[j])) {

			//iterates through all triangles
			for(unsigned int i = 0; i < objects[j].triangles.size(); i++) {
				//increment the variable counting the number of triangle ray intersection tests
				numRayTrianglesTests++;
	
				//triangles vertices
				vec3 v0 = objects[j].triangles[i].v0;
				vec3 v1 = objects[j].triangles[i].v1;
				vec3 v2 = objects[j].triangles[i].v2;
		
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

							//Increment the variable containing the total number of triangle ray intersections
							numRayTrianglesIntersections++;
	
							//if this triangle is closer than the current closest intersection
							if(t < radius + epsilon) {
								//set intersection flag to true
								return true;
							}
						}
					}
				}
			}
		}
	}

	//return flag indicating whether ray intersects with any triangles
	return false;
}

void updateRotationMatrix() {
	//Calcuate new columns for the camera's rotation matrix
	cameraRot[0] = vec3(cos(yaw), 0, -sin(yaw));
	cameraRot[2] = vec3(sin(yaw), 0, cos(yaw));
}

vector<vec3> CalculateLightPoints() {
	vector<vec3> points;
	points.push_back(lightCentre + vec3(lightRadius,0,0));
	points.push_back(lightCentre + vec3(-lightRadius,0,0));
	points.push_back(lightCentre + vec3(0,lightRadius,0));
	points.push_back(lightCentre + vec3(0,-lightRadius,0));
	points.push_back(lightCentre + vec3(0,0,lightRadius));
	points.push_back(lightCentre + vec3(0,0,-lightRadius));
	return points;
}

//Output the illumination of the point in the intersection
vec3 DirectLight(const Intersection& i) {

	//distance from intersection point to light source
	float radius = length(i.position - lightCentre);

	//r is the unit vector describing direction from surface point to light source
	vec3 v(lightCentre.x - i.position.x, lightCentre.y - i.position.y, lightCentre.z - i.position.z);
	vec3 r = normalize(v);

	vec3 D(0,0,0);

	//trace ray from intersection point to lightsource, if intersection distance is less than distance to light
	//source then give give this point no direct illumination. This creates shadow effect
	if(!PointInShadow(i.position, r, objects, radius)) {
		//The power per area at this point
		vec3 B = lightColor / (4 * PI * pow(radius,3));

		//unit vector describing normal of surface
		vec3 n = objects[i.objectIndex].triangles[i.triangleIndex].normal;

		//fraction of the power per area depending on surface's angle from light source
		D = B * max(dot(r,n),0.0f);
	}
	return D;
}

//Stochastic sampling for antialiasing (could implement distance based weights)
void getArrayOfDirectionVectors(float x, float y, int n, vec3 dir[]) {

	int rowLength = sqrt(n);

	float cellLength = 1/(float) rowLength;
	int k = 0;

	for(int i = 0; i < rowLength; i++) {
		for(int j = 0; j < rowLength; j++) {

			//For now not using any randomness
			float newX = (x - ((i-1) * cellLength));
			float newY = (y - ((j-1) * cellLength));

			dir[k] = normalize(vec3(newX, newY, focalLength));
			dir[k] = cameraRot * dir[k];
			k++;
		}
	}
}

void raycasting() {
	//Iterate through all pixels in window
	for(int y = 0; y < SCREEN_HEIGHT; y++) {
		for(int x = 0; x < SCREEN_WIDTH; x++) {
		
			//Calculate relative x and y positions of the pixel to the camera position
			float newX = (float) x - (float) SCREEN_WIDTH / 2;
			float newY = (float) y - (float) SCREEN_HEIGHT / 2;
			
			//array of direction vectors used for antialiasing
			vec3 d[antiAliasingCells];
			getArrayOfDirectionVectors(newX, newY, antiAliasingCells, d);

			vec3 R(0,0,0);

			float factor = 1.0f/(float) antiAliasingCells;
			
			//If the ray intersects a triangle then fill the pixel
			//with the color of the closest intersecting triangle
			for(int i = 0; i < antiAliasingCells; i++) {

				//holds information about the closest intersection for this ray
				Intersection closest = {vec3(0,0,0), std::numeric_limits<float>::max(), -1};

				if(ClosestIntersection(cameraPos, d[i], objects, closest) == true) {
					//row
					vec3 color = objects[closest.objectIndex].triangles[closest.triangleIndex].color;
					//D
					vec3 light = DirectLight(closest);
		
					//Assuming diffuse surface, the light that gets reflected is the color vector * the light vector plus
					//the indirect light vector where the * operator denotes element-wise multiplication between vectors.
					R += color * (light + indirectLight) * factor;
				}
			}
			PutPixelSDL(screen, x, y, R);

		}
	}
}
