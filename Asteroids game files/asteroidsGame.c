#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

//PREDEFINED SIZES
//window size
#define WINDOWHEIGHT 21
#define WINDOWWIDTH 80
//ship size
#define SHIPHEIGHT 3
#define SHIPWIDTH 4
//asteroids size
#define S_AST_WIDTH 6
#define S_AST_HEIGHT 3
#define M_AST_WIDTH 9
#define M_AST_HEIGHT 4
#define L_AST_WIDTH 15
#define L_AST_HEIGHT 4

//CREATE A WINDOW IN THE TERMINAL
WINDOW *createWindow(int height,int width,int startY,int startX){
	WINDOW *newWin;
	newWin = newwin(height, width, startY, startX);
	wrefresh(newWin);
	return newWin;
}

typedef struct scoreBoard{
	int score;		//score of the game
	int shipLives;	//the number of lives the ship has
	int wave;		//the stage of the game a player is at
}scoreBoard;

typedef struct fireBall{
	double x;		//the x position
	double y;       //the y position
	int directionWhenShot;		//it depends on the direction of the ship
	bool active;	//fireball is in motion
}fireBall;
fireBall fireBalls[500]; //the max number of balls

typedef struct ship{
	double x;		//the x position
	double y;		//the y position
	int direction;	//the direction the ship is pointing (0 to 15)
	int lives;		//the number of lives the ship spawn with
	bool active;	//if hit by an asteroid, this becomes false
}ship;

//SHIP MODELS IN DIFF. DIRECTIONS - 16
const char *shipModel[][4] = {
	  {" _", "| ''--.._", "| __..--'", " ¯"},
	  {"________", "\\     .'", " \\_.'"},
	  {" _..--'¯.","'.    .'","  '..'"},
	  {"   .'|"," .'  |","'..__|"},
	  {"  /\\"," |  |","/____\\"},
	  {"|'.","|  '.","|__..'"},
	  {".¯''-.._"," '.    .'","   '..'"},
	  {"________","'.     /","   '._/"},
	  {"       _","_..--'' |"," ''--.. |","       ¯"},
	  {"    .'¯\\"," .'     \\","¯¯¯¯¯¯¯¯¯"},
	  {"   .''."," .'    '.","'_..-''¯"},
	  {"|¯¯''.","|  .'","|.'"},
	  {"\\¯¯¯¯/"," |  |","  \\/"},
	  {".''¯¯|"," '.  |","   '.|"},
	  {"  .''.",".'    '."," ¯''-.._'"},
	  {" /¯'.","/     '.","¯¯¯¯¯¯¯¯¯"}
    };

typedef struct asteroid{
	double x;		//the x position
	double y;		//the y position
	int size;     	// the size of the ast.  2-small 1-medium 0-big
	int direction;  //the asteroid can go in 8 directions (0 to 7)
	bool active;	//after it is destroyed, it becomes false
}asteroid;
asteroid asteroids[50];

//3 DIFF MODELS FOR THE ASTEROIDS - small - medium - large
const char *asteroidsModel[][6] = {
	{"     __.---.__","  _.'         '.",".'              '.",	//LARGE
	"|  '.    _..     |"," '.  '--'   .--''","   '.___.--'"},
	{"  .'¯¯¯.",".'    \\ '.","'.  _.'  |"," '.___.-'"}, //MEDIUM
	{" .'¯¯'.", "| -' .'", " ¯'-'"} //SMALL
};

//THIS FUNCTION RENDERS THE SHIP ON THE TERMINAL DEP. ON THE DIRECTION
void renderShip(ship *ship, WINDOW *window){
	double yShip = ship -> y;
	for (int i = 0; i < 4; i++){
		if (shipModel[ship->direction][i] == NULL)
			break;
		mvwprintw(window, yShip, ship->x, "%s", shipModel[ship->direction][i]);
		yShip++;
	}
}

//THIS FUNCTION RENDERS THE ASTEROIDS ON THE TERMINAL DEP. ON THE SIZE
void renderAsteroid(asteroid asteroid, WINDOW *gameWindow){
	double yAsteroid = asteroid.y;
	for (int i = 0; i < 6; i++){
		if (asteroidsModel[asteroid.size][i] == NULL)
			break;
		mvwprintw(gameWindow, yAsteroid, asteroid.x, "%s", asteroidsModel[asteroid.size][i]);
		yAsteroid++;
	}
}

//THIS FUNCTION CHANGES THE AST. COORDS DEP. ON THE SIZE (smaller ones move faster)
void moveAsteroids(asteroid asteroids[], int noAst, WINDOW *gameWindow){
	//change the coords of each asteroid
	for(int i = 0; i < 50; i++){
		if(asteroids[i].active == true){
			if(asteroids[i].direction == 0){
				asteroids[i].x = asteroids[i].x + 0.5 * (asteroids[i].size + 1);
			}
			if(asteroids[i].direction == 1){
				asteroids[i].x = asteroids[i].x + 0.5 * (asteroids[i].size + 1);
				asteroids[i].y = asteroids[i].y - 0.5 * (asteroids[i].size + 1);
			}
			if(asteroids[i].direction == 2){
				asteroids[i].y = asteroids[i].y - 0.5 * (asteroids[i].size + 1);
			}
			if(asteroids[i].direction == 3){
				asteroids[i].x = asteroids[i].x - 0.5 * (asteroids[i].size + 1);
				asteroids[i].y = asteroids[i].y - 0.5 * (asteroids[i].size + 1);
			}
			if(asteroids[i].direction == 4){
				asteroids[i].x = asteroids[i].x - 0.5 * (asteroids[i].size + 1);
			}
			if(asteroids[i].direction == 5){
				asteroids[i].x = asteroids[i].x - 0.5 * (asteroids[i].size + 1);
				asteroids[i].y = asteroids[i].y + 0.5 * (asteroids[i].size + 1);
			}
			if(asteroids[i].direction == 6){
				asteroids[i].y = asteroids[i].y + 0.5 * (asteroids[i].size + 1);
			}
			if(asteroids[i].direction == 7){
				asteroids[i].x = asteroids[i].x + 0.5 * (asteroids[i].size + 1);
				asteroids[i].y = asteroids[i].y + 0.5 * (asteroids[i].size + 1);
			}
			renderAsteroid(asteroids[i], gameWindow);
		}else{
			//if the asteroid is not active, set its coord outside the terminal
			//this avoids ghost collision
			asteroids[i].x = 200;
			asteroids[i].y = 200;
		}
	}
}

//CHECK IF THE BALL IS INSIDE THE TERMINAL WINDOW
void ballWindowCollision(fireBall ball[], int noBalls){
	for(int i=0; i < noBalls; i++){
		if(ball[i].y < 0 || ball[i].y > WINDOWHEIGHT || ball[i].x < 0 || ball[i].x > WINDOWWIDTH)
			ball[i].active = false;
	}
}

//THIS FUNCTION RENDERS THE BALL ON THE TERMINAL
void renderBalls(WINDOW *gameWindow, fireBall ball[], int noBalls){
	//first check if the balls are active.
	ballWindowCollision(ball, noBalls);
	for(int i=0; i < noBalls; i++){
		if(ball[i].active == true){
			if(ball[i].directionWhenShot == 0){
				ball[i].x = ball[i].x + 4;
			}
			if(ball[i].directionWhenShot == 1){
				ball[i].x = ball[i].x + 6;
				ball[i].y = ball[i].y - 1;
			}
			if(ball[i].directionWhenShot == 2){
				ball[i].x = ball[i].x + 3;
				ball[i].y = ball[i].y - 1;
			}
			if(ball[i].directionWhenShot == 3){
				ball[i].x = ball[i].x + 1;
				ball[i].y = ball[i].y - 2;
			}
			if(ball[i].directionWhenShot == 4){
				ball[i].y = ball[i].y - 2;
			}
			if(ball[i].directionWhenShot == 5){
				ball[i].x = ball[i].x - 1;
				ball[i].y = ball[i].y - 2;
			}
			if(ball[i].directionWhenShot == 6){
				ball[i].x = ball[i].x - 3;
				ball[i].y = ball[i].y - 1;
			}
			if(ball[i].directionWhenShot == 7){
				ball[i].x = ball[i].x - 6;
				ball[i].y = ball[i].y - 1;
			}
			if(ball[i].directionWhenShot == 8){
				ball[i].x = ball[i].x - 4;
			}
			if(ball[i].directionWhenShot == 9){
				ball[i].x = ball[i].x - 6;
				ball[i].y = ball[i].y + 1;
			}
			if(ball[i].directionWhenShot == 10){
				ball[i].x = ball[i].x - 3;
				ball[i].y = ball[i].y + 1;
			}
			if(ball[i].directionWhenShot == 11){
				ball[i].x = ball[i].x - 1;
				ball[i].y = ball[i].y + 2;
			}
			if(ball[i].directionWhenShot == 12){
				ball[i].y = ball[i].y + 2;
			}
			if(ball[i].directionWhenShot == 13){
				ball[i].x = ball[i].x + 1;
				ball[i].y = ball[i].y + 2;
			}
			if(ball[i].directionWhenShot == 14){
				ball[i].x = ball[i].x + 3;
				ball[i].y = ball[i].y + 1;
			}
			if(ball[i].directionWhenShot == 15){
				ball[i].x = ball[i].x + 6;
				ball[i].y = ball[i].y + 1;
			}
			mvwprintw(gameWindow, ball[i].y, ball[i].x, "o"); //render the ball
		}
	}
}

//THIS FUNCTION RENDERS EVERY CHANGE IN THE WINDOWS - it clears and updates them
void renderWindow(WINDOW *gameWindow, WINDOW *statsWindow, ship *ship, scoreBoard *scoreBoard){

	wrefresh(gameWindow);
	wrefresh(statsWindow);
	wclear(gameWindow);
	wclear(statsWindow);
	box(statsWindow, '|', '-');
	//no borders for the gameWindow
	wborder(gameWindow, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');

	//update the score board
	mvwprintw(statsWindow, 1, 1, "Lives:");
	scoreBoard->shipLives = ship->lives;
	for(int i = 0; i < scoreBoard->shipLives; i++){
		mvwprintw(statsWindow, 1, 7 + (i*3), " <3");
	}
	mvwprintw(statsWindow, 1, 24, "Score: %d", scoreBoard->score);
	mvwprintw(statsWindow, 1, 46, "Wave: %d", scoreBoard->wave);
	mvwprintw(statsWindow, 1, 61, "Press 'q' to exit");
}

//THIS FUNCTION CHECKS IF THE SHIP OR ASTEROIDS ARE INSIDE THE TERMINAL WINDOW
//IF NOT, WRAP AROUND
void shipAndAsteroidsWindowBounds(ship *ship, asteroid asteroids[], int noAst){
	//check window bound only if ship active (not dead)
	if(ship->active == true){
		if(ship -> y < -SHIPHEIGHT)
			ship -> y = WINDOWHEIGHT;
		if(ship -> y > WINDOWHEIGHT)
			ship -> y = -SHIPHEIGHT;
		if(ship -> x < 0)
			ship -> x = WINDOWWIDTH;
		if(ship -> x > WINDOWWIDTH)
			ship -> x = 0;
	}
	//asteroids
	for(int i = 0; i < 50; i++){
		if(asteroids[i].active == true){
			if(asteroids[i].y < -SHIPHEIGHT)
				asteroids[i].y = WINDOWHEIGHT;
			if(asteroids[i].y > WINDOWHEIGHT)
				asteroids[i].y = -SHIPHEIGHT;
			if(asteroids[i].x < 0)
				asteroids[i].x = WINDOWWIDTH;
			if(asteroids[i].x > WINDOWWIDTH)
				asteroids[i].x = 0;
		}
	}
}

//CHECK IF AN ASTEROID HAS HIT THE SHIP
int shipAsteroidCollision(ship *ship, asteroid asteroids[], int noAst){
    int shipLeft, asteroidLeft;
    int shipRight, asteroidRight;
    int shipTop, asteroidTop;
    int shipBottom, asteroidBottom;
	int collision = 0;

    shipLeft = ship -> x + 1;
    shipRight = ship -> x + 1 + SHIPWIDTH;
	shipTop = ship -> y + 1;
	shipBottom = ship -> y + 1 + SHIPHEIGHT;

	for(int i = 0; i < 50; i++){
		asteroidLeft = asteroids[i].x;
		asteroidTop = asteroids[i].y;
		if(asteroids[i].size == 2){
			asteroidRight = asteroids[i].x + S_AST_WIDTH;
			asteroidBottom = asteroids[i].y + S_AST_HEIGHT;
		}else if (asteroids[i].size == 1){
			asteroidRight = asteroids[i].x + M_AST_WIDTH;
			asteroidBottom = asteroids[i].y + M_AST_HEIGHT;
		}else if (asteroids[i].size == 0){
			asteroidRight = asteroids[i].x + L_AST_WIDTH;
			asteroidBottom = asteroids[i].y + L_AST_HEIGHT;
		}

		if (shipRight <= asteroidLeft || asteroidRight <= shipLeft ||
			shipBottom <= asteroidTop || asteroidBottom <= shipTop){
			//no collision
			collision = 0;
		}else{
			collision = 1;
			ship -> lives = ship ->lives - 1;
			return collision;
		}
	}
	return collision;
}

//ADD POINTS TO THE SCOREBOARD
void scoreBoardPoints(scoreBoard *scoreBoard, asteroid *asteroid){
	if (asteroid->size == 0){ //large
		scoreBoard->score = scoreBoard->score + 20;
	}
	if (asteroid->size == 1){ //medium
		scoreBoard->score = scoreBoard->score + 50;
	}
	if (asteroid->size == 2){ //small
		scoreBoard->score = scoreBoard->score + 100;
	}
}

//this function is used when an ast is distroyed and we have to spawn 2 smaller ones
//the x and y are the same, size is smaller, direction is random, active is true
int asteroidMitosis(WINDOW *window,asteroid *asteroid, int noAst, int indexInArray){
		if(asteroid->size == 2){ //small
			noAst--;
		}
		if(asteroid->size < 2){
			int asteroidSize =  asteroid -> size;
			int randDirection1 = rand() % 8;
			int randDirection2 = rand() % 8;

			//we dont want the asteroids to overlap and go in the same direction
			while (randDirection1 == randDirection2){
				randDirection1 = rand() % 8;
			}

			//replace the asteroid with a smaller version
			asteroids[indexInArray].active = true;
			asteroids[indexInArray].size = asteroidSize + 1;
			asteroids[indexInArray].direction = randDirection2;			
			asteroids[indexInArray].x = asteroid->x;
			asteroids[indexInArray].y = asteroid->y;

			//check the array for an available position
			for(int i = 0; i < 50; i++){
				if(asteroids[i].active == false){
					asteroids[i].active = true;
					asteroids[i].size = asteroidSize + 1;
					asteroids[i].direction = randDirection1;			
					asteroids[i].x = asteroid->x;
					asteroids[i].y = asteroid->y;
					break;
				}
			}
			noAst++;
		}
	return noAst;
}

//check the collison of the ast and the fireball
//distroy the asteroid and the fireball and set the score depending on the size.
int ballAsteroidCollision(WINDOW *window,scoreBoard *scoreBoard, asteroid asteroids[], int noAst, fireBall ball[], int noBalls){
	int astLeft, astRight, astTop, astBot;

	for(int i =0; i < 50; i++){
		astLeft = asteroids[i].x;
		astTop = asteroids[i].y;

		if(asteroids[i].size == 2){
			astRight = asteroids[i].x + S_AST_WIDTH;
			astBot = asteroids[i].y + S_AST_HEIGHT;
		}else if (asteroids[i].size == 1){
			astRight = asteroids[i].x + M_AST_WIDTH;
			astBot = asteroids[i].y + M_AST_HEIGHT;
		}else if (asteroids[i].size == 0){
			astRight = asteroids[i].x + L_AST_WIDTH;
			astBot = asteroids[i].y + L_AST_HEIGHT;
		}

		for(int j=0; j < noBalls; j++){
			if(ball[j].active == true){
				if((ball[j].x >= astLeft && ball[j].x <= astRight) && (ball[j].y >= astTop && ball[j].y <= astBot)){
					ball[j].active = false;
					asteroids[i].active = false;
					scoreBoardPoints(scoreBoard, &asteroids[i]);
					noAst = asteroidMitosis(window, &asteroids[i], noAst, i);
				}
			}
		}
	}
	return noAst;
}

//SHIP MOVEMENT
void shipMovement(ship *ship, float acc)
{
	if (ship->direction == 0)
	{
		ship->x = ship->x + acc * 1;
	}
	if (ship->direction == 1)
	{
		ship->x = ship->x + acc * 1;
		ship->y = ship->y - acc * 0.5;
	}
	if (ship->direction == 2)
	{
		ship->x = ship->x + acc * 1;
		ship->y = ship->y - acc * 1;
	}
	if (ship->direction == 3)
	{
		ship->x = ship->x + acc * 0.5;
		ship->y = ship->y - acc * 1;
	}
	if (ship->direction == 4)
	{
		ship->y = ship->y - acc * 1;
	}
	if (ship->direction == 5)
	{
		ship->x = ship->x - acc * 0.5;
		ship->y = ship->y - acc * 1;
	}
	if (ship->direction == 6)
	{
		ship->x = ship->x - acc * 1;
		ship->y = ship->y - acc * 1;
	}
	if (ship->direction == 7)
	{
		ship->x = ship->x - acc * 1;
		ship->y = ship->y - acc * 0.5;
	}
	if (ship->direction == 8)
	{
		ship->x = ship->x - acc * 1;
	}
	if (ship->direction == 9)
	{
		ship->x = ship->x - acc * 1;
		ship->y = ship->y + acc * 0.5;
	}
	if (ship->direction == 10)
	{
		ship->x = ship->x - acc * 1;
		ship->y = ship->y + acc * 1;
	}
	if (ship->direction == 11)
	{
		ship->x = ship->x - acc * 0.5;
		ship->y = ship->y + acc * 1;
	}
	if (ship->direction == 12)
	{
		ship->y = ship->y + acc * 1;
	}
	if (ship->direction == 13)
	{
		ship->x = ship->x + acc * 0.5;
		ship->y = ship->y + acc * 1;
	}
	if (ship->direction == 14)
	{
		ship->x = ship->x + acc * 1;
		ship->y = ship->y + acc * 1;
	}
	if (ship->direction == 15)
	{
		ship->x = ship->x + acc * 1;
		ship->y = ship->y + acc * 0.5;
	}
}

//SET THE SPAWN COORD OF THE BALL DEP. ON THE SHIP DIRECTION.
fireBall setBallSpawnCoords(ship *ship, fireBall ball){
	if (ball.directionWhenShot == 0){
		ball.x = ship->x + 11;
		ball.y = ship->y + 1;
	}
	if (ball.directionWhenShot == 1 || ball.directionWhenShot == 2){
		ball.x = ship->x + 12;
		ball.y = ship->y - 1;
	}
	if (ball.directionWhenShot == 3){
		ball.x = ship->x + 7;
		ball.y = ship->y - 1;
	}
	if (ball.directionWhenShot == 4){
		ball.x = ship->x + 4;
		ball.y = ship->y - 1;
	}
	if (ball.directionWhenShot == 5){
		ball.x = ship->x;
		ball.y = ship->y - 1;
	}
	if (ball.directionWhenShot == 6 || ball.directionWhenShot == 7){
		ball.x = ship->x - 2;
		ball.y = ship->y - 1;
	}
	if (ball.directionWhenShot == 8){
		ball.x = ship->x - 1;
		ball.y = ship->y + 1;
	}
	if (ball.directionWhenShot == 9){
		ball.x = ship->x - 1;
		ball.y = ship->y + 2;
	}
	if (ball.directionWhenShot == 10){
		ball.x = ship->x - 2;
		ball.y = ship->y + 4;
	}
	if (ball.directionWhenShot == 11){
		ball.x = ship->x;
		ball.y = ship->y + 4;
	}
	if (ball.directionWhenShot == 12){
		ball.x = ship->x + 3;
		ball.y = ship->y + 4;
	}
	if (ball.directionWhenShot == 13){
		ball.x = ship->x + 6;
		ball.y = ship->y + 4;
	}
	if (ball.directionWhenShot == 14){
		ball.x = ship->x + 12;
		ball.y = ship->y + 4;
	}
	if (ball.directionWhenShot == 15){
		ball.x = ship->x + 12;
		ball.y = ship->y + 3;
	}
	return ball;
}

void renderEverything(WINDOW *gameWindow, WINDOW *statsWindow, scoreBoard scoreBoard, ship *ship, asteroid asteroids[], fireBall fireBalls[], int noAst, int noBall){
	renderShip(ship, gameWindow);
	moveAsteroids(asteroids, noAst, gameWindow);
	renderBalls(gameWindow, fireBalls, noBall);
	renderWindow(gameWindow, statsWindow, ship, &scoreBoard);
}

//MAIN FUNCTION
int main()
{	
	srand(time(NULL));

	//start ncurses
	initscr();
	raw();
	keypad(stdscr, TRUE);
	noecho();
	curs_set(0);

	//create ship and place it in the middle of the screen
	ship *ship;
	ship->x = WINDOWWIDTH / 2 - SHIPWIDTH / 2;
	ship->y = WINDOWHEIGHT / 2 - SHIPHEIGHT / 2;
	ship->direction = 4;
	ship->lives = 3;
	ship->active = true;
	refresh();

	//SPAWN ONE ASTEROID AS A FIRST WAVE
	int noAst = 1;
	asteroids[0].active = true;
	asteroids[0].size = 0; //large
	asteroids[0].direction = 7;
	asteroids[0].x = 1;
	asteroids[0].y = 1;
	refresh();

	//FILE SCANNING FOR PREV. HIGHSCORE
	int highscore;
	FILE *file = NULL;
	file = fopen("./highscore_table.txt", "r");
	fseek(file, 0, SEEK_SET);
	if(file != NULL){
		fscanf(file, "%d", &highscore);
	}
	fclose(file);

	//create windows
	//having a boarder and a gamewindow helps with rendering glitches
	WINDOW *border;
	WINDOW *gameWindow;
	WINDOW *statsWindow;
	border = createWindow(WINDOWHEIGHT, WINDOWWIDTH, 3, 0);
	gameWindow = createWindow(WINDOWHEIGHT - 2, WINDOWWIDTH - 2, 4, 1);
	statsWindow = createWindow(3, WINDOWWIDTH, 0, 0);
	box(border, '|', '-');
	wrefresh(border);
	box(statsWindow, '|', '-');

	//AUTHOR, YEAR OF PRODUCTION
	mvwprintw(statsWindow, 1, 3, "Author: Stefan Popovici");
	mvwprintw(statsWindow, 1, 73, "2020");
	wrefresh(statsWindow);

	//GAME LOGO
	mvwprintw(gameWindow, 7, 16, "   /¯\\   ___|¯|_  ___  _ _  ___ (¯) __|¯| ___");
	mvwprintw(gameWindow, 8, 16, "  / ¯ \\ (_-<|  _|/ -_)| '_|/ _ \\|¯|/ _` |(_-<");
	mvwprintw(gameWindow, 9, 16, " /_/¯\\_\\/__/ \\__|\\___||_|  \\___/|_|\\__,_|/__/");
	mvwprintw(gameWindow, 10, 31, "Highscore: %d", highscore);
	mvwprintw(gameWindow, 11, 26, "Press arrow keys to start");

	//CONTROLS
	mvwprintw(gameWindow, 15, 1, "Space - fire");
	mvwprintw(gameWindow, 16, 1, "Arrow UP - thrust");
	mvwprintw(gameWindow, 17, 1, "Arrow LEFT & RIGHT - rotate");
	mvwprintw(gameWindow, 18, 1, "'X' - hyperspace");
	mvwprintw(gameWindow, 18, 60, "Press 'q' to exit");
	wrefresh(gameWindow);

	//CREATE A SCOREBOARD
	scoreBoard scoreBoard;
	scoreBoard.score = 0;
	scoreBoard.shipLives = ship->lives;
	scoreBoard.wave = 1;
	refresh();

	int key;
	int collision = 0;
	int noBall = 0;
	int frames = 15;
	float acc = 0;
	
	while(true){
		//check collison with border and other entities.
		shipAndAsteroidsWindowBounds(ship, asteroids, noAst);
		collision = shipAsteroidCollision(ship, asteroids, noAst);

		key = 0;	//reset key
		//get input from user
		key = getch();
		timeout(300);

		//FIRE BALL
		if(key == ' '){
			//set active the ball
			fireBalls[noBall].active = true;
			//set the direction of the ball
			fireBalls[noBall].directionWhenShot = ship->direction;
			//set the spawn coords
			fireBalls[noBall] = setBallSpawnCoords(ship, fireBalls[noBall]);
			//increas the no of balls
			noBall++;
		}
		noAst = ballAsteroidCollision(gameWindow, &scoreBoard, asteroids, noAst, fireBalls, noBall);

		//SHIP ROTATION
		if(key == KEY_RIGHT){
			ship->direction = ship -> direction - 1;
			if(ship->direction < 0)
				ship->direction = 15;
			renderShip(ship, gameWindow);
		}
		if(key == KEY_LEFT){
			ship->direction = ship -> direction + 1;
			if(ship->direction > 15)
				ship->direction = 0;
			renderShip(ship, gameWindow);
		}

		//THRUST - ACC
		if(key == KEY_UP){
			acc = acc + 0.4;
			if(acc >= 2)
				acc = 2;
			shipMovement(ship, acc);
			renderEverything(gameWindow, statsWindow, scoreBoard, ship, asteroids, fireBalls, noAst, noBall);
				// renderShip(ship, gameWindow);
				// renderWindow(gameWindow, statsWindow, ship, &scoreBoard);
		}

		//THRUST - DEACC
		if (key != KEY_UP){
			acc = acc - 0.4;
			if(acc <= 0)
				acc = 0;
			shipMovement(ship, acc);
			renderEverything(gameWindow, statsWindow, scoreBoard, ship, asteroids, fireBalls, noAst, noBall);
				// renderShip(ship, gameWindow);
				// renderWindow(gameWindow, statsWindow, ship, &scoreBoard);
		}

		//HYPERSPACE - teleport ship at a random pos on the terminal
		if(key == 'x' || key == 'X'){
			ship -> x = rand() % 72;
			ship -> y = rand() % 15;
		}

		//COLLISION - ship becomes inactive
		if(collision == 1 || ship->active == false){
			//start timer
			frames = frames - 1;
			ship->active = false;
			//teleport the ship outside the window.
			ship->x = 200;
			ship->y = 100;
			mvwprintw(gameWindow, 10, 32, "Spawning in %d..", frames);
			wrefresh(gameWindow);
			//wait 15 frames after death - grace period
			if (frames == 0){
				frames = 15;
				ship -> active = true;
				ship->x = WINDOWWIDTH / 2 - SHIPWIDTH / 2;
				ship->y = WINDOWHEIGHT / 2 - SHIPHEIGHT / 2;
				ship->direction = 4;
			}
		}

		//START NEW WAVE
		if (noAst == 0){
			scoreBoard.wave = scoreBoard.wave + 1;
			for(int i = 0; i < scoreBoard.wave; i++){
				int randomDirection = rand() % 8;
				asteroids[i].active = true;
				asteroids[i].size = 0;
				asteroids[i].x = rand() % 80;
				asteroids[i].y = 1;
			}
			noAst = scoreBoard.wave;
		}

		//render the window, ship, asteroids and fireballs every game loop
			// renderShip(ship, gameWindow);
			// moveAsteroids(asteroids, noAst, gameWindow);
			// renderBalls(gameWindow, fireBalls, noBall);
			// renderWindow(gameWindow, statsWindow, ship, &scoreBoard);
		renderEverything(gameWindow, statsWindow, scoreBoard, ship, asteroids, fireBalls, noAst, noBall);

		//GAME OVER and EXIT
		//display the score if out of lives or the player wants to exit the game
		if(ship -> lives == 0 || key == 'q' || key == 'Q'){
			//if the score is bigger than the current highscore open file and change it
			if(scoreBoard.score > highscore){
				file = fopen("./highscore_table.txt", "w");
				fprintf(file, "%d", scoreBoard.score);
				fclose(file);
			}
			wrefresh(statsWindow);
			wclear(statsWindow);
			wrefresh(statsWindow);			

			//DISPLAY GAME OVER AND SCORES
			wrefresh(gameWindow);
			wclear(gameWindow);
			mvwprintw(gameWindow, 8, 34, "GAME OVER.");
			mvwprintw(gameWindow, 9, 26, "Press any key to exit game");
			mvwprintw(gameWindow, 11, 23, "SCORE: %d", scoreBoard.score);
			mvwprintw(gameWindow, 11, 40, "HIGHSCORE: %d", highscore);
			wrefresh(gameWindow);
			//wait for the player to press a key for exit
			timeout(40000);
			key = getch();
			//press any key to exit.
			if (key >= 0 && key <= 400)
				break;
		}
	}
	//exit our of ncurses
	endwin();
	return 0;
}