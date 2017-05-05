#ifndef TEST_MODEL_CORNEL_BOX_H
#define TEST_MODEL_CORNEL_BOX_H

// Defines a simple test model: The Cornel Box

#include <glm/glm.hpp>
#include <vector>


// Defines colors:
glm::vec3 red(    0.75f, 0.15f, 0.15f );
glm::vec3 yellow( 0.75f, 0.75f, 0.15f );
glm::vec3 green(  0.15f, 0.75f, 0.15f );
glm::vec3 cyan(   0.15f, 0.75f, 0.75f );
glm::vec3 blue(   0.15f, 0.15f, 0.75f );
glm::vec3 purple( 0.75f, 0.15f, 0.75f );
glm::vec3 white(  0.75f, 0.75f, 0.75f );

float L = 555;			// Length of Cornell Box side.


// Used to describe a triangular surface:
class Triangle
{
public:
	glm::vec3 v0;
	glm::vec3 v1;
	glm::vec3 v2;
	glm::vec3 normal;
	glm::vec3 color;

	Triangle( glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 color )
		: v0(v0), v1(v1), v2(v2), color(color)
	{
		ComputeNormal();
	}

	void ComputeNormal()
	{
		glm::vec3 e1 = v1-v0;
		glm::vec3 e2 = v2-v0;
		normal = glm::normalize( glm::cross( e2, e1 ) );
	}
};

class Object
{
public:
	glm::vec3 Pmin;
	glm::vec3 Pmax;
	std::vector<Triangle> triangles;

	Object( std::vector<Triangle>&triangles)
		: triangles(triangles)
	{
		ComputeBoundingBox();
	}

	void ComputeBoundingBox()
	{
		Pmin = triangles[0].v0;
		Pmax = triangles[0].v1;
		for(unsigned int i = 0; i < triangles.size(); i++) {
			
			//Check min for v0
			if(triangles[i].v0.x < Pmin.x) {
				Pmin.x = triangles[i].v0.x;
			}
			if(triangles[i].v0.y < Pmin.y) {
				Pmin.y = triangles[i].v0.y;
			}
			if(triangles[i].v0.z < Pmin.z) {
				Pmin.z = triangles[i].v0.z;
			}
			//Check max for v0
			if(triangles[i].v0.x > Pmax.x) {
				Pmax.x = triangles[i].v0.x;
			}
			if(triangles[i].v0.y > Pmax.y) {
				Pmax.y = triangles[i].v0.y;
			}
			if(triangles[i].v0.z > Pmax.z) {
				Pmax.z = triangles[i].v0.z;
			}

			//Check min for v1
			if(triangles[i].v1.x < Pmin.x) {
				Pmin.x = triangles[i].v1.x;
			}
			if(triangles[i].v1.y < Pmin.y) {
				Pmin.y = triangles[i].v1.y;
			}
			if(triangles[i].v1.z < Pmin.z) {
				Pmin.z = triangles[i].v1.z;
			}
			//Check max for v1
			if(triangles[i].v1.x > Pmax.x) {
				Pmax.x = triangles[i].v1.x;
			}
			if(triangles[i].v1.y > Pmax.y) {
				Pmax.y = triangles[i].v1.y;
			}
			if(triangles[i].v1.z > Pmax.z) {
				Pmax.z = triangles[i].v1.z;
			}

			//Check min for v2
			if(triangles[i].v2.x < Pmin.x) {
				Pmin.x = triangles[i].v2.x;
			}
			if(triangles[i].v2.y < Pmin.y) {
				Pmin.y = triangles[i].v2.y;
			}
			if(triangles[i].v2.z < Pmin.z) {
				Pmin.z = triangles[i].v2.z;
			}
			//Check max for v2
			if(triangles[i].v2.x > Pmax.x) {
				Pmax.x = triangles[i].v2.x;
			}
			if(triangles[i].v2.y > Pmax.y) {
				Pmax.y = triangles[i].v2.y;
			}
			if(triangles[i].v2.z > Pmax.z) {
				Pmax.z = triangles[i].v2.z;
			}				
		}
	}
};

void ReScaleTriangles(std::vector<Triangle>& triangles)
{
	// Scale to the volume [-1,1]^3

	for( size_t i=0; i<triangles.size(); ++i )
	{
		triangles[i].v0 *= 2/L;
		triangles[i].v1 *= 2/L;
		triangles[i].v2 *= 2/L;

		triangles[i].v0 -= glm::vec3(1,1,1);
		triangles[i].v1 -= glm::vec3(1,1,1);
		triangles[i].v2 -= glm::vec3(1,1,1);

		triangles[i].v0.x *= -1;

		triangles[i].v1.x *= -1;
		triangles[i].v2.x *= -1;

		triangles[i].v0.y *= -1;
		triangles[i].v1.y *= -1;
		triangles[i].v2.y *= -1;

		triangles[i].ComputeNormal();
	}
}

void RoomTriangles(std::vector<Triangle>& triangles)
{
	using glm::vec3;

	triangles.clear();
	triangles.reserve( 5*2*3 );

	// ---------------------------------------------------------------------------
	// Room

	vec3 A(L,0,0);
	vec3 B(0,0,0);
	vec3 C(L,0,L);
	vec3 D(0,0,L);

	vec3 E(L,L,0);
	vec3 F(0,L,0);
	vec3 G(L,L,L);
	vec3 H(0,L,L);

	// Floor:
	triangles.push_back( Triangle( C, B, A, green ) );
	triangles.push_back( Triangle( C, D, B, green ) );

	// Left wall
	triangles.push_back( Triangle( A, E, C, purple ) );
	triangles.push_back( Triangle( C, E, G, purple ) );

	// Right wall
	triangles.push_back( Triangle( F, B, D, yellow ) );
	triangles.push_back( Triangle( H, F, D, yellow ) );

	// Ceiling
	triangles.push_back( Triangle( E, F, G, cyan ) );
	triangles.push_back( Triangle( F, H, G, cyan ) );

	// Back wall
	triangles.push_back( Triangle( G, D, C, white ) );
	triangles.push_back( Triangle( G, H, D, white ) );

	ReScaleTriangles(triangles);
}

void ShortBlock(std::vector<Triangle>& triangles)
{
	// Short block

	glm::vec3 A(290,0,114);
	glm::vec3 B(130,0, 65);
	glm::vec3 C(240,0,272);
	glm::vec3 D( 82,0,225);

	glm::vec3 E(290,165,114);
	glm::vec3 F(130,165, 65);
	glm::vec3 G(240,165,272);
	glm::vec3 H( 82,165,225);

	// Front
	triangles.push_back( Triangle(E,B,A,red) );
	triangles.push_back( Triangle(E,F,B,red) );

	// Front
	triangles.push_back( Triangle(F,D,B,red) );

	triangles.push_back( Triangle(F,H,D,red) );

	// BACK
	triangles.push_back( Triangle(H,C,D,red) );
	triangles.push_back( Triangle(H,G,C,red) );

	// LEFT
	triangles.push_back( Triangle(G,E,C,red) );
	triangles.push_back( Triangle(E,A,C,red) );

	// TOP
	triangles.push_back( Triangle(G,F,E,red) );
	triangles.push_back( Triangle(G,H,F,red) );

	ReScaleTriangles(triangles);
}

void TallBlock(std::vector<Triangle>& triangles)
{
	// Tall block

	glm::vec3 A(423,0,247);
	glm::vec3 B(265,0,296);
	glm::vec3 C(472,0,406);
	glm::vec3 D(314,0,456);

	glm::vec3 E(423,330,247);
	glm::vec3 F(265,330,296);
	glm::vec3 G(472,330,406);
	glm::vec3 H(314,330,456);

	// Front
	triangles.push_back( Triangle(E,B,A,blue) );
	triangles.push_back( Triangle(E,F,B,blue) );


	// Front
	triangles.push_back( Triangle(F,D,B,blue) );
	triangles.push_back( Triangle(F,H,D,blue) );

	// BACK
	triangles.push_back( Triangle(H,C,D,blue) );
	triangles.push_back( Triangle(H,G,C,blue) );

	// LEFT
	triangles.push_back( Triangle(G,E,C,blue) );
	triangles.push_back( Triangle(E,A,C,blue) );

	// TOP
	triangles.push_back( Triangle(G,F,E,blue) );
	triangles.push_back( Triangle(G,H,F,blue) );

	ReScaleTriangles(triangles);
}


// Loads the Cornell Box. It is scaled to fill the volume:
// -1 <= x <= +1
// -1 <= y <= +1
// -1 <= z <= +1
void LoadTestModel( std::vector<Object>& objects )
{	
	//Load the room triangles
	std::vector<Triangle> triangles1;
	RoomTriangles(triangles1);	
	objects.push_back( Object(triangles1));

	//Load the short block
	std::vector<Triangle> triangles2;
	ShortBlock(triangles2);
	objects.push_back( Object(triangles2));

	//Load the tall block
	std::vector<Triangle> triangles3;
	TallBlock(triangles3);
	objects.push_back( Object(triangles3));
}


#endif
