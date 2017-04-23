#include <SFML/Graphics.hpp>

#include <stdlib.h>
#include <vector>
#include <time.h>
#include <iostream>

#define PI 3.14159265359

#define Y_CENTER 300

#define INIT_VERTEXES 20
#define INIT_HEIGHT 100
#define INIT_WIDTH 30

#define EXTRUDE_MAX 10
#define EXTRUDE_VERTEX_CHANCE 2 // 1/X

#define EXTRUDE_PROP_CHANCE 70 // PERCENTAGE
#define LARGE_EXTRUDE_CHANCE 10 // PERCENTAGE

#define MAX_ENGINE_SLOPE 35
#define ENGINE_SPACING 5

#define MERGE_MAX_OFFSET 50

float angleHor(sf::Vector2f a, sf::Vector2f b) {
	int deltaX = b.x - a.x;
	int deltaY = b.y - a.y;
	float angle = atan2(deltaY, deltaX) * 180 / PI;
	return angle;
}

float angleLines(sf::Vector2f a1, sf::Vector2f a2, sf::Vector2f b1, sf::Vector2f b2) {
	int x1 = a2.x - a1.x;
	int y1 = a2.y - a1.y;

	int x2 = b2.x - b1.x;
	int y2 = b2.y - b1.y;

	int q = x1*x2 + y1*y2;
	int mod1 = sqrt(x1*x1 + y1*y1);
	int mod2 = sqrt(x2*x2 + y2*y2);

	return acos(q / (mod1*mod2));
}

float dist(sf::Vector2f a, sf::Vector2f b) {
	return sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y));
}

sf::ConvexShape smallExtrudeGenerator(int &originX, int &originY) {
	sf::ConvexShape c = sf::ConvexShape(4);
	sf::Vector2f v;

	//Base Values
	int width = rand() % 25 + 20;
	int height = rand() % 4 + 3;

	//Deviations
	v.x = rand() % 2;
	v.y = 0;
	c.setPoint(0, v);
	v.x = width - rand() % 2;
	v.y = 0;
	c.setPoint(1, v);
	v.x = width;
	v.y = height;
	c.setPoint(2, v);
	v.x = 0;
	v.y = height;
	c.setPoint(3, v);

	originX = width / 2;
	originY = height;

	return c;
}

sf::ConvexShape largeExtrudeGenerator(int &originX, int &originY) {
	sf::ConvexShape c = sf::ConvexShape(4);
	sf::Vector2f v;

	//Base Values
	int width = rand() % 10 + 40;
	int height = rand() % 20 + 15;

	//Deviations
	v.x = 2 + rand() % 8;
	v.y = 0;
	c.setPoint(0, v);
	v.x = width - (rand() % 8 + 2);
	v.y = 0;
	c.setPoint(1, v);
	v.x = width;
	v.y = height;
	c.setPoint(2, v);
	v.x = 0;
	v.y = height;
	c.setPoint(3, v);

	originX = width/2;
	originY = height;

	return c;
}

std::vector<sf::ConvexShape> generateExtrudes(sf::VertexArray hull) {
	std::vector<sf::ConvexShape> extrudes;

	// --- EXTRUDES ---
	// Edges are picked by taking vertexes in pairs
	for (size_t i = 2; i < INIT_VERTEXES - 2 - 2; i += 1) {		
		// Extrude chance
		if (rand() % 100 < EXTRUDE_PROP_CHANCE) {

			// Middle point between edge
			int middleX = hull[i].position.x + (hull[i + 2].position.x - hull[i].position.x) / 2;
			int middleY = hull[i].position.y + (hull[i + 2].position.y - hull[i].position.y) / 2;

			// Edge angle
			int angle = angleHor(hull[i].position, hull[i + 2].position);

			sf::ConvexShape extrude;
			sf::Vector2f middle;

			int originX = 0;
			int originY = 0;

			//TODO add more extrudes here (ie. weapons, control tower)
			int extrudeType = rand() % 100 + 1;
			if (extrudeType < LARGE_EXTRUDE_CHANCE)
				extrude = largeExtrudeGenerator(originX, originY);
			else
				extrude = smallExtrudeGenerator(originX, originY);


			//Distinguish top and bottom
			if (i%2 == 0)
				extrude.setRotation(angle);
			else
				extrude.setRotation(angle + 180);
			
			middle.x = middleX;
			middle.y = middleY;
			extrude.setOrigin(originX, originY - 1);
			extrude.setPosition(middleX, middleY);
			extrude.setFillColor(sf::Color::White);
			extrudes.push_back(extrude);
		}
	}

	return extrudes;
}

//TODO add more engines here
sf::ConvexShape generateEngine(int &engineHeight) {
	sf::ConvexShape engine;

	engine = sf::ConvexShape(5);

	sf::Vector2f v;

	v.x = 0;
	v.y = 20;
	engine.setPoint(0, v);
	v.x = 5;
	v.y = 10;
	engine.setPoint(1, v);
	v.x = 0;
	v.y = 0;
	engine.setPoint(2, v);
	v.x = 40;
	v.y = 0;
	engine.setPoint(3, v);
	v.x = 40;
	v.y = 20;
	engine.setPoint(4, v);
	engine.setOrigin(25, 10);

	engineHeight = 20;

	return engine;
}

//TODO Engine placement is not optimal (probably can be improved)
std::vector<sf::ConvexShape> generateEngines(sf::VertexArray hull) {
	std::vector<sf::ConvexShape> engines;

	int middleX;
	int middleY;

	int deltaX;
	int deltaY;

	int engineHeight=0;
	int nEngines;

	int angle;

	sf::ConvexShape engine;

	//Uni Engines (when no merge is present)
	angle = abs(angleHor(hull[0].position, hull[0 + 1].position));
	if (angle > MAX_ENGINE_SLOPE)
	{
		middleX = hull[0].position.x + (hull[0 + 1].position.x - hull[0].position.x) / 2;
		middleY = hull[0].position.y + (hull[0 + 1].position.y - hull[0].position.y) / 2;

		deltaX = abs(hull[0].position.x - hull[0 + 1].position.x);
		deltaY = abs(hull[0].position.y - hull[0 + 1].position.y);

		engine = generateEngine(engineHeight);
		engine.setFillColor(sf::Color::White);

		nEngines = deltaY / (engineHeight + ENGINE_SPACING*2);

		for (size_t i = 1; i < nEngines + 1; i++)
		{
			engine.setPosition(hull[0].position.x + deltaX / nEngines * i, hull[0].position.y + deltaY / nEngines * i - ENGINE_SPACING *(i + 1));
			engines.push_back(engine);
		}
	}

	//Top Engines
	angle = abs(angleHor(hull[0].position, hull[0 + 2].position));
	if (angle > MAX_ENGINE_SLOPE)
	{
		middleX = hull[0].position.x + (hull[0 + 2].position.x - hull[0].position.x) / 2;
		middleY = hull[0].position.y + (hull[0 + 2].position.y - hull[0].position.y) / 2;

		deltaX = abs(hull[0].position.x - hull[0 + 2].position.x);
		deltaY = abs(hull[0].position.y - hull[0 + 2].position.y);

		engine = generateEngine(engineHeight);
		engine.setFillColor(sf::Color::White);

		nEngines = deltaY / (engineHeight + ENGINE_SPACING * 2);

		for (size_t i = 1; i < nEngines + 1; i++)
		{
			engine.setPosition(hull[0].position.x + deltaX / nEngines * i - ENGINE_SPACING *(i + 1), hull[0].position.y - deltaY / nEngines * i + ENGINE_SPACING *(i + 1));
			engines.push_back(engine);
		}
	}

	//Bottom Engines
	angle = abs(angleHor(hull[1].position, hull[1 + 2].position));
	if (angle > MAX_ENGINE_SLOPE)
	{
		middleX = hull[1].position.x + (hull[1 + 2].position.x - hull[1].position.x) / 2;
		middleY = hull[1].position.y + (hull[1 + 2].position.y - hull[1].position.y) / 2;

		deltaX = abs(hull[1].position.x - hull[1 + 2].position.x);
		deltaY = abs(hull[1].position.y - hull[1 + 2].position.y);

		engine = generateEngine(engineHeight);
		engine.setFillColor(sf::Color::White);

		nEngines = deltaY / (engineHeight + ENGINE_SPACING * 2);

		for (size_t i = 1; i < nEngines + 1; i++)
		{
			engine.setPosition(hull[1].position.x + deltaX / nEngines * i - ENGINE_SPACING *(i + 1), hull[1].position.y + deltaY / nEngines * i - ENGINE_SPACING *(i + 1));
			engines.push_back(engine);
		}
	}

	return engines;
}

sf::VertexArray generateHull() {

	// --- Initial Setup (2x4 rectangle) ---

	std::vector<sf::Vector2f> vertexes(INIT_VERTEXES);

	for (size_t i = 0; i < INIT_VERTEXES; i+=2)
	{
		vertexes[i] = sf::Vector2f(INIT_WIDTH + INIT_WIDTH * i, Y_CENTER - INIT_HEIGHT /2);
		vertexes[i+1] = sf::Vector2f(INIT_WIDTH + INIT_WIDTH * i, Y_CENTER + INIT_HEIGHT / 2);
	}

	// --- Decides if ship has merge points in front and back ---
	// 0 - Front (right)
	// 1 - Back (left)
	// 2 - Both
	int pointy = rand() % 3;

	//Defines back and front ignores for rest of generation (ignore back and front vertexes if they are merged)
	int backIgnore = 0;
	int frontIgnore = 0;

	int off = 0;

	switch (pointy)
	{
	case 0:
		frontIgnore = 2;
		off = Y_CENTER + rand() % (MERGE_MAX_OFFSET * 2) - MERGE_MAX_OFFSET;
		vertexes[INIT_VERTEXES - 1].y = off;
		vertexes[INIT_VERTEXES - 2].y = off;
		break;
	case 1:
		backIgnore = 2;
		off = Y_CENTER + rand() % (MERGE_MAX_OFFSET * 2) - MERGE_MAX_OFFSET;
		vertexes[0].y = off;
		vertexes[1].y = off;
		break;
	case 2:
		backIgnore = 2;
		frontIgnore = 2;
		off = Y_CENTER + rand() % (MERGE_MAX_OFFSET * 2) - MERGE_MAX_OFFSET;
		vertexes[INIT_VERTEXES - 1].y = off;
		vertexes[INIT_VERTEXES - 2].y = off;
		off = Y_CENTER + rand() % (MERGE_MAX_OFFSET * 2) - MERGE_MAX_OFFSET;
		vertexes[0].y = off;
		vertexes[1].y = off;
		break;
	default:
		break;
	}

	// --- Global Shape (extrude vertexes) ---
	for (size_t i = backIgnore; i < INIT_VERTEXES - frontIgnore; i+=2)
	{
		// Extrude Chance
		if (rand() % EXTRUDE_VERTEX_CHANCE == 0) {
			// Extrude Type
			// 0 - Up
			// 1 - Down
			// 2 - Both oppose
			// 3 - Both up
			// 4 - Both down
			int extrudeType = rand() % 3;
			switch (extrudeType)
			{
			case 0:
				off = rand() % EXTRUDE_MAX;
				vertexes[i].y -= off;
				break;
			case 1:
				off = rand() % EXTRUDE_MAX;
				vertexes[i + 1].y += off;
				break;
			case 2:
				off = rand() % (EXTRUDE_MAX * 2) - EXTRUDE_MAX;
				vertexes[i].y -= off;
				vertexes[i + 1].y += off;
				break;
			case 3:
				off = rand() % (EXTRUDE_MAX * 2) - EXTRUDE_MAX;
				vertexes[i].y -= off;
				vertexes[i + 1].y -= off;
				break;
			case 4:
				off = rand() % (EXTRUDE_MAX * 2) - EXTRUDE_MAX;
				vertexes[i].y += off;
				vertexes[i + 1].y += off;
				break;
			default:
				break;
			}
		}		
	}

	// --- Pass vertexes to actual VertexArray ---
	sf::VertexArray ship = sf::VertexArray(sf::TrianglesStrip, vertexes.size());

	for (size_t i = 0; i < vertexes.size(); i++)
	{
		ship[i].position = vertexes.at(i);
	}

	return ship;
}

void display(sf::VertexArray ship, sf::RenderWindow *window) {
	window->clear();
	window->draw(ship);
	window->display();
}

int main()
{
	srand(time(NULL));

	sf::RenderWindow window(sf::VideoMode(600, 600), "2d SpaceShip Generator");

	sf::VertexArray ship_hull;
	std::vector<sf::ConvexShape> ship_extrudes;
	std::vector<sf::ConvexShape> ship_engines;

	//testing
	sf::VertexArray ship2;
	std::vector<sf::Vector2f> vertexes;

	while (window.isOpen())
	{
		sf::Event event;
		while (window.waitEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
			else if (event.type == sf::Event::KeyPressed) {
				if (event.key.code == sf::Keyboard::R)
				{
					ship_hull = generateHull();
					ship_extrudes = generateExtrudes(ship_hull);
					ship_engines = generateEngines(ship_hull);

					window.clear();

					window.draw(ship_hull);
					for (size_t i = 0; i < ship_extrudes.size(); i++) {
						window.draw(ship_extrudes[i]);
					}
					for (size_t i = 0; i < ship_engines.size(); i++) {
						window.draw(ship_engines[i]);
					}

					window.display();
				}
			}
			// for testing
			else if (event.type == sf::Event::MouseButtonPressed) {
				if (event.mouseButton.button == sf::Mouse::Left) {
					sf::Vector2f a(event.mouseButton.x, event.mouseButton.y);
					vertexes.push_back(a);
					ship2 = sf::VertexArray(sf::TrianglesStrip, vertexes.size());
					for (size_t i = 0; i < vertexes.size(); i++)
					{
						ship2[i].position = vertexes[i];
					}
					display(ship2, &window);
				}
			}
		}
	}

	return 0;
}