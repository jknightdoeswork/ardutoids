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
#include <Tinyfont.h>
#define DO_SCREEN_CLEAR true
// make an instance of arduboy used for many functions
Arduboy2 arduboy;

float prevShipX;
float prevShipY;
float prevShipAngle;

float shipX;
float shipY;
int score;
uint8_t asteroids;

const float acceleration = 0.4f;
const float bulletSpeed = 4.0f;
const float rotSpeed = 0.2f;
float shipVelX;
float shipVelY;
float shipAngle;

const uint8_t shipSize = 2;
const uint8_t asteroidSize = 2;
const uint8_t bulletSize = 1;

const uint8_t MAX_BULLETS=6;
uint8_t bulletCounter;
float bulletX[MAX_BULLETS];
float bulletY[MAX_BULLETS];
float bulletVelX[MAX_BULLETS];
float bulletVelY[MAX_BULLETS];

const int MAX_ASTEROIDS=16;
uint8_t asteroidCounter;
uint8_t asteroidActive[MAX_ASTEROIDS];
uint8_t asteroidBeenOnScreen[MAX_ASTEROIDS];
uint8_t asteroidSizes[MAX_ASTEROIDS];
float asteroidX[MAX_ASTEROIDS];
float asteroidY[MAX_ASTEROIDS];
float asteroidVelX[MAX_ASTEROIDS];
float asteroidVelY[MAX_ASTEROIDS];

const int MAX_EXPLOSIONS=0;
uint8_t explosionIndex;
uint8_t explosionX[MAX_EXPLOSIONS];
uint8_t explosionY[MAX_EXPLOSIONS];
uint8_t explosionTimer[MAX_EXPLOSIONS];
const int explosionFrames = 12;

const uint8_t MAX_SHIP_PARTICLES=12;
uint8_t shipParticleX[MAX_SHIP_PARTICLES];
uint8_t shipParticleY[MAX_SHIP_PARTICLES];
//int shipFrame[MAX_SHIP_PARTICLES];
//const int shipLifeFrames = 12;
uint8_t shipParticleIndex;


const uint8_t MAX_PARTICLES=10;
const float particleSpeed = 0.3;
const uint16_t particleLifeFrames=12;
float particleX[MAX_PARTICLES];
float particleY[MAX_PARTICLES];
float particleVelX[MAX_PARTICLES];
float particleVelY[MAX_PARTICLES];
uint16_t particleFrame[MAX_PARTICLES];
int particleIndex;

const int numExplosionParticles = 5;
const float explosionForce = 4.0f;



const int scanClearRateY = 768;
int scanClearY;
uint8_t scanningHorizontal;

void scanClear() {
  for (int i = 0; i < scanClearRateY; i++) {
    if (i % 2 == scanningHorizontal % 2) {
      int i2 = (i + scanClearY)%(WIDTH*HEIGHT);

      if (scanningHorizontal % 2 == 0) {
        int y = i2/WIDTH;
        int x = i2 - y*WIDTH;
        if (scanningHorizontal < 2)
          arduboy.drawPixel(x,y,BLACK);
        else
          arduboy.drawPixel(x+1,y,BLACK);
      }
      else{
        int x = i2/HEIGHT;
        int y = i2 - x*HEIGHT;
        if (scanningHorizontal < 2)
          arduboy.drawPixel(x,y,BLACK);
        else
          arduboy.drawPixel(x,y+1,BLACK);
      }
    }
  }
  scanClearY = (scanClearY + scanClearRateY);
  if (scanClearY > WIDTH*HEIGHT) {
    scanClearY = 0;
    scanningHorizontal = (scanningHorizontal + 1) % 4;
  }
}
void explosion(int x, int y) {
    //arduboy.fillCircle(x,y,asteroidSize*,BLACK);

  float angle = (2*3.1415901f)/numExplosionParticles;
  for (int i = 0; i < numExplosionParticles; i++) {
      float velX = explosionForce*cos(angle*i);
      float velY = explosionForce*sin(angle*i);
      spawnParticle(x,y,velX,velY);
  }
  if (MAX_EXPLOSIONS > 0) {
    explosionX[explosionIndex] = x;
    explosionY[explosionIndex] = y;
    explosionTimer[explosionIndex] = 0;
    explosionIndex = (explosionIndex+1) % MAX_EXPLOSIONS;
  }
  
}
void spawnShipParticle(int x, int y) {
  shipParticleX[shipParticleIndex] = x;
  shipParticleY[shipParticleIndex] = y;
  //shipFrame[shipParticleIndex] = 0;
  shipParticleIndex = (shipParticleIndex+1) % MAX_SHIP_PARTICLES;
  arduboy.drawPixel(x,y);
}
void spawnParticle(int x, int y, float velX, float velY) {
  particleX[particleIndex] = x;
  particleY[particleIndex] = y;
  particleVelX[particleIndex] = velX;
  particleVelY[particleIndex] = velY;
  particleFrame[particleIndex] = 0;
  particleIndex = (particleIndex+1) % MAX_PARTICLES;
  arduboy.drawPixel(x,y);
}
int spawnAsteroid(int s, uint8_t doOverwrite, uint8_t& overwrote) {
    
    int bitBigger = HEIGHT/4;
    int bitBigger2 = HEIGHT/2;
    if (asteroidActive[asteroidCounter] == 1) {
      for (int a = 0; a < MAX_ASTEROIDS; a++) {
        asteroidCounter = (asteroidCounter+1) % MAX_ASTEROIDS;
        if (asteroidActive[asteroidCounter] == 0) {
          break;
        }
      }
    }

    int i = asteroidCounter;
    overwrote = 1;
    if (doOverwrite == 0 && asteroidActive[i] == 1) {
      overwrote = 0;
      return i;
    }
    else if (doOverwrite == 1 && asteroidActive[i] == 0) {
      overwrote = 0;
    }
    else if (doOverwrite == 1 && asteroidActive[i] == 1) {
      overwrote = 1;
    }

    asteroidVelX[i] = 0.25f + 0.75f*(random(40)/40.0f);
    asteroidVelY[i] = 0.25f + 0.75f*(random(40)/40.0f);
    asteroidSizes[i] = s;
    //int randomInnerScreenY = HEIGHT/4 + random(HEIGHT/2);
    //int randomInnerScreenX = WIDTH/4 + random(WIDTH/2);

    int randomInnerScreenY = HEIGHT/2;
    int randomInnerScreenX = WIDTH/2;
    float randomAngle = (random(60)/60.0f) * 3.1415901*2;
    float randomAngleTweak = (random(60)-30)/30.0f * 3.1415926f*0.125f;
    
    asteroidX[i] = randomInnerScreenX + cos(randomAngle)*(10 + random(5) + WIDTH/2);
    asteroidY[i] = randomInnerScreenY + sin(randomAngle)*(10 + random(5) + WIDTH/2);

    asteroidVelX[i] = cos(3.14159f+randomAngle + randomAngleTweak) * 0.75f;
    asteroidVelY[i] = sin(3.14159f+randomAngle + randomAngleTweak) * 0.75f;

    if (asteroidActive[i] == 1) {

    }
    else {
      asteroids++;
    }
    asteroidActive[i] = 1;
    asteroidBeenOnScreen[i] = 0;
    asteroidCounter = (asteroidCounter+1) % MAX_ASTEROIDS;
    return i;
}


float wrapX(float x, float wrapT) {
  if (x > WIDTH + wrapT) {
    x = x - WIDTH;
  }
  else if (x < -wrapT) {
    x = x + WIDTH;
  }
  return x;
}
float wrapY(float y, float wrapT) {
  if (y > HEIGHT + wrapT) {
    y = y - HEIGHT;
  }
  else if (y < -wrapT) {
    y = y + HEIGHT;
  }
  return y;
}
int lastShotTick;
Tinyfont* tinyfont;
// This function runs once in your game.
// use it for anything that needs to be set only once in your game.
void setup() {
  tinyfont = new Tinyfont(arduboy.getBuffer(), WIDTH,HEIGHT);

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
  for (int i = 0; i < MAX_SHIP_PARTICLES; i++) {
    shipParticleX[i] = -100;
    shipParticleY[i] = -100;
  }
  arduboy.initRandomSeed();
  arduboy.clear();
  
  arduboy.setCursor(16, 16);
  arduboy.setTextSize(1);
  arduboy.print(F("asteroids"));
  arduboy.setCursor(16, 32);
  arduboy.setTextSize(1);
  arduboy.print(F("by 00jknight"));
  //arduboy.print(1337);
  arduboy.display();
  arduboy.delayShort(2000);


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
  BeepPin2::noTone();
  // first we clear our screen to black
  
  if (DO_SCREEN_CLEAR)
    arduboy.clear();
  else
    scanClear();
 
  prevShipAngle = shipAngle;
  prevShipX = shipX;
  prevShipY = shipY;
  // thrusting
  bool playedSound = false;
  if (arduboy.pressed(B_BUTTON)) {
    float angle = 0.125f;
    //float angle = 0.0f;
    if (arduboy.pressed(LEFT_BUTTON)) {
      shipVelX += cos(shipAngle + 3.14159f*angle)*acceleration;
      shipVelY += sin(shipAngle + 3.14159f*angle)*acceleration;
    } else if (arduboy.pressed(RIGHT_BUTTON)) {
      shipVelX += cos(shipAngle - 3.14159f*angle)*acceleration;
      shipVelY += sin(shipAngle - 3.14159f*angle)*acceleration;
    }
    else {
      shipVelX += cos(shipAngle)*acceleration;
      shipVelY += sin(shipAngle)*acceleration;
    }

    if (arduboy.everyXFrames(2)){
      BeepPin2::tone(BeepPin2::freq(2.0f+(shipParticleIndex*0.5f)), 1);
    }
  } else {
    //BeepPin2::noTone();
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
    if (arduboy.pressed(B_BUTTON) || arduboy.pressed(DOWN_BUTTON)) {
      shipAngle-=rotSpeed*2.0f;
      
    }
    else
      shipAngle-=rotSpeed;
  }
  if (arduboy.pressed(RIGHT_BUTTON)) {
    if (arduboy.pressed(B_BUTTON) || arduboy.pressed(DOWN_BUTTON)) {
      shipAngle+=rotSpeed*2.0f;
    }
    else
      shipAngle+=rotSpeed;
  }
  if (arduboy.pressed(DOWN_BUTTON)) {
    shipVelX += cos(shipAngle)*-acceleration;
    shipVelY += sin(shipAngle)*-acceleration;
  }




  // draw the bullets
  Rect screen;
  screen.x = 0;
  screen.y = 0;
  screen.width = WIDTH;
  screen.height = HEIGHT;



  /*
  arduboy.print(WIDTH);
  arduboy.print(F(" x "));
  arduboy.print(HEIGHT);
  arduboy.print('\n');
  */
  // draw the asteroids
  for (int i = 0; i < MAX_SHIP_PARTICLES; i++) {
    //if (shipFrame[i] < shipLifeFrames) {
      //shipFrame[i]++;
      int x0 = shipParticleX[i];
      int y0 = shipParticleY[i];
      int x1 = shipParticleX[(i+2)%MAX_SHIP_PARTICLES];
      int y1 = shipParticleY[(i+2)%MAX_SHIP_PARTICLES];
      //arduboy.drawPixel((int)shipParticleX[i], (int)shipParticleY[i]);
      int deltaX = x1-x0;
      int deltaY = y1-y0;
      const int eps = 5;
      if ((deltaX < eps && deltaX > -eps) &&(deltaY < eps && deltaY > -eps)){
        arduboy.drawLine(x0, y0, x1,y1);
      }
      else {
        arduboy.drawPixel(x0, y0);
      }
    //}

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
      //arduboy.drawLine(xn1,yn1,xn2,yn2,BLACK);
      particleX[i] += particleVelX[i];
      particleY[i] += particleVelY[i];
      particleVelX[i] *= 0.8f;
      particleVelY[i] *= 0.8f;
      int x2= (int)particleX[i];
      int y2 = (int)particleY[i];
      //arduboy.drawPixel(2*(x2-x1) + x1, 2*(y2-y1) + y1, BLACK);
      if (!DO_SCREEN_CLEAR) {
        arduboy.drawPixel(x1, y1, BLACK);
        arduboy.drawPixel(x2, y2);
      }
      else {
        arduboy.drawLine((int)particleX[i], (int)particleY[i], x1,y1);  
      }
      //arduboy.fillCircle((int)particleX[i],(int)particleY[i],1,BLACK);
      //arduboy.drawLine((int)particleX[i], (int)particleY[i], x1,y1);
      particleFrame[i]++;
    }
    else {
      if (!DO_SCREEN_CLEAR) {
        arduboy.drawPixel((int)particleX[i], (int)particleY[i], BLACK);
      }
    }
  }

  
  for (int i = 0; i < MAX_ASTEROIDS; i++) {
      if(asteroidActive[i]) {
        Point asteroidPoint;
        if (!DO_SCREEN_CLEAR) {
          arduboy.fillCircle((int)asteroidX[i], (int)asteroidY[i], asteroidSize+2, BLACK);
        }
        int thisAsteroidSize = asteroidSize + asteroidSizes[i];
        asteroidX[i] += asteroidVelX[i];
        asteroidY[i] += asteroidVelY[i];
        asteroidPoint.x = (int)asteroidX[i];
        asteroidPoint.y = (int)asteroidY[i];
        Rect asteroidScreen;
        asteroidScreen.x = -thisAsteroidSize;
        asteroidScreen.y = -thisAsteroidSize;
        asteroidScreen.width = WIDTH + thisAsteroidSize*2;
        asteroidScreen.height = HEIGHT + thisAsteroidSize*2;
        arduboy.drawCircle((int)asteroidX[i], (int)asteroidY[i], thisAsteroidSize, WHITE);
      
        // kill ship
        float deltaX = asteroidX[i] - shipX;
        float deltaY = asteroidY[i] - shipY;
        float mutalRadius = shipSize + thisAsteroidSize;
        if (pow(deltaX,2) + pow(deltaY,2) < pow(mutalRadius,2)) {
          arduboy.clear();
          score = 0;
          asteroids = 0;
          shipX = WIDTH*0.5f;
          shipY = HEIGHT*0.5f;
          shipVelX = 0;
          shipVelY = 0;
          shipAngle = 0.0f;
          for (int i = 0; i < MAX_SHIP_PARTICLES; i++) {
            shipParticleX[i] = -100;
            shipParticleY[i] = -100;
          }
          break;
        }
        
        Rect innerScreen;
        innerScreen.x = thisAsteroidSize;
        innerScreen.y = thisAsteroidSize;
        innerScreen.width = WIDTH - 2*thisAsteroidSize;
        innerScreen.height = HEIGHT - 2*thisAsteroidSize;
        if (asteroidBeenOnScreen[i] || arduboy.collide(asteroidPoint, innerScreen)) {
          asteroidBeenOnScreen[i] = 1;  
          asteroidX[i] = wrapX(asteroidX[i], 1);
          asteroidY[i] = wrapY(asteroidY[i], 1);
          // maybe draw second asteroid
          int t = thisAsteroidSize;
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
            if (!DO_SCREEN_CLEAR) {
              arduboy.fillCircle(secondToidX-asteroidVelX[i], secondToidY-asteroidVelY[i], thisAsteroidSize+2, BLACK);
            }
            arduboy.drawCircle(secondToidX, secondToidY, thisAsteroidSize, WHITE);
          }
        }
      }
  }
  if (asteroids == 0) {
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
      asteroidActive[i] = 0;
      asteroidCounter = 0;
    }

    uint8_t level = min(3,(1 + (score / 8)));
    uint8_t levelSize = 3 + min(4,((score / 32)));
    uint8_t overwrote;
    for (int i = 0; i < level; i++) {
      spawnAsteroid(levelSize, 0, overwrote);
    }
  }
  
  else if (asteroids <=2) {
    int foundX = 7;
    int foundY = 7;
    int numFound = 0;
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
      if (asteroidActive[i] == 1 && asteroidBeenOnScreen[i] == 0) {
        asteroidBeenOnScreen[i] == 1;
      }
    }
  }
  
  for (int i = 0; i < MAX_BULLETS; i++) {
    Point bulletPoint;
    bulletPoint.x = (int)bulletX[i];
    bulletPoint.y = (int)bulletY[i];
    
    if (arduboy.collide(bulletPoint, screen)) {
      //bulletVelX[i] -= bulletVelX[i]*0.05f;
      //bulletVelY[i] -= bulletVelY[i]*0.05f;
      if (!DO_SCREEN_CLEAR) {
        arduboy.fillCircle((int)bulletX[i], (int)bulletY[i], bulletSize*2, BLACK);
      }
      bulletX[i] += bulletVelX[i];
      bulletY[i] += bulletVelY[i];
      
      arduboy.drawCircle((int)bulletX[i], (int)bulletY[i], bulletSize, WHITE);
      bool doBeep = false;
      for (int a = 0; a < MAX_ASTEROIDS; a++) {
      if (asteroidActive[a] == 1) {
        float deltaX = bulletX[i] - asteroidX[a];
        float deltaY = bulletY[i] - asteroidY[a];
        float mutalRadius = bulletSize + asteroidSize+asteroidSizes[a];
          if (pow(deltaX,2) + pow(deltaY,2) < pow(mutalRadius,2)) {
            score++;
            bulletX[i] = -200;
            bulletY[i] = -200;
            explosion(asteroidX[a], asteroidY[a]);
            doBeep = true;
            if (asteroidSizes[a] == 1) {
              asteroidActive[a] = 0;
              asteroids--;
              asteroidX[a] = -100.0f;
              asteroidY[a] = -100.0f;
            }else {
              float angle = 3.14159f * 0.333333f;
              asteroidSizes[a]--;
              float bulletAngle = atan2(bulletVelY[i], bulletVelX[i]);
              float prevVelX = asteroidVelX[a];
              float prevVelY = asteroidVelY[a];
              asteroidVelX[a] = 0.25f * prevVelX + 0.75f*cos(bulletAngle - angle)*1.35f;
              asteroidVelY[a] = 0.25f * prevVelY + 0.75f*sin(bulletAngle - angle)*1.35f;
              asteroidBeenOnScreen[a] = 1;
              uint8_t overwrote = 0;
              int a2 = spawnAsteroid(asteroidSizes[a], 0, overwrote);
              if (overwrote == 1) {
                asteroidBeenOnScreen[a2] = 1;
                asteroidVelX[a2] = 0.25f * prevVelX + 0.75f*cos(bulletAngle + angle)*1.35f;
                asteroidVelY[a2] = 0.25f * prevVelY + 0.75f*sin(bulletAngle + angle)*1.35f;
                asteroidX[a2] = asteroidX[a] + asteroidVelX[a2];
                asteroidY[a2] = asteroidY[a] + asteroidVelY[a2];
              }
            }
          }
        }
      }
      if (doBeep) {
        BeepPin2::tone(BeepPin2::freq(50.0f), 2);
      }
    }
  }
  // draw the ship
  //arduboy.fillCircle((int)shipX, (int)shipY, shipSize*0.5f, WHITE);
  // rotation matrix:
  //  cos   -sin
  //  sin    cos
  //int x1 = x0 * c - y0 * s;
  //int y1 = y0 * s + y0 * c;


  float shipSize = 3.0f;
  float oneThird = 2.0f / 3.0f * 3.14159;

  // undraw previous ship
  {
    
    /*
    int x0 = prevShipX + cos(prevShipAngle)*2*shipSize;
    int y0 = prevShipY + sin(prevShipAngle)*2*shipSize;
    
    int x1 = prevShipX + cos(prevShipAngle - oneThird)*shipSize;
    int y1 = prevShipY + sin(prevShipAngle - oneThird)*shipSize;

    int x2 = prevShipX + cos(prevShipAngle + oneThird)*shipSize;
    int y2 = prevShipY + sin(prevShipAngle + oneThird)*shipSize;
    */




    /*
    int x0 = shipX + cos(prevShipAngle)*shipSize;
    int y0 = shipY + sin(prevShipAngle)*shipSize;
    int x1 = shipX + cos(prevShipAngle - 1.25f*oneThird)*shipSize;
    int y1 = shipY + sin(prevShipAngle - 1.25f*oneThird)*shipSize;

    int x2 = shipX + cos(prevShipAngle + 1.25f*oneThird)*shipSize;
    int y2 = shipY + sin(prevShipAngle + 1.25f*oneThird)*shipSize;
    arduboy.drawLine(x0,y0,x1,y1,BLACK);
    arduboy.drawLine(x1,y1,x2,y2,BLACK);
    arduboy.drawLine(x2,y2,x0,y0,BLACK);
    arduboy.fillCircle(prevShipX,prevShipY, 2, BLACK);
    */


    drawLine(prevShipX, prevShipY, shipSize, prevShipAngle, BLACK);

    //arduboy.drawCircle(x0,y0,3, WHITE);


    //arduboy.drawRect(shipX-halfRectshipSize,shipY-halfRectshipSize, halfRectshipSize*2,halfRectshipSize*2, BLACK);
  }
  
  // move ship
  shipX = shipX + shipVelX;
  shipY = shipY + shipVelY;
  shipX = wrapX(shipX, 1);
  shipY = wrapY(shipY, 1);

  //arduboy.setTextColor(0x3c3c3c);
  //arduboy.setTextshipSize(1);  
  //arduboy.setCursor(0, HEIGHT-8);
  //arduboy.print(score);

  
  int highScore = 1337;
  char highScoreBuffer[8];
  int nchars = sprintf(highScoreBuffer, "%d", score);
  tinyfont->setCursor(0,HEIGHT-4);
  for (int i = 0; i < nchars; i++) {
    tinyfont->write(highScoreBuffer[i]);
  }
  
  //tinyfont->setCursor(8,HEIGHT-8);
  //tinyfont->printChar((char)score, 8, HEIGHT-8);
  /*
  tinyfont->setCursor(0,0);
  
  nchars = sprintf(highScoreBuffer, "%d", highScore);
  for (int i = 0; i < nchars; i++) {
    tinyfont->write(highScoreBuffer[i]);
  }
  */
  /*
  do {
  tinyfont->write('0'+);
  highScore 
  } while(highScore > 0);
  */
  //tinyfont->printChar('w', 8, HEIGHT-8);
  //tinyfont->printChar('t', 12, HEIGHT-8);
  //tinyfont->printChar('f', 16, HEIGHT-8);
  // draw ship
  {
    int x0 = shipX + cos(shipAngle)*shipSize;
    int y0 = shipY + sin(shipAngle)*shipSize;
    int x1 = shipX + cos(shipAngle - 1.25f*oneThird)*shipSize;
    int y1 = shipY + sin(shipAngle - 1.25f*oneThird)*shipSize;

    int x2 = shipX + cos(shipAngle + 1.25f*oneThird)*shipSize;
    int y2 = shipY + sin(shipAngle + 1.25f*oneThird)*shipSize;
    arduboy.fillCircle(shipX + (shipVelX*1.2f),shipY + (shipVelY*1.2f),shipSize, BLACK);

    arduboy.drawLine(x0,y0,x1,y1,WHITE);
    arduboy.drawLine(x1,y1,x2,y2,WHITE);
    arduboy.drawLine(x2,y2,x0,y0,WHITE);



  // maybe draw second ship
  {
    int secondX = shipX;
    int secondY = shipY;
    bool drawSecondShip = false;
    if (secondX < shipSize) {
      secondX += WIDTH;
      drawSecondShip = true;
    } else if (secondX > WIDTH-shipSize) {
      secondX -= WIDTH;
      drawSecondShip = true;
    }
    if (secondY < shipSize) {
      secondY += HEIGHT;
      drawSecondShip = true;
    } else if (secondY > HEIGHT-shipSize) {
      secondY -= HEIGHT;
      drawSecondShip = true;
    }
    if (drawSecondShip) {
      int x0 = secondX + cos(shipAngle)*shipSize;
      int y0 = secondY + sin(shipAngle)*shipSize;
      int x1 = secondX + cos(shipAngle - 1.25f*oneThird)*shipSize;
      int y1 = secondY + sin(shipAngle - 1.25f*oneThird)*shipSize;

      int x2 = secondX + cos(shipAngle + 1.25f*oneThird)*shipSize;
      int y2 = secondY + sin(shipAngle + 1.25f*oneThird)*shipSize;
      arduboy.fillCircle(shipX + (shipVelX*1.2f),shipY + (shipVelY*1.2f),shipSize, BLACK);

      arduboy.drawLine(x0,y0,x1,y1,WHITE);
      arduboy.drawLine(x1,y1,x2,y2,WHITE);
      arduboy.drawLine(x2,y2,x0,y0,WHITE);
    }
  }

    //arduboy.drawCircle(shipX,shipY, shipSize, WHITE);
    //if (arduboy.pressed(B_BUTTON)) {
      float eps = 0.01f;
      if ((shipVelX > eps || shipVelX < -eps) || (shipVelY > eps || shipVelY < -eps)) {
        int p0 = shipX + cos(shipAngle+3.1415901f) *shipSize;
        int p1 = shipY + sin(shipAngle+3.1415901f) *shipSize;
        //spawnParticle(p0, p1, -shipVelX, -shipVelY);
        //int p0 = (x1 + x2 )/2;
        //int p1 = (y1 + y2 )/2;
        //arduboy.drawPixel(p0, p1, BLACK);

        //spawnParticle(0.5f*(x1+x2), 0.5f*(y1+y2), shipVelX*0.5f, shipVelY*0.5f);
        
        float v = DO_SCREEN_CLEAR ? 0.0f : 0.5f;

        spawnShipParticle(x2, y2);
        spawnShipParticle(x1, y1);
        //spawnShipParticle((x2+x1)/2, (y1+y2)/2, -shipVelX*v, -shipVelY*v);
        //spawnShipParticle(x1, y1);
        //spawnShipParticle(x2, y2);
      }
    //}
  }
  
  shipVelX = shipVelX - shipVelX*0.05f;  
  shipVelY = shipVelY - shipVelY*0.05f;
  // then we finaly we tell the arduboy to display what we just wrote to the display
  arduboy.display();
}
