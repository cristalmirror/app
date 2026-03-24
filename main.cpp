#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_net.h>
#include <string>
#include <iostream>

using namespace std;

int main(int argc, char *argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    SDLNet_Init();

    // Obtener resolución real de la pantalla
    SDL_DisplayMode dm;
    SDL_GetDesktopDisplayMode(0, &dm);
    int screenW = dm.w;
    int screenH = dm.h;

    SDL_Window *window = SDL_CreateWindow("Chat Android", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenW, screenH, SDL_WINDOW_FULLSCREEN | SDL_WINDOW_OPENGL);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    IPaddress ip;
    // IMPORTANTE: Asegurate que la IP sea accesible desde el WiFi del celular
    if (SDLNet_ResolveHost(&ip, "192.168.0.251", 1234) == -1) {
        SDL_Log("Error resolviendo host");
    }
    
    TCPsocket client = SDLNet_TCP_Open(&ip);

    string input_text = "Escribe...";
    SDL_Color white = {255, 255, 255, 255};
    
    // UI adaptativa simple
    SDL_Rect text_box = { 50, 200, screenW - 100, 120 };
    SDL_Rect send_button = { 50, 350, 300, 100 };

    TTF_Font *font = TTF_OpenFont("arial.ttf", 64);
    if (!font) SDL_Log("Error fuente: %s", TTF_GetError());

    // Estado de la conexión para mostrar en pantalla
    string status_msg = client ? "[SERVER]: Conectado!" : "[ERROR]: Sin conexion";
    SDL_Surface* sSurf = TTF_RenderText_Blended(font, status_msg.c_str(), white);
    SDL_Texture* sTex = SDL_CreateTextureFromSurface(renderer, sSurf);
    SDL_Rect sRect = { 50, 50, sSurf->w / 2, sSurf->h / 2 };
    SDL_FreeSurface(sSurf);

    SDL_StartTextInput();

    int running = 1;
    SDL_Event event;
    SDL_Texture* textTex = nullptr; // Para el texto dinámico

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = 0;

            if (event.type == SDL_TEXTINPUT) {
                if (input_text == "Escribe...") input_text = "";
                input_text += event.text.text;
            }

            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_BACKSPACE && !input_text.empty()) {
                    input_text.pop_back();
                }
            }

            if (event.type == SDL_FINGERDOWN) {
                // Convertir coordenadas normalizadas a píxeles
                int tx = event.tfinger.x * screenW;
                int ty = event.tfinger.y * screenH;

                // Si toca el botón "Enviar"
                if (tx > send_button.x && tx < send_button.x + send_button.w &&
                    ty > send_button.y && ty < send_button.y + send_button.h) {
                    
                    if (client && !input_text.empty()) {
                        SDLNet_TCP_Send(client, input_text.c_str(), input_text.length() + 1);
                        input_text = "";
                    }
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
        SDL_RenderClear(renderer);

        // Dibujar UI
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &text_box);
        SDL_SetRenderDrawColor(renderer, 0, 200, 0, 255);
        SDL_RenderFillRect(renderer, &send_button);

        // Render Status
        SDL_RenderCopy(renderer, sTex, NULL, &sRect);

        // Render Texto de entrada (Optimizado: solo si hay texto)
        if (!input_text.empty()) {
            SDL_Surface *surf = TTF_RenderText_Blended(font, input_text.c_str(), white);
            if (surf) {
                textTex = SDL_CreateTextureFromSurface(renderer, surf);
                SDL_Rect tPos = {text_box.x + 10, text_box.y + 10, surf->w, surf->h};
                // Ajuste simple para que el texto no se salga de la caja
                if (tPos.w > text_box.w - 20) tPos.w = text_box.w - 20; 
                
                SDL_RenderCopy(renderer, textTex, NULL, &tPos);
                
                // LIMPIEZA CRÍTICA: Liberar en cada frame
                SDL_FreeSurface(surf);
                SDL_DestroyTexture(textTex); 
            }
        }

        SDL_RenderPresent(renderer);
    }

    // Limpieza final
    SDL_DestroyTexture(sTex);
    if (client) SDLNet_TCP_Close(client);
    TTF_CloseFont(font);
    SDL_StopTextInput();
    SDLNet_Quit();
    TTF_Quit();
    SDL_Quit();
    return 0;
}