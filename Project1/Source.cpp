/*This source code copyrighted by Lazy Foo' Productions (2004-2015)
and may not be redistributed without written permission.*/

//Using SDL and standard IO
#include <SDL.h>
#include <stdio.h>
#include "chip8.h"
//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

int main(int argc, char* args[])
{
	//The window we'll be rendering to
	SDL_Window* window = NULL;

	//The surface contained by the window
	SDL_Surface* screenSurface = NULL;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
	}
	else
	{
		//Create window
		window = SDL_CreateWindow("ChiPP8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (window == NULL)
		{
			printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		}
		else
		{
			//Get window surface
			screenSurface = SDL_GetWindowSurface(window);
			SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, 0);
			SDL_Texture * texture = SDL_CreateTexture(renderer,
				SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, 32, 64);
			unsigned char * pixels = new unsigned char[32 * 64];
			FILE* rom;
			errno_t ferror;
			chip8* chip;
			if ((ferror = fopen_s(&rom, "rom", "rb")) != 0) {
				std::cout << "Unable to open ROM" << std::endl;
				return 1;
			}
			else {


				chip = new chip8(rom);
			}
			fclose(rom);
			int returnStat;
				while ((returnStat = chip->stepCycle()) != 99) {
					if (returnStat != 2) {
						unsigned char* tempScreen = chip->getgfx();
						for (int i = 0; i < 2048; i++) {
							if (tempScreen[i] == 1) {
								pixels[i] = 255;
							}
						}
					}
					SDL_UpdateTexture(texture, NULL, pixels, 32 * sizeof(unsigned char));
					//Update the surface
					SDL_UpdateWindowSurface(window);
					SDL_RenderClear(renderer);
					SDL_RenderCopy(renderer, texture, NULL, NULL);
					SDL_RenderPresent(renderer);
				}
			
			delete[] pixels;
			SDL_DestroyTexture(texture);
			SDL_DestroyRenderer(renderer);

			

		}
	}

	//Destroy window
	SDL_DestroyWindow(window);

	//Quit SDL subsystems
	SDL_Quit();

	return 0;
}