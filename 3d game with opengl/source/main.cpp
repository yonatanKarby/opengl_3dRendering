#include <GL/glew.h>
#include "../inc/GLFW/glfw3.h"
#include <stdio.h>
#include <iostream>
#include <conio.h>
#include <chrono>
#include <thread>
#include <vector>
#include <algorithm>

#define SCREEN_WIDTH 2560
#define SCREEN_HEIGHT 1080

#define BOXES_SIZE 30
#define BOX_DISTANCE 60

#define PLAYER_SIZE 25
#define PLAYER_CAMERA_DISTANCE 30.0
////////////////////////////////////////
#define SLOW_DONW_CONSTANT 0.7
#define MOVMENT_SPEED_MULT 4
#define START_JUMP_SPEED_PLAYER 3
#define gravity -25
#define TIME_ADDER 0.01


#define PRESPECTIV_CONSTANT 100
float FFOVRAD = 0.3 / tanf((120 * 0.5) / (180 * 3.14159));
float SPECT = SCREEN_WIDTH / SCREEN_WIDTH;
float PRESPECTIVE_X_MULT = FFOVRAD * SPECT;

bool gameOver = false;
float playerMoveAmout = 4;

float angleX = 0;
float angleZ = 0;

using namespace std;

GLfloat lotsOfColors[] = {
255, 0, 0,
0, 255, 0,
0, 0, 255
};

GLfloat blue[] = {
0, 0, 255,
0, 0, 255,
0, 0, 255
};

GLfloat white[] = {
255, 255, 255,
255, 255, 255,
255, 255, 255
};

GLfloat none[] = {
255, 255, 255,
255, 255, 255,
255, 255, 255
};

class point
{
public:
	float x = 0;
	float y = 0;
	float z = 0;

	float locationx = 0;
	float locationy = 0;
	float locationz = 0;

	float vx = 0;
	float vy = 0;
	float vz = 0;

	float anglex = 0;
	float angley = 0;
	float anglez = 0;

	point() = default;

	point(float _x, float _y, float _z, float _locationx, float _locationy, float _locationz)
	{
		x = _x;
		y = _y;
		z = _z;

		locationx = _locationx;
		locationy = _locationy;
		locationz = _locationz;
	}
	

	/////////////////////////////////////////////////matrix function///////////////////////////////////////////////////
	std::vector<std::vector<float>>* makeRotateZ_Matrix(float angle)
	{
		std::vector<std::vector<float>>* temp = new std::vector<std::vector<float>>
		{
			{ cos(angle), -sin(angle), 0 },
			{ sin(angle), cos(angle), 0 },
			{ 0,0,1 }
		};
		return temp;
	}
	std::vector<std::vector<float>>* makeRotateX_Matrix(float angle)
	{
		std::vector<std::vector<float>>* temp = new std::vector<std::vector<float>>
		{
			{ 1, 0, 0 },
			{ 0, cos(angle), -sin(angle) },
			{ 0,  sin(angle), cos(angle) }
		};
		return temp;
	}
	std::vector<std::vector<float>>* makeRotateY_Matrix(float angle)
	{
		std::vector<std::vector<float>>* temp = new std::vector<std::vector<float>>
		{
			{ cos(angle), 0, sin(angle)},
			{ 0, 1, 0 },
			{ -sin(angle), 0, cos(angle), }
		};
		return temp;
	}


	point makePrespective_Matrix_updatePoint(point p, point center)
	{
		p.x = p.x * PRESPECTIV_CONSTANT / sqrt(p.z);
		p.y = p.y * PRESPECTIV_CONSTANT / sqrt(p.z);
		return p;
	}

	point matrixMult_point(std::vector<std::vector<float>> matrix, point p)
	{
		if (matrix[0].size() < 3)
			return point(0, 0, 0, 0, 0, 0);
		int x = p.matrixMult(0, matrix[0]);
		int y = p.matrixMult(1, matrix[1]);
		int z = p.matrixMult(2, matrix[2]);
		point multed = point(x, y, z, p.locationx, p.locationy, p.locationz);
		return multed;
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void moveBy(float delx, float dely, float delz)
	{
		x += delx;
		y += dely;
		z += delz;
	}

	int matrixMult(int index, std::vector<float> mult)
	{
		if (mult.size() < 3)
			return -1;
		switch (index)
		{
		case 0:
			return (int)(x * mult[0] + y * mult[1] + z * mult[2]);
		case 1:
			return (int)(x * mult[0] + y * mult[1] + z * mult[2]);
		case 2:
			return (int)(x * mult[0] + y * mult[1] + z * mult[2]);
		}
		return 0;
	}

	vector<point> render(vector<vector<float>>* rotateY_camera, vector<vector<float>>* rotateX_camera, point camera_point)
	{
		//setup the matrixs that will be needed
		vector<vector<float>>* rotateZ = makeRotateZ_Matrix(anglez);
		vector<vector<float>>* rotateY = makeRotateY_Matrix(angley);
		vector<vector<float>>* rotateX = makeRotateX_Matrix(anglex);

		point rotated = matrixMult_point(*rotateZ, (*this));
		rotated = matrixMult_point(*rotateX, rotated);
		rotated = matrixMult_point(*rotateY, rotated);
		rotated.moveBy(locationx, locationy, locationz);
		//camera rotation
		rotated.moveBy(camera_point.x, camera_point.y, camera_point.z);
		rotated = matrixMult_point(*rotateY_camera, rotated);
		rotated = matrixMult_point(*rotateX_camera, rotated);
		rotated.moveBy(0, 0, PLAYER_CAMERA_DISTANCE);
		//perspective
		rotated = makePrespective_Matrix_updatePoint(rotated, camera_point);

		delete rotateZ;
		delete rotateY;
		delete rotateX;
		return vector<point>{rotated};
	}
};

class Entity
{
public:
	virtual vector<float> getEntityLocation()
	{
		return vector<float>();
	}
	
	virtual void moveEntityTo(float x, float y, float z) = 0;
	virtual void moveEntityBy(float x, float y, float z) = 0;
	virtual void moveEntityAngle(float anglex, float angley, float anglez) = 0;
	virtual void changeSpeed(float vx, float vy, float vz) = 0;
	virtual void syncEntity() = 0;
	virtual void physicsTick() = 0;
	virtual void speedUpdate() = 0;
	virtual vector<point> render(vector<vector<float>>* rotateY_camera, vector<vector<float>>* rotateX_camera, point renderCenterfucos) = 0;
};

//////////////////////////////////////////draw functions////////////////////////////////////////////

void drawLine(point a, point b)
{
	GLfloat lineVertices[] =
	{
		a.x / SCREEN_WIDTH, a.y / SCREEN_HEIGHT,
		b.x / SCREEN_WIDTH, b.y / SCREEN_HEIGHT
	};
	glVertexPointer(2, GL_FLOAT, 0, lineVertices);
	glDrawArrays(GL_LINE_LOOP, 0, 2);

}
void drawTryAngle(point a, point b, point c, string color)
{
	if (color == "none")
	{
		glColorPointer(3, GL_FLOAT, 0, none);
		drawLine(a, b);
		drawLine(b, c);
		drawLine(a, c);
		return;
	}
	float lineVertices[] =
	{
		a.x / SCREEN_WIDTH, a.y / SCREEN_HEIGHT,
		b.x / SCREEN_WIDTH, b.y / SCREEN_HEIGHT,
		c.x / SCREEN_WIDTH, c.y / SCREEN_HEIGHT
	};
	glVertexPointer(2, GL_FLOAT, 0, lineVertices);
	if (color == "blue")
		glColorPointer(3, GL_FLOAT, 0, blue);
	if (color == "color")
		glColorPointer(3, GL_FLOAT, 0, lotsOfColors);
	if (color == "white")
		glColorPointer(3, GL_FLOAT, 0, white);
	glDrawArrays(GL_TRIANGLES, 0, 3);

}
//////////////////////////////////////draw functions///////////////////////////////////////////////

class triangle : public Entity
{
public:
	vector<point*> points;

	float _vx = 0, _vy = 0, _vz = 0;

	float _anglex = 0;
	float _angley = 0;
	float _anglez = 0;

	float _x = 0;
	float _y = 0;
	float _z = 0;

	string color;

	triangle(vector<point*> _points, string _color)
	{
		points = _points;
		color = _color;
	}
	~triangle()
	{
		for (auto p : points)
			delete p;
	}
	void moveEntityBy(float x, float y, float z)override
	{
		_x += x;
		_y += y;
		_z += z;
	}
	void moveEntityTo(float x, float y, float z)override
	{
		_x = x;
		_y = y;
		_z = z;
	}
	void moveEntityAngle(float anglex, float angley, float anglez)override
	{
		if (anglex == -1)
			anglex = _anglex;
		if (angley == -1)
			angley = _angley;
		if (anglex == -1)
			anglez = _anglez;
		_anglex = anglex;
		_angley = angley;
		_anglez = anglez;
	}
	void syncEntity()override
	{
		for (auto p : points)
		{
			p->locationx = _x;
			p->locationy = _y;
			p->locationz = _z;
			p->anglex = _anglex;
			p->angley = _angley;
			p->anglez = _anglez;
		}
	}
	void speedUpdate()override
	{
		_x += _vx;
		_y += _vy;
		_z += _vz;
	}
	void changeSpeed(float vx, float vy, float vz)override
	{
		if (vx != 0)
			_vx = vx;
		if (vy != 0)
			_vy = vy;
		if (vz != 0)
			_vz = vz;
	}
	void physicsTick()override
	{
		speedUpdate();
	}
	point getNormal(point* a, point* b, point* c)
	{
		point line1, line2, normal;
		line1.x = b->x - a->x;
		line1.y = b->y - a->y;
		line1.z = b->z - a->z;

		line2.x = c->x - a->x;
		line2.y = c->y - a->y;
		line2.z = c->z - a->z;

		normal.x = line1.y * line2.z - line1.z * line2.y;
		normal.y = line1.z * line2.x - line1.x * line2.z;
		normal.z = line1.x * line2.y - line1.y * line2.x;

		float l = sqrt((normal.x*normal.x) + (normal.y*normal.y) + (normal.z*normal.z));
		normal.x /= l; normal.y /= l; normal.z /= l;
		return normal;
	}

	vector<point> render(vector<vector<float>>* rotateY_camera, vector<vector<float>>* rotateX_camera, point renderCenterfucos)
	{
		vector<point> returnPoints;
		for (auto p : points)
		{
			point temp = p->render(rotateY_camera, rotateX_camera, renderCenterfucos)[0];
			if (temp.z <= 0)
				return vector<point>();
			returnPoints.push_back(temp);
		}
		return returnPoints;
	}

	vector<point> drawTriangle(vector<vector<float>>* rotateY_camera, vector<vector<float>>* rotateX_camera, point renderCenterfucos)
	{
		vector<point> renderedPoints = render(rotateY_camera, rotateX_camera, renderCenterfucos);
		if (renderedPoints.size() == 0)
			return vector<point>();
		point normal = getNormal(&renderedPoints[0], &renderedPoints[1], &renderedPoints[2]);
		if(normal.z < 0)
			drawTryAngle(renderedPoints[0], renderedPoints[1], renderedPoints[2], color);
		return renderedPoints;
	}

};

class cube : public Entity
{

public:
	vector<triangle*> polygons;
	float _x, _y, _z, _anglex=0, _angley=0, _anglez=0;
	float cube_width, cube_height, cube_length;
	string color;

private:
	//window settings
	float _vx = 0, _vy = 0, _vz = 0;
	int _width;
	int _height;

public:
	cube(float x, float y, float z, float width, float height, float length, string _color)
	{
		///////////////////make cube points///////////////////////////
		point* tempVar0 = new point(-width / 2, -height / 2, -length / 2, x, y, z);
		point* tempVar1 = new point(-width / 2, height / 2, -length / 2, x, y, z);
		point* tempVar2 = new point(width / 2, height / 2, -length / 2, x, y, z);
		point* tempVar3 = new point(width / 2, -height / 2, -length / 2, x, y, z);

		point* tempVar4 = new point(-width / 2, -height / 2, length / 2, x, y, z);
		point* tempVar5 = new point(-width / 2, height / 2, length / 2, x, y, z);
		point* tempVar6 = new point(width / 2, height / 2, length / 2, x, y, z);
		point* tempVar7 = new point(width / 2, -height / 2, length / 2, x, y, z);
		///set color///
		color = _color;
		///////////////////////////////make triangels////////////////////////////////////////
		triangle* t1 = new triangle(vector<point*>{ tempVar0, tempVar1, tempVar2 }, "none");
		triangle* t2 = new triangle(vector<point*>{ tempVar0, tempVar2, tempVar3 }, "none");
		
		triangle* t3 = new triangle(vector<point*>{ tempVar3, tempVar2, tempVar6 }, "none");
		triangle* t4 = new triangle(vector<point*>{ tempVar3, tempVar6, tempVar7 }, "none");

		triangle* t5 = new triangle(vector<point*>{ tempVar7, tempVar6, tempVar5 }, "none");
		triangle* t6 = new triangle(vector<point*>{ tempVar7, tempVar5, tempVar4 }, "none");
		
		triangle* t7 = new triangle(vector<point*>{ tempVar4, tempVar5, tempVar1 }, "none");
		triangle* t8 = new triangle(vector<point*>{ tempVar4, tempVar1, tempVar0 }, "none");

		triangle* t9 = new triangle(vector<point*>{ tempVar1, tempVar5, tempVar6 }, "none");
		triangle* t10 = new triangle(vector<point*>{ tempVar1, tempVar6, tempVar2 }, "none");
		
		triangle* t11 = new triangle(vector<point*>{ tempVar7, tempVar4, tempVar0 }, "none");
		triangle* t12 = new triangle(vector<point*>{ tempVar7, tempVar0, tempVar3 }, "none");
		
		////////////////create polygon/////////////////////
		polygons = {t1,t2,t3,t4,t5,t6,t7,t8,t9,t10, t11, t12};
		_x = x;
		_y = y;
		_z = z;

		cube_width = width;
		cube_height = height;
		cube_length = length;

		_width = width;
		_height = height;
	}
	~cube()
	{
		for (auto p : polygons)
			delete p;
	}
	bool checkCollision(cube* c)
	{
		if (_x + cube_width / 2 - c->_x - c->cube_width / 2 < 0 && _x - cube_width / 2 - c->_x - c->cube_width / 2 > 0)
			return true;
		if ((_y + cube_height / 2 - c->_y - c->cube_height / 2) < 0 && (_y - cube_height / 2 - c->_y - c->cube_height / 2) > 0)
			return true;
		if ((_z + cube_length / 2 - c->_z - c->cube_length / 2) < 0 && (_z - cube_length / 2 - c->_z - c->cube_length / 2) > 0)
			return true;
		return false;
	}
	void applyForce(float force, float xalpha, float zalpha)
	{

	}
	vector<float> getEntityLocation()override
	{
		return vector<float> {_x, _y, _z, _anglex, _angley, _anglez};
	}
	void moveEntityBy(float x, float y, float z)override
	{
		_x += x;
		_y += y;
		_z += z;
	}
	void moveEntityTo(float x, float y, float z) override
	{
		_x = x;
		_y = y;
		_z = z;
	}
	void moveEntityAngle(float anglex, float angley, float anglez)override
	{
		if (anglex == -1)
			anglex = _anglex;
		if (angley == -1)
			angley = _angley;
		if (anglex == -1)
			anglez = _anglez;
		_anglex = anglex;
		_angley = angley;
		_anglez = anglez;
	}
	void syncEntity()
	{
		for (auto p : polygons)
		{
			p->moveEntityTo(_x, _y, _z);
			p->moveEntityAngle(_anglex, _angley, _anglez);
			p->syncEntity();
		}
	}
	void speedUpdate()override
	{
		_x += _vx;
		_y += _vy;
		_z += _vz;
	}
	void changeSpeed(float vx, float vy, float vz)override
	{
		if (vx != 0)
			_vx = vx;
		if (vy != 0)
			_vy = vy;
		if (vz != 0)
			_vz = vz;
	}
	void physicsTick()override
	{
		for (auto p : polygons)
			p->physicsTick();
	}
	vector<point> render(vector<vector<float>>* rotateY_camera, vector<vector<float>>* rotateX_camera, point renderCenterfucos)
	{
		vector<triangle*> polys = polygons;
		for (auto p : polys)
		{
			p->drawTriangle(rotateY_camera, rotateX_camera, renderCenterfucos);
		}
		return vector<point>();
	}

};


class player : Entity
{
private:
	float _vx=0, _vy=0, _vz=0;
	bool isJumping = false;
	float tCouting = 0;
public:
	Entity* _player;
	float _x=0, _y=0, _z=0, _anglex=0, _angley=0, _anglez=0, _anglex_speed=0, _angley_speed=0, _anglez_speed=0;
	player(Entity* _player_point)
	{
		_player = _player_point;

		//////////////location and angle data///////////////////
		vector<float> playerData = _player->getEntityLocation();
		_x = playerData[0];
		_x = playerData[1];
		_x = playerData[2];

		_anglex = playerData[3];
		_angley = playerData[4];
		_anglez = playerData[5];
	}

	void jump()
	{
		tCouting = 0;
		isJumping = true;
		_anglex_speed = 0.1;
		_vy = START_JUMP_SPEED_PLAYER;
	}

	vector<float> getEntityLocation()override
	{
		return vector<float>{_x, _y, _z, _anglex, _angley, _anglez};
	}
	void moveEntityTo(float x, float y, float z)override
	{
		_x = x;
		_y = y;
		_z = z;
	}
	void moveEntityBy(float x, float y, float z)override
	{
		_x += x;
		_y += y;
		_z += z;
	}
	void moveEntityAngle(float anglex, float angley, float anglez)override
	{
		if(anglex != 0)
			_anglex = anglex;
		if (angley != 0)
			_angley = angley;
		if (anglez != 0)
			_anglez = anglez;
	}
	void angleSpeedUpdate()
	{
		_anglex += _anglex_speed;
		_angley += _angley_speed;
		_anglez += _anglez_speed;
	}
	void speedUpdate()
	{
		locationSpeedUpdate();
		angleSpeedUpdate();
	}
	void syncEntity()override
	{
		_player->moveEntityAngle(_anglex, _angley, _anglez);
		_player->moveEntityTo(_x, _y, _z);
		_player->syncEntity();
	}
	void locationSpeedUpdate()
	{
		_x += _vx;
		_y += _vy;
		_z += _vz;
	}
	float speedDecelarate(float speed, float a)
	{
		if (speed == 0)
			return 0;
		int direction = 0;
		if (speed < 0) { direction = 1; }
		else { direction = -1; }

		float slowDown = 1 - (1 / (1+abs(speed)));

		return speed + direction * slowDown * SLOW_DONW_CONSTANT + a;
	}
	void changeSpeedUpdate()
	{
		_vx = speedDecelarate(_vx, 0);
		_vz = speedDecelarate(_vz, 0);

		//std::cout << "vx: " << _vx << "vy: " << _vy << "vz: " << _vz << endl;
	}
	void changeSpeed(float vx, float vy, float vz)override
	{
		_vx = vx;
		_vy = vy;
		_vz = vz;
	}

	void jumpingCalculations()
	{
		tCouting += TIME_ADDER;
		_vy = START_JUMP_SPEED_PLAYER + gravity * tCouting;
		if (_y < 0)
		{
			tCouting = 0;
			isJumping = false;
			_y = 0;
			_vy = 0;
			_anglex_speed = 0;
			_anglex = 0;
			syncEntity();
		}
	}

	void physicsCalculations()
	{
		if (isJumping)
		{
			jumpingCalculations();
		}
		changeSpeedUpdate();
		speedUpdate();
	}

	void physicsTick()override
	{
		physicsCalculations();
	}
	vector<point> render(vector<vector<float>>* rotateY_camera, vector<vector<float>>* rotateX_camera, point renderCenterfucos)override
	{
		return _player->render(rotateY_camera, rotateX_camera, renderCenterfucos);
	}

};

class renderer
{
public:
	point p;
	player* _player;
	vector<Entity*> Entitys;
	point renderCenterfucos;

private:

	vector<vector<float>>* rotateZ_camera = p.makeRotateY_Matrix(0);
	vector<vector<float>>* rotateX_camera = p.makeRotateX_Matrix(0);

	int width;
	int height;

public:

	renderer(player* _player_pointer)
	{
		_player = _player_pointer;
		renderCenterfucos = point(0, 0, 0, 0, 0, 0);
	}
	~renderer()
	{
		for (auto c : Entitys)
			delete c;
		delete _player;
	}
	
	void change_cameraAngle_Z(float angle)
	{
		rotateZ_camera = p.makeRotateY_Matrix(angle);
	}
	void change_cameraAngle_X(float angle)
	{
		rotateX_camera = p.makeRotateX_Matrix(angle);
	}
	void physicsTick()
	{
		for (auto e : Entitys)
			e->physicsTick();
		_player->physicsTick();
		vector<float> p = _player->getEntityLocation();
		renderCenterfucos = point(-p[0], -p[1], -p[2], 0, 0, 0);
		syncAll();
	}

	void syncAll()
	{
		_player->syncEntity();
		for (auto e : Entitys)
		{
			e->syncEntity();
		}
	}

	void renderImage()
	{
		vector<Entity*> cubesDraw = Entitys;
		sort(cubesDraw.begin(), cubesDraw.end(), [](Entity* a, Entity* b)
		{
			return a->getEntityLocation()[2] < b->getEntityLocation()[2];
		});
		for (auto c : cubesDraw)
		{
			c->render(rotateZ_camera, rotateX_camera, renderCenterfucos);
		}
		_player->render(rotateZ_camera, rotateX_camera, renderCenterfucos);
	}
};

void draw(GLFWwindow* window, renderer* rendererObject)
{
	/* Render here */
	glClear(GL_COLOR_BUFFER_BIT);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	rendererObject->renderImage();
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	/* Swap front and back buffers */
	glfwSwapBuffers(window);
	/* Poll for and process events */
	glfwPollEvents();
}

Entity* c = new cube(0, 0, 0, PLAYER_SIZE, PLAYER_SIZE, PLAYER_SIZE, "none");
player* p = new player(c);
static renderer* rendererObject = new renderer(p);
static void cursoPositionCallback(GLFWwindow* window, double xpos, double ypos);
static void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

GLFWwindow* setup()
{
	GLFWwindow* window;
	/* Initialize the library */
	if (!glfwInit())
		std::cout << "error with init opengl";
	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "cube jump game", NULL, NULL);
	glfwSetCursorPosCallback(window, cursoPositionCallback);
	glfwSetKeyCallback(window, keyboardCallback);
	if (!window)
	{
		glfwTerminate();
		std::cout << "window did not open properly";
	}
	/* Make the window's context current */
	glfwMakeContextCurrent(window);
	//init glew
	if (glewInit() != GLEW_OK)
	{
		std::cout << "error with glew init";
		std::cout << "may need a restart";
	}
	else
		std::cout << "glew init ok";
	return window;
}

void physicsTicker()
{
	while (!gameOver)
	{
		rendererObject->physicsTick();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

int main(void)
{
	GLFWwindow* window = setup();
	Entity* player2 = new cube(0, 0, 50, PLAYER_SIZE, PLAYER_SIZE, PLAYER_SIZE, "none");
	Entity* player3 = new cube(50, 0, 50, PLAYER_SIZE, PLAYER_SIZE, PLAYER_SIZE, "none");
	
	rendererObject->Entitys.push_back(player2);
	rendererObject->Entitys.push_back(player3);
	while (!glfwWindowShouldClose(window))
	{
		rendererObject->physicsTick();
		draw(window, rendererObject);
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	glfwTerminate();
	delete rendererObject;
	return 0;
}

static void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	float vz;
	float vx;

	std::cout << "key: " << key << ", action: " << action << ", scancode: " << scancode << ", mods: " << mods << std::endl;

	if (scancode == 0)
		return;

	switch (key)
	{
	case 87:
		vz = MOVMENT_SPEED_MULT * cos(angleZ);
		vx = MOVMENT_SPEED_MULT * sin(angleZ);
		rendererObject->_player->changeSpeed(-vx, 0, vz);
		rendererObject->_player->syncEntity();
		break;
	case 83:
		 vz = MOVMENT_SPEED_MULT * cos(angleZ);
		 vx = MOVMENT_SPEED_MULT * sin(angleZ);
		 rendererObject->_player->changeSpeed(vx, 0, -vz);
		rendererObject->_player->syncEntity();
		break;
	case 65:
		vz = MOVMENT_SPEED_MULT * sin(angleZ);
		vx = MOVMENT_SPEED_MULT * cos(angleZ);
		rendererObject->_player->changeSpeed(-vx, 0, -vz);
		rendererObject->_player->syncEntity();
		break;
	case 68:
		vz = MOVMENT_SPEED_MULT * sin(angleZ);
		vx = MOVMENT_SPEED_MULT * cos(angleZ);
		rendererObject->_player->changeSpeed(vx, 0, vz);
		rendererObject->_player->syncEntity();
		break;
	case 32:
		rendererObject->_player->jump();
	}
}

static void cursoPositionCallback(GLFWwindow* window, double xpos, double ypos)
{
	angleZ = -(((4 * xpos / SCREEN_WIDTH)-2)*1.571);
	angleX = ((ypos / SCREEN_HEIGHT))*0.78541 - 0.78;

	rendererObject->change_cameraAngle_Z(angleZ);
	rendererObject->change_cameraAngle_X(angleX);

	rendererObject->_player->moveEntityAngle(0, -angleZ, 0);
	rendererObject->_player->syncEntity();
}