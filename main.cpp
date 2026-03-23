#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h> // Librería para fuentes
#include <SDL2/SDL_net.h> //libreria de red
#include <iostream>

using namespace std;

int main(int argc, char *argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init(); // Inicializar el sistema de fuentes
    SDLNet_Init(); //Inicializar SDL_Net
    

    IPaddress ip;

    //Escuhca el puerto 1234
    SDLNet_ResolveHost(&ip, "192.168.0.251", 1234);

    TCPsocket client = SDLNet_TCP_Open(&ip);
   
    SDL_Window *window = SDL_CreateWindow("SDL_ttf Android", 0, 0, 0, 0, SDL_WINDOW_FULLSCREEN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // 1. Cargar la fuente (Ajustá el nombre al archivo que tengas)
    // El '24' es el tamaño de la fuente
    TTF_Font *font = TTF_OpenFont("arial.ttf", 64);
    if (!font) {
        SDL_Log("No se pudo cargar la fuente: %s", TTF_GetError());
    }

    // 2. Crear una superficie con el texto
    SDL_Color white = {255, 255, 255, 255};
    SDL_Surface *surface = NULL;
    if (!client) {
        surface = TTF_RenderText_Solid(font, "[ERROR]: Server connection has broken...", white);
    } else {
        surface = TTF_RenderText_Solid(font, "[SERVER]: Server connection has Successful!!!", white);
        SDLNet_TCP_Close(client);
    }

    // 3. Convertir superficie a textura (más rápido para renderizar)
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

    // Definir dónde se va a dibujar el texto
    SDL_Rect destRect = { 50, 100, surface->w/2, surface->h/2 };

    int running = 1;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_FINGERDOWN) running = 0;
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Fondo negro
        SDL_RenderClear(renderer);

        // 4. Dibujar la textura del texto
        SDL_RenderCopy(renderer, texture, NULL, &destRect);

        SDL_RenderPresent(renderer);
    }

    // Limpieza
    SDLNet_Quit();
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_Quit();
    return 0;
}