/*
Hello, World! example
June 11, 2015
Copyright (C) 2015 David Martinez
All rights reserved.
This code is the most basic barebones code for writing a program for Arduboy.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.
*/

#include <Arduboy2.h>

// make an instance of arduboy used for many functions
Arduboy2 arduboy;

float prevShipX;
float prevShipY;
float prevShipAngle;

float shipX;
float shipY;
int score;
int asteroids;

const float acceleration = 0.4f;
const float bulletSpeed = 4.0f;
const float rotSpeed = 0.2f;
float shipVelX;
float shipVelY;
float shipAngle;

const int shipSize = 2;
const int asteroidSize = 3;
const int bulletSize = 1;

const int MAX_BULLETS=8;
int bulletCounter;
float bulletX[MAX_BULLETS];
float bulletY[MAX_BULLETS];
float bulletVelX[MAX_BULLETS];
float bulletVelY[MAX_BULLETS];

const int MAX_ASTEROIDS=3;
int asteroidCounter;
uint8_t asteroidActive[MAX_ASTEROIDS];
uint8_t asteroidBeenOnScreen[MAX_ASTEROIDS];
float asteroidX[MAX_ASTEROIDS];
float asteroidY[MAX_ASTEROIDS];
float asteroidVelX[MAX_ASTEROIDS];
float asteroidVelY[MAX_ASTEROIDS];

const int MAX_EXPLOSIONS=1;
uint8_t explosionIndex;
uint8_t explosionX[MAX_EXPLOSIONS];
uint8_t explosionY[MAX_EXPLOSIONS];
uint8_t explosionTimer[MAX_EXPLOSIONS];
const int explosionFrames = 12;

const uint8_t MAX_SHIP_PARTICLES=0;
const uint16_t shipParticleLifeFrames=20;
uint8_t shipParticleX[MAX_SHIP_PARTICLES];
uint8_t shipParticleY[MAX_SHIP_PARTICLES];
uint8_t shipParticleIndex;
uint16_t shipParticleFrame[MAX_SHIP_PARTICLES];


const int MAX_PARTICLES=32;
const float particleSpeed = 0.3;
const uint16_t particleLifeFrames=10;
float particleX[MAX_PARTICLES];
float particleY[MAX_PARTICLES];
float particleVelX[MAX_PARTICLES];
float particleVelY[MAX_PARTICLES];
uint16_t particleFrame[MAX_PARTICLES];
int particleIndex;

const int numExplosionParticles = 16;
const float explosionForce = 4.0f;
void explosion(int x, int y) {
  
  float angle = (2*3.1415901f)/numExplosionParticles;
  for (int i = 0; i < numExplosionParticles; i++) {
      float velX = explosionForce*cos(angle*i);
      float velY = explosionForce*sin(angle*i);
      spawnParticle(x,y,velX,velY);
  }
  
  explosionX[explosionIndex] = x;
  explosionY[explosionIndex] = y;
  explosionTimer[explosionIndex] = 0;
  explosionIndex = (explosionIndex+1) % MAX_EXPLOSIONS;
  
  //arduboy.fillCircle(x,y,12,BLACK);
}
void spawnShipParticle(int x, int y) {
  shipParticleX[shipParticleIndex] = x;
  shipParticleY[shipParticleIndex] = y;
  shipParticleFrame[shipParticleIndex] = 0;
  shipParticleIndex = (shipParticleIndex+1) % MAX_SHIP_PARTICLES;
}
void spawnParticle(int x, int y, float velX, float velY) {
  particleX[particleIndex] = x;
  particleY[particleIndex] = y;
  particleVelX[particleIndex] = velX;
  particleVelY[particleIndex] = velY;
  particleFrame[particleIndex] = 0;
  particleIndex = (particleIndex+1) % MAX_PARTICLES;
}
void spawnAsteroid(int i) {
    int dist = asteroidSize*3;
    int bitBigger = asteroidSize+2;
    int bitBigger2 = 2*bitBigger;
    int r = random(0,4);
    asteroidVelX[i] = 0.25f + 0.75f*(random(40)/40.0f);
    asteroidVelY[i] = 0.25f + 0.75f*(random(40)/40.0f);
    if (r == 0) {
      asteroidX[i] = -dist;
      asteroidY[i] = bitBigger + random (HEIGHT-bitBigger2);

      asteroidVelY[i] = asteroidY[i] > HEIGHT/2 ? -asteroidVelY[i] : asteroidVelY[i];
      //asteroidVelY[i] = 0;

    } else if (r == 1) {
      asteroidX[i] = WIDTH + dist;
      asteroidY[i] = bitBigger + random (HEIGHT - bitBigger2);
      asteroidVelX[i] = -asteroidVelX[i];
      asteroidVelY[i] = asteroidY[i] > HEIGHT/2 ? -asteroidVelY[i] : asteroidVelY[i];
      //asteroidVelY[i] = 0;

    } else if (r == 2) {
      asteroidX[i] = bitBigger + random (WIDTH - bitBigger2);
      asteroidY[i] = -dist;

      asteroidVelX[i] = asteroidX[i] > WIDTH/2 ? -asteroidVelX[i] : asteroidVelX[i];
      //asteroidVelX[i] = 0;

    } else if (r == 3) {
      asteroidX[i] = bitBigger + random (WIDTH - bitBigger2);
      asteroidY[i] = HEIGHT+dist;
      asteroidVelY[i] = -asteroidVelY[i];
      asteroidVelX[i] = asteroidX[i] > WIDTH/2 ? -asteroidVelX[i] : asteroidVelX[i];
      //asteroidVelX[i] = 0;
    }
    asteroidActive[i] = 1;
    asteroidBeenOnScreen[i] = 0;
}

const float wrapT = asteroidSize;
float wrapX(float x) {
  if (x > WIDTH + wrapT) {
    x = x - WIDTH;
  }
  else if (x < -wrapT) {
    x = x + WIDTH;
  }
  return x;
}
float wrapY(float y) {
  if (y > HEIGHT + wrapT) {
    y = y - HEIGHT;
  }
  else if (y < -wrapT) {
    y = y + HEIGHT;
  }
  return y;
}
int lastShotTick;
// This function runs once in your game.
// use it for anything that needs to be set only once in your game.
void setup() {


  // initiate arduboy instance
  arduboy.begin();

  // here we set the framerate to 15, we do not need to run at
  // default 60 and it saves us battery life
  arduboy.setFrameRate(15);
  score = 0;
  shipX = WIDTH*0.5f;
  shipY = HEIGHT*0.5f;
  shipVelX = 0;
  shipVelY = 0;
  shipAngle = 1.0f;

  for (int i = 0; i < MAX_BULLETS; i++) {
    bulletX[i] = -10;
    bulletY[i] = -10;
  }
  arduboy.initRandomSeed();
  arduboy.clear();
}

void drawLine(float x, float y, float size, float angle, uint8_t color) {
  float oneThird = 2.0f / 3.0f * 3.14159;
  int x0 = x + cos(angle)*size;
  int y0 = y + sin(angle)*size;
  int x1 = x + cos(angle - 1.25f*oneThird)*size;
  int y1 = y + sin(angle - 1.25f*oneThird)*size;

  int x2 = x + cos(angle + 1.25f*oneThird)*size;
  int y2 = y + sin(angle + 1.25f*oneThird)*size;
  arduboy.drawLine(x0,y0,x1,y1,color);
  arduboy.drawLine(x1,y1,x2,y2,color);
  arduboy.drawLine(x2,y2,x0,y0,color);
}

// our main game loop, this runs once every cycle/frame.
// this is where our game logic goes.
void loop() {

  // pause render until it's time for the next frame
  if (!(arduboy.nextFrame())) {
    return;
  }

  BeepPin2::timer();
  // first we clear our screen to black
  
  //arduboy.clear();
 
  prevShipAngle = shipAngle;
  prevShipX = shipX;
  prevShipY = shipY;
  // thrusting
  bool playedSound = false;
  if (arduboy.pressed(B_BUTTON)) {
    shipVelX += cos(shipAngle)*acceleration;
    shipVelY += sin(shipAngle)*acceleration;
    BeepPin2::tone(BeepPin2::freq(100.0f), 0);
  } else {
    BeepPin2::noTone();
  }
  // shooting
  if (arduboy.pressed(A_BUTTON)) {
    if (arduboy.frameCount - lastShotTick > 3) {
      bulletX[bulletCounter] = shipX;
      bulletY[bulletCounter] = shipY;
      bulletVelX[bulletCounter] = cos(shipAngle)*bulletSpeed;
      bulletVelY[bulletCounter] = sin(shipAngle)*bulletSpeed;

      if ((bulletVelY[bulletCounter] > 0 && shipVelY > 0 )||(bulletVelY[bulletCounter] < 0 && shipVelY < 0)) {
        bulletVelY[bulletCounter] += shipVelY;
      }
      if ((bulletVelX[bulletCounter] > 0 && shipVelX> 0 )||(bulletVelX[bulletCounter] < 0 && shipVelX < 0)) {
        bulletVelX[bulletCounter] += shipVelX;
      }
      bulletCounter = (bulletCounter+1)%MAX_BULLETS;
      lastShotTick = arduboy.frameCount;
      BeepPin2::tone(BeepPin2::freq(1000.0f),2);

    }
  }
  // steering
  if (arduboy.pressed(LEFT_BUTTON)) {
    shipAngle-=rotSpeed;
  }
  if (arduboy.pressed(RIGHT_BUTTON)) {
    shipAngle+=rotSpeed;
  }
  // reset
  if (arduboy.pressed(UP_BUTTON)) {

  }




  // draw the bullets
  Rect screen;
  screen.x = 0;
  screen.y = 0;
  screen.width = WIDTH;
  screen.height = HEIGHT;

  for (int i = 0; i < MAX_BULLETS; i++) {
    Point bulletPoint;
    bulletPoint.x = (int)bulletX[i];
    bulletPoint.y = (int)bulletY[i];
    
    if (arduboy.collide(bulletPoint, screen)) {
      //bulletVelX[i] -= bulletVelX[i]*0.05f;
      //bulletVelY[i] -= bulletVelY[i]*0.05f;
      arduboy.fillCircle((int)bulletX[i], (int)bulletY[i], bulletSize*2, BLACK);

      bulletX[i] += bulletVelX[i];
      bulletY[i] += bulletVelY[i];
      
      arduboy.drawCircle((int)bulletX[i], (int)bulletY[i], bulletSize, WHITE);

      for (int a = 0; a < MAX_ASTEROIDS; a++) {
        float deltaX = bulletX[i] - asteroidX[a];
        float deltaY = bulletY[i] - asteroidY[a];
        float mutalRadius = bulletSize + asteroidSize;
        if (pow(deltaX,2) + pow(deltaY,2) < pow(mutalRadius,2)) {
          score++;
          explosion(asteroidX[a], asteroidY[a]);
          asteroidActive[a] = 0;
          asteroids--;
          asteroidX[a] = -100.0f;
          asteroidY[a] = -100.0f;

        }

      }
    }
  }

  /*
  arduboy.print(WIDTH);
  arduboy.print(F(" x "));
  arduboy.print(HEIGHT);
  arduboy.print('\n');
  */
  // draw the asteroids
  for (int i = 0; i < MAX_SHIP_PARTICLES; i++) {
    if (shipParticleFrame[i] < shipParticleLifeFrames) {
      shipParticleFrame[i]++;
      arduboy.drawPixel((int)shipParticleX[i], (int)shipParticleY[i]);
    }
  }
  for (int i = 0; i < MAX_EXPLOSIONS; i++) {
    if (explosionTimer[i] < explosionFrames) {
      explosionTimer[i]++;

      float zeroToOne = ((float)explosionTimer[i])/explosionFrames;
      arduboy.fillCircle(explosionX[i], explosionY[i], zeroToOne*16, BLACK);
      //arduboy.drawCircle(explosionX[i], explosionY[i], zeroToOne*4);
    }
  }

  for (int i = 0; i < MAX_PARTICLES; i++) {
    if (particleFrame[i] < particleLifeFrames) {

      int x1 = (int)particleX[i];
      int y1 = (int)particleY[i];

      int xn1 = (int)(particleX[i]-particleVelX[i]);
      int yn1 = (int)(particleY[i]-particleVelY[i]);
      float d = 1.25f;
      int xn2 = (int)(particleX[i]-particleVelX[i]*d);
      int yn2 = (int)(particleY[i]-particleVelY[i]*d);
      arduboy.drawLine(xn1,yn1,xn2,yn2,BLACK);
      particleX[i] += particleVelX[i];
      particleY[i] += particleVelY[i];
      particleVelX[i] *= 0.8f;
      particleVelY[i] *= 0.8f;

      //arduboy.drawPixel((int)particleX[i], (int)particleY[i]);  
      //arduboy.drawLine((int)particleX[i], (int)particleY[i], x1,y1, BLACK);
      //arduboy.fillCircle((int)particleX[i],(int)particleY[i],1,BLACK);
      arduboy.drawLine((int)particleX[i], (int)particleY[i], x1,y1);
      particleFrame[i]++;
    }
  }

  
  for (int i = 0; i < MAX_ASTEROIDS; i++) {
      if(asteroidActive[i]) {
        Point asteroidPoint;
        arduboy.fillCircle((int)asteroidX[i], (int)asteroidY[i], asteroidSize+2, BLACK);
        asteroidX[i] += asteroidVelX[i];
        asteroidY[i] += asteroidVelY[i];
        asteroidPoint.x = (int)asteroidX[i];
        asteroidPoint.y = (int)asteroidY[i];
        Rect asteroidScreen;
        asteroidScreen.x = -asteroidSize;
        asteroidScreen.y = -asteroidSize;
        asteroidScreen.width = WIDTH + asteroidSize*2;
        asteroidScreen.height = HEIGHT + asteroidSize*2;
        arduboy.drawCircle((int)asteroidX[i], (int)asteroidY[i], asteroidSize, WHITE);
      
        // kill ship
        float deltaX = asteroidX[i] - shipX;
        float deltaY = asteroidY[i] - shipY;
        float mutalRadius = shipSize + asteroidSize;
        if (pow(deltaX,2) + pow(deltaY,2) < pow(mutalRadius,2)) {
          arduboy.clear();
          score = 0;
          asteroids = 0;
          shipX = WIDTH*0.5f;
          shipY = HEIGHT*0.5f;
          shipVelX = 0;
          shipVelY = 0;
          shipAngle = 0.0f;
        }
        
        Rect innerScreen;
        innerScreen.x = asteroidSize;
        innerScreen.y = asteroidSize;
        innerScreen.width = WIDTH - asteroidSize*2;
        innerScreen.height = HEIGHT - asteroidSize*2;
        if (asteroidBeenOnScreen[i] || arduboy.collide(asteroidPoint, innerScreen)) {
          asteroidBeenOnScreen[i] = 1;  
          asteroidX[i] = wrapX(asteroidX[i]);
          asteroidY[i] = wrapY(asteroidY[i]);
          // maybe draw second asteroid
          int t = asteroidSize;
          int secondToidX = asteroidPoint.x;
          int secondToidY = asteroidPoint.y;
          bool drawSecondToid = false;
          if (secondToidX < t) {
            secondToidX += WIDTH;
            drawSecondToid = true;
          } else if (secondToidX > WIDTH-t) {
            secondToidX -= WIDTH;
            drawSecondToid = true;
          }
          if (secondToidY < t) {
            secondToidY += HEIGHT;
            drawSecondToid = true;
          } else if (secondToidY > HEIGHT-t) {
            secondToidY -= HEIGHT;
            drawSecondToid = true;
          }
          if (drawSecondToid) {
            arduboy.fillCircle(secondToidX-asteroidVelX[i], secondToidY-asteroidVelY[i], asteroidSize+2, BLACK);
            arduboy.drawCircle(secondToidX, secondToidY, asteroidSize, WHITE);
          }
        }
      }
  }
  if (asteroids == 0) {
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
      spawnAsteroid(i);
    }
    asteroids = MAX_ASTEROIDS;
  }
  // draw the ship
  //arduboy.fillCircle((int)shipX, (int)shipY, shipSize*0.5f, WHITE);
  // rotation matrix:
  //  cos   -sin
  //  sin    cos
  //int x1 = x0 * c - y0 * s;
  //int y1 = y0 * s + y0 * c;


  float size = 3.0f;
  float oneThird = 2.0f / 3.0f * 3.14159;

  // undraw previous ship
  {
    
    /*
    int x0 = prevShipX + cos(prevShipAngle)*2*size;
    int y0 = prevShipY + sin(prevShipAngle)*2*size;
    
    int x1 = prevShipX + cos(prevShipAngle - oneThird)*size;
    int y1 = prevShipY + sin(prevShipAngle - oneThird)*size;

    int x2 = prevShipX + cos(prevShipAngle + oneThird)*size;
    int y2 = prevShipY + sin(prevShipAngle + oneThird)*size;
    */




    /*
    int x0 = shipX + cos(prevShipAngle)*size;
    int y0 = shipY + sin(prevShipAngle)*size;
    int x1 = shipX + cos(prevShipAngle - 1.25f*oneThird)*size;
    int y1 = shipY + sin(prevShipAngle - 1.25f*oneThird)*size;

    int x2 = shipX + cos(prevShipAngle + 1.25f*oneThird)*size;
    int y2 = shipY + sin(prevShipAngle + 1.25f*oneThird)*size;
    arduboy.drawLine(x0,y0,x1,y1,BLACK);
    arduboy.drawLine(x1,y1,x2,y2,BLACK);
    arduboy.drawLine(x2,y2,x0,y0,BLACK);
    arduboy.fillCircle(prevShipX,prevShipY, 2, BLACK);
    */


    drawLine(prevShipX, prevShipY, size, prevShipAngle, BLACK);

    //arduboy.drawCircle(x0,y0,3, WHITE);


    //arduboy.drawRect(shipX-halfRectSize,shipY-halfRectSize, halfRectSize*2,halfRectSize*2, BLACK);
  }
  
  // move ship
  shipX = shipX + shipVelX;
  shipY = shipY + shipVelY;
  shipX = wrapX(shipX);
  shipY = wrapY(shipY);

  arduboy.setTextColor(0x3c3c3c);
  arduboy.setTextSize(1);  
  arduboy.setCursor(0, 0);
  arduboy.print(score);

  // draw ship
  {
    int x0 = shipX + cos(shipAngle)*size;
    int y0 = shipY + sin(shipAngle)*size;
    int x1 = shipX + cos(shipAngle - 1.25f*oneThird)*size;
    int y1 = shipY + sin(shipAngle - 1.25f*oneThird)*size;

    int x2 = shipX + cos(shipAngle + 1.25f*oneThird)*size;
    int y2 = shipY + sin(shipAngle + 1.25f*oneThird)*size;
    arduboy.fillCircle(shipX + (shipVelX*1.2f),shipY + (shipVelY*1.2f),size, BLACK);

    arduboy.drawLine(x0,y0,x1,y1,WHITE);
    arduboy.drawLine(x1,y1,x2,y2,WHITE);
    arduboy.drawLine(x2,y2,x0,y0,WHITE);

    //arduboy.drawCircle(shipX,shipY, size, WHITE);
    float eps = 0.01f;
    if ((shipVelX > eps || shipVelX < -eps) || (shipVelY > eps || shipVelY < -eps)) {
      int p0 = shipX + cos(shipAngle+3.1415901f) *size;
      int p1 = shipY + sin(shipAngle+3.1415901f) *size;
      //spawnParticle(p0, p1, -shipVelX, -shipVelY);
      //int p0 = (x1 + x2 )/2;
      //int p1 = (y1 + y2 )/2;
      //arduboy.drawPixel(p0, p1, BLACK);

      spawnParticle(x1, y1, -shipVelX, -shipVelY);
      spawnParticle(x2, y2, -shipVelX, -shipVelY);
      //spawnShipParticle(x1, y1);
      //spawnShipParticle(x2, y2);
    }
  }
  
  shipVelX = shipVelX - shipVelX*0.05f;  
  shipVelY = shipVelY - shipVelY*0.05f;
  // then we finaly we tell the arduboy to display what we just wrote to the display
  arduboy.display();
}
