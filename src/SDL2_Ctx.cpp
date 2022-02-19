#include "SDL2_Ctx.h"
int Init_SDL_ctx(SDL_Window* &pWindow, SDL_Renderer* &pRenderer) {
  pWindow = NULL;

  //Initialize SDL
  if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
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


  return 0;
}

int DeInit_SDL_ctx(SDL_Window* &pWindow, SDL_Renderer* &pRenderer) {
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
