#include "SDL2_Ctx.h"

void Init_Joystick();
void DeInit_Joystick();
void Init_Sound();
void DeInit_Sound();

int Init_SDL_ctx(SDL_Window* &pWindow, SDL_Renderer* &pRenderer) {
  pWindow = NULL;

  //Initialize SDL
  if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO ) <0)   
  {
    printf("\033[1;31m[%s][%d] :x: Could not Init SDL2 [%s] \033[m\n",
        __FUNCTION__,__LINE__, SDL_GetError());
    return -1;
  }

  //pWindow = SDL_CreateWindow( "YK_Adventure", 
  pWindow = SDL_CreateWindow( "phys_test", 
      SCREEN_POS_X, SCREEN_POS_Y, 
      SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
  if( pWindow == NULL )
  {
    printf("\033[1;31m[%s][%d] :x: error %s \033[m\n",
        __FUNCTION__,__LINE__,SDL_GetError());
    return -1;
  }
  // initial cursor position
  SDL_WarpMouseInWindow(pWindow, INITIAL_CURSOR_POS_X, INITIAL_CURSOR_POS_Y);
	pRenderer = SDL_CreateRenderer( pWindow, -1, 
      SDL_RENDERER_ACCELERATED  );

  if (!pRenderer) {
    printf("\033[1;31m[%s][%d] :x: error %s \033[m\n",
        __FUNCTION__,__LINE__,SDL_GetError());
    return -1;
  }

  Init_Joystick();
  Init_Sound();

  return 0;
}

int DeInit_SDL_ctx(SDL_Window* &pWindow, SDL_Renderer* &pRenderer) {
  DeInit_Sound();
  DeInit_Joystick();
  SDL_RenderClear(pRenderer);
  SDL_DestroyRenderer(pRenderer);
  //Destroy window
  SDL_DestroyWindow( pWindow );
  pWindow = NULL;
  //Quit SDL subsystems
  SDL_Quit();

  return 0;
}

int Init_Fonts(TTF_Font* &pFont,int iFontSize) {
  if( TTF_Init() == -1 ) {
    printf("\033[1;33m[%s][%d] :x: SDL_ttf could not initialize!"
        " SDL_ttf Error: %s \033[m\n",__FUNCTION__,__LINE__,TTF_GetError());
    return -1;
  }
	pFont = TTF_OpenFont( "resource/fonts/NanumGothicCoding-Regular.ttf", 
                        iFontSize );
	if( pFont == NULL )
	{
    printf("\033[1;31m[%s][%d] :x: Failed to load font [%s] \033[m\n",
        __FUNCTION__,__LINE__,TTF_GetError());
    return -1;
	}
  else {
    
  }
  return 0;
}
int DeInit_Fonts(TTF_Font* &pFont) {
	TTF_CloseFont( pFont );
  return 0;
}
int DrawText(SDL_Texture* &pTxtTexture,TTF_Font* &pFont,
    std::string strText,SDL_Renderer* &pRenderer) 
{
  SDL_Color textColor = { 255, 255, 0 };
	SDL_Surface* pTxtSurface = TTF_RenderText_Solid( pFont, strText.c_str(),
      textColor );
  if( pTxtSurface == NULL )
	{
    printf("\033[1;31m[%s][%d] :x: SDL_ttf Error: %s \033[m\n",
        __FUNCTION__,__LINE__, TTF_GetError());
    return -1;
	}
  pTxtTexture = SDL_CreateTextureFromSurface(pRenderer, pTxtSurface);
  SDL_FreeSurface(pTxtSurface);
	
  return 0;
}



/**
 * @brief Control frame rates
 *
 * @param dbFPS[IN]
 * @param dbTimeDiff[OUT]
 *
 * @return  Actual FPS
 */
double Frame_Rate_Control(double dbFPS,double &dbTimeDiff_SEC){
  static std::chrono::time_point<std::chrono::system_clock> Frame_time;
  std::chrono::duration<double> Frame_diff_SEC;

  Frame_diff_SEC = std::chrono::system_clock::now() - Frame_time;
  if (Frame_diff_SEC.count() < SCREEN_INTERVAL_TIME_SEC ) {
    usleep( (SCREEN_INTERVAL_TIME_SEC-Frame_diff_SEC.count())*1000000);
  }
  Frame_time = std::chrono::system_clock::now();

  static std::chrono::time_point<std::chrono::system_clock> Actual_time;
  std::chrono::duration<double> Actual_diff_SEC = 
                                 std::chrono::system_clock::now() - Actual_time;
  Actual_time = std::chrono::system_clock::now();
  dbTimeDiff_SEC = Actual_diff_SEC.count();
  return (1.0/dbTimeDiff_SEC);

}

std::map<int,SDL_Joystick*> g_mapJoystickInstance;

void Init_Joystick() {
  printf("\033[1;33m[%s][%d] :x: Init Joystick \033[m\n",__FUNCTION__,__LINE__);
  int iNumJoysticks = SDL_NumJoysticks();

  if( SDL_NumJoysticks() < 1 )
  {
    printf("\033[1;31m[%s][%d] :x: Warning: No joysticks connected! \033[m\n",
        __FUNCTION__,__LINE__);
  }
  else
  {
    printf("\033[1;33m[%s][%d] :x: The number of joysticks = %d \033[m\n",
        __FUNCTION__,__LINE__,iNumJoysticks);

    SDL_JoystickEventState(SDL_ENABLE);
    for (int i =0 ; i < iNumJoysticks ; i++ ) {
      SDL_Joystick *pJoystick = SDL_JoystickOpen(i);
      printf("\033[1;33m[%s][%d] :x: Open Joystick [%d] \033[m\n",
          __FUNCTION__,__LINE__,i);

      if (pJoystick != nullptr) {
        g_mapJoystickInstance[i] = pJoystick;
      } else {
        printf("\033[1;31m[%s][%d] :x: Error Joystick [%d] Unable to open\033[m\n",
            __FUNCTION__,__LINE__,i);
      }
    }
  }
  printf("\033[1;33m[%s][%d] :x: Init Joystick Done \033[m\n",
      __FUNCTION__,__LINE__);
  return ;
}
void DeInit_Joystick() {
  for (auto Item : g_mapJoystickInstance) {
    SDL_Joystick* &pJoystick = Item.second;
    if (pJoystick != nullptr) {
      SDL_JoystickClose(pJoystick);
      pJoystick = nullptr;
    }
  }
  printf("\033[1;33m[%s][%d] :x: DeInit Joystick Done \033[m\n",
      __FUNCTION__,__LINE__);

  return ;
}

std::map<std::string,Mix_Chunk *> g_mapSound;
std::map <std::string, std::string> g_mapSoundPath {
  { "tang", "resource/sound/tang.wav" },
  { "boom", "resource/sound/boom.wav" }
};

void Init_Sound() {

  printf("\033[1;33m[%s][%d] :x: Init \033[m\n",__FUNCTION__,__LINE__);
  if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 )
  //if( Mix_OpenAudio( 22050, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 )
  {
    printf("\033[1;31m[%s][%d] :x: SDL_mixer , Init Err \033[m\n",
        __FUNCTION__,__LINE__);
  } 
  else {
    printf("\033[1;33m[%s][%d] :x: SDL mixer start \033[m\n",__FUNCTION__,__LINE__);
    for (auto item : g_mapSoundPath) {
      Mix_Chunk *pSound;
      std::string strName = item.first;
      std::string strFilePath = item.second;
      pSound =  Mix_LoadWAV( strFilePath.c_str() );
      if (pSound) {
        g_mapSound[strName]=pSound;
      }
    }

    //Mix_PlayChannel( -1, pSound, 0 );
  }

}
void DeInit_Sound() {
  for (auto item : g_mapSound) {
    Mix_Chunk *&pSound = item.second;
    if (pSound) {
      Mix_FreeChunk(pSound); 
      pSound = nullptr;
      printf("\033[1;33m[%s][%d] :x: DeInit sound [%s] \033[m\n",
          __FUNCTION__,__LINE__,item.first);

    }
  }
  Mix_CloseAudio();
  printf("\033[1;33m[%s][%d] :x: DeInit Sound Done\033[m\n",
      __FUNCTION__,__LINE__);
}
bool g_bSoundOnff = true;
void Set_Sound(bool bOnOff) {
  g_bSoundOnff = bOnOff;
}
void Play_Sound(std::string strSound) {
  if (g_bSoundOnff) 
    Mix_PlayChannel( -1, g_mapSound[strSound], 0 );
}
