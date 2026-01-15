#include "renderSDL.h"

int render_image(Image* image)
{
    // Default window size
    int winWidth = 600;
    int winHeight = 400;

    const int WIDTH = (int)(image->width);
    const int HEIGHT = (int)(image->height);

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    // Create a window
    SDL_Window* window = SDL_CreateWindow("SIGNALS", SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED, winWidth, winHeight,
            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    if (!window) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    SDL_Renderer* renderer
            = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Auto scales image to aspect ratio of image
    SDL_RenderSetLogicalSize(renderer, WIDTH, HEIGHT);

    // Create a texture
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_BGR24,
            SDL_TEXTUREACCESS_STATIC, WIDTH, HEIGHT);

    // Upload pixels to the GPU
    SDL_UpdateTexture(texture, NULL, image->pixelData, WIDTH * 3);

    // Run the main event loop
    int running = 1;
    SDL_Event event;
    while (running) {

        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }

            // Close the window if user presses ESC
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = 0;
                }
            }
        }

        // Clear screen
        SDL_RenderClear(renderer);

        // Copy texture to renderer
        SDL_RenderCopy(renderer, texture, NULL, NULL);

        // Show result
        SDL_RenderPresent(renderer);
    }

    // Cleanup
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
