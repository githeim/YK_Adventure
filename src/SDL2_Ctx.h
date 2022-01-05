#ifndef _SDL2_CTX_H_
#define _SDL2_CTX_H_ 
#include <chrono>
#include <string>
#include <unistd.h>
#include<map>


#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

#include "TMX_Reader/CTMX_Reader.h"


#define SCREEN_POS_X (0)
#define SCREEN_POS_Y (1350)

//#define SCREEN_WIDTH  (640)
//#define SCREEN_HEIGHT (480)
#define SCREEN_WIDTH  (1280)
#define SCREEN_HEIGHT (720)


#define SCREEN_FPS (50)
#define POINTS_COUNT (4)



int Init_SDL_ctx(SDL_Window* &pWindow, SDL_Renderer* &pRenderer); 
int DeInit_SDL_ctx(SDL_Window* &pWindow, SDL_Renderer* &pRenderer);

int Init_Fonts(TTF_Font* &pFont,int iFontSize = 18); 
int DeInit_Fonts(TTF_Font* &pFont); 
double Frame_Rate_Control(double dbFPS,double &dbTimeDiff_SEC);
#endif /* ifndef _SDL2_CTX_H_ */
