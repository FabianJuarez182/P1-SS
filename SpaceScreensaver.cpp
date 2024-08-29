#include <SDL2/SDL.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <vector>

// Estructura para representar un planeta
struct Planet {
    int centerX, centerY;  // Centro de la órbita
    float orbitRadius;     // Radio de la órbita
    float angle;           // Ángulo actual en la órbita
    float speed;           // Velocidad angular
    int size;              // Tamaño del planeta
    SDL_Color color;       // Color del planeta
};

// Estructura para representar un asteroide
struct Asteroid {
    int x, y;
    int size;
    int dx, dy;            // Velocidad en x e y
    SDL_Color color;
};

// Función para actualizar y dibujar un planeta
void updateAndDrawPlanet(SDL_Renderer* renderer, Planet& planet) {
    planet.angle += planet.speed;
    int x = planet.centerX + planet.orbitRadius * cos(planet.angle);
    int y = planet.centerY + planet.orbitRadius * sin(planet.angle);

    SDL_SetRenderDrawColor(renderer, planet.color.r, planet.color.g, planet.color.b, 255);
    SDL_Rect rect = { x - planet.size / 2, y - planet.size / 2, planet.size, planet.size };
    SDL_RenderFillRect(renderer, &rect);
}

// Función para actualizar y dibujar un asteroide
void updateAndDrawAsteroid(SDL_Renderer* renderer, Asteroid& asteroid) {
    asteroid.x += asteroid.dx;
    asteroid.y += asteroid.dy;

    // Rebote en los bordes
    if (asteroid.x <= 0 || asteroid.x >= 640) asteroid.dx = -asteroid.dx;
    if (asteroid.y <= 0 || asteroid.y >= 480) asteroid.dy = -asteroid.dy;

    SDL_SetRenderDrawColor(renderer, asteroid.color.r, asteroid.color.g, asteroid.color.b, 255);
    SDL_Rect rect = { asteroid.x - asteroid.size / 2, asteroid.y - asteroid.size / 2, asteroid.size, asteroid.size };
    SDL_RenderFillRect(renderer, &rect);
}

// Función para dibujar una nebulosa
void drawNebula(SDL_Renderer* renderer) {
    int centerX = rand() % 640;
    int centerY = rand() % 480;
    int radius = 50 + rand() % 100;

    for (int i = 0; i < radius; i++) {
        SDL_SetRenderDrawColor(renderer, rand() % 256, rand() % 256, rand() % 256, 128);
        SDL_Rect rect = { centerX - i, centerY - i, 2 * i, 2 * i };
        SDL_RenderFillRect(renderer, &rect);
    }
}

// Función para dibujar un agujero negro
void drawBlackHole(SDL_Renderer* renderer, int centerX, int centerY) {
    int radius = 50;
    for (int i = 0; i < radius; i++) {
        int intensity = 255 - (i * 5);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, intensity);
        SDL_Rect rect = { centerX - i, centerY - i, 2 * i, 2 * i };
        SDL_RenderFillRect(renderer, &rect);
    }
}

// Función para dibujar estrellas
void drawStars(SDL_Renderer* renderer) {
    for (int i = 0; i < 100; i++) {
        int x = rand() % 640;
        int y = rand() % 480;
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawPoint(renderer, x, y);
    }
}

int main(int argc, char* argv[]) {
    // Inicialización de SDL
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Space Screensaver",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          640, 480, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Inicialización de variables
    std::vector<Planet> planets;
    std::vector<Asteroid> asteroids;
    srand(time(0));
    bool running = true;
    Uint32 startTime = SDL_GetTicks();

    // Crear planetas en órbitas
    for (int i = 0; i < 5; i++) {
        Planet planet;
        planet.centerX = 320;
        planet.centerY = 240;
        planet.orbitRadius = 50 + i * 50;
        planet.angle = rand() % 360;
        planet.speed = 0.01f + (rand() % 10) * 0.001f;
        planet.size = 10 + rand() % 10;
        planet.color = { (Uint8)(rand() % 256), (Uint8)(rand() % 256), (Uint8)(rand() % 256) };
        planets.push_back(planet);
    }

    // Crear asteroides
    for (int i = 0; i < 10; i++) {
        Asteroid asteroid;
        asteroid.x = rand() % 640;
        asteroid.y = rand() % 480;
        asteroid.size = 5 + rand() % 10;
        asteroid.dx = 1 + rand() % 3;
        asteroid.dy = 1 + rand() % 3;
        asteroid.color = { 128, 128, 128 };
        asteroids.push_back(asteroid);
    }

    // Bucle principal
    while (running) {
        // Manejo de eventos
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        // Limpiar la pantalla
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Dibujar estrellas
        drawStars(renderer);

        // Dibujar agujero negro
        drawBlackHole(renderer, 320, 240);

        // Dibujar nebulosas
        if (rand() % 100 < 5) {
            drawNebula(renderer);
        }

        // Actualizar y dibujar planetas
        for (auto& planet : planets) {
            updateAndDrawPlanet(renderer, planet);
        }

        // Actualizar y dibujar asteroides
        for (auto& asteroid : asteroids) {
            updateAndDrawAsteroid(renderer, asteroid);
        }

        // Actualizar la pantalla
        SDL_RenderPresent(renderer);

        // Controlar la tasa de cuadros por segundo
        Uint32 elapsedTime = SDL_GetTicks() - startTime;
        if (elapsedTime < 16) {
            SDL_Delay(16 - elapsedTime);  // 60 FPS
        }
        startTime = SDL_GetTicks();
    }

    // Limpiar recursos
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

