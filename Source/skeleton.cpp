#include <iostream>
#include <glm/glm.hpp>
#include <SDL.h>
#include "SDLauxiliary.h"
#include "TestModel.h"

using namespace std;
using glm::vec3;
using glm::mat3;

/* ----------------------------------------------------------------------------*/
/* GLOBAL VARIABLES                                                            */

const int SCREEN_WIDTH = 500;
const int SCREEN_HEIGHT = 500;
SDL_Surface* screen;
int t;
vector<vec3> stars(1000);

/* ----------------------------------------------------------------------------*/
/* FUNCTIONS                                                                   */

float Update();
void Draw();
void DrawI();
void DrawS(float dt);
void Interpolate(vec3 a, vec3 b, vector<vec3>& result);

int main( int argc, char* argv[] )
{
	/*vector<vec3> result(4);
	vec3 a(1,4,9.2);
	vec3 b(4,1,9.8);
	Interpolate(a,b,result);
	for(unsigned int i = 0; i < result.size(); ++i)
	{
		cout << "( " << result[i].x << ", " << result[i].y << ", " << result[i].z << " ) ";
	}*/


	for(unsigned int i = 0; i < stars.size(); i++) {
		float x = (float(rand()) / float(RAND_MAX) * 2) - 1;
		float y = (float(rand()) / float(RAND_MAX) * 2) - 1;
		float z = float(rand()) / float(RAND_MAX);

		vec3 p(x,y,z);
		stars[i] = p;
	}

	/*for(unsigned int i = 0; i < stars.size(); ++i)
	{
		cout << "( " << stars[i].x << ", " << stars[i].y << ", " << stars[i].z << " )\n";
	}*/


	screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT );
	t = SDL_GetTicks();	// Set start value for timer.
	float dt;

	while( NoQuitMessageSDL() )
	{
		dt = Update();
		DrawS(dt);
	}

	SDL_SaveBMP( screen, "screenshot.bmp" );
	return 0;
}

void Interpolate(vec3 a, vec3 b, vector<vec3>& result) {
	
	float xAdd, yAdd, zAdd;
	xAdd = (b.x - a.x) / (float) (result.size() - 1);
	yAdd = (b.y - a.y) / (float) (result.size() - 1);
	zAdd = (b.z - a.z) / (float) (result.size() - 1);

	result[0] = a;
	for(unsigned int i = 1; i < result.size(); i++) {
		result[i] = vec3(result[i-1].x + xAdd, result[i-1].y + yAdd, result[i-1].z + zAdd);
	}
	
}


float Update()
{
	// Compute frame time:
	int t2 = SDL_GetTicks();
	float dt = float(t2-t);
	t = t2;
	cout << "Render time: " << dt << " ms." << endl;
	return dt;
}

void DrawS(float dt)
{
	SDL_FillRect(screen, 0, 0);

	if( SDL_MUSTLOCK(screen) )
		SDL_LockSurface(screen);

	float vel = 0.0005;

	for(size_t s = 0; s < stars.size(); ++s) {
		float u = (SCREEN_HEIGHT/2) * (stars[s].x / stars[s].z) + (SCREEN_WIDTH/2);
		float v = (SCREEN_HEIGHT/2) * ((stars[s].y / stars[s].z) + 1);

		stars[s].z = stars[s].z - vel * dt;
		if(stars[s].z <= 0)
			stars[s].z += 1;
		if(stars[s].z > 1) 
			stars[s].z -= 1;

		vec3 color = 0.2f * vec3(1,1,1) / (stars[s].z * stars[s].z);
		PutPixelSDL(screen, u, v, color);

	}

	if( SDL_MUSTLOCK(screen) )
		SDL_UnlockSurface(screen);

	SDL_UpdateRect( screen, 0, 0, 0, 0 );
}

void DrawI()
{
	if( SDL_MUSTLOCK(screen) )
		SDL_LockSurface(screen);

	vec3 topLeft(1,0,0);		// red
	vec3 topRight(0,0,1);		// blue
	vec3 bottomRight(0,1,0);	// green
	vec3 bottomLeft(1,1,0);		// yellow

	vector<vec3> leftSide(SCREEN_HEIGHT);
	vector<vec3> rightSide(SCREEN_HEIGHT);
	Interpolate(topLeft, bottomLeft, leftSide);
	Interpolate(topRight, bottomRight, rightSide);


	for( int y=0; y<SCREEN_HEIGHT; ++y )
	{
		vector<vec3> row(SCREEN_WIDTH);
		Interpolate(leftSide[y], rightSide[y], row);

		for( int x=0; x<SCREEN_WIDTH; ++x )
		{
			vec3 color(row[x].x, row[x].y, row[x].z);
			PutPixelSDL( screen, x, y, color );
		}
	}

	if( SDL_MUSTLOCK(screen) )
		SDL_UnlockSurface(screen);

	SDL_UpdateRect( screen, 0, 0, 0, 0 );
}
