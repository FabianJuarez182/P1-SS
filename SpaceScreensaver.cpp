#include <SDL2/SDL.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdlib>

// Estructura para representar un planeta
struct Planet {
    int centerX, centerY;  // Centro de la órbita
    float orbitRadius;     // Radio de la órbita
    float angle;           // Ángulo actual en la órbita
    float speed;           // Velocidad angular
    int size;              // Tamaño del planeta (radio)
    SDL_Color color;       // Color del planeta
    bool isVisible;        // Indica si el planeta está visible
    Uint32 respawnTime;    // Tiempo de regeneración
};

// Estructura para representar un asteroide
struct Asteroid {
    int x, y;
    int size;
    int dx, dy;            // Velocidad en x e y
    SDL_Color color;
};

// Estructura para representar una estrella
struct Star {
    int x, y;
    int size;
    float speed;           // Velocidad de movimiento
};

// Función para dibujar un círculo
void drawCircle(SDL_Renderer* renderer, int x, int y, int radius, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (int w = 0; w < radius * 2; w++) {
        for (int h = 0; h < radius * 2; h++) {
            int dx = radius - w; // desplazamiento horizontal
            int dy = radius - h; // desplazamiento vertical
            if ((dx * dx + dy * dy) <= (radius * radius)) {
                SDL_RenderDrawPoint(renderer, x + dx, y + dy);
            }
        }
    }
}

// Función para actualizar y dibujar un planeta
void updateAndDrawPlanet(SDL_Renderer* renderer, Planet& planet, Uint32 currentTime) {
    if (planet.isVisible) {
        planet.angle += planet.speed;
        int x = planet.centerX + planet.orbitRadius * cos(planet.angle);
        int y = planet.centerY + planet.orbitRadius * sin(planet.angle);

        // Crear un gradiente para hacer los planetas más realistas
        for (int i = 0; i < planet.size; i++) {
            SDL_Color gradientColor = { Uint8(planet.color.r * (planet.size - i) / planet.size),
                                        Uint8(planet.color.g * (planet.size - i) / planet.size),
                                        Uint8(planet.color.b * (planet.size - i) / planet.size) };
            drawCircle(renderer, x, y, planet.size - i, gradientColor);
        }
    } else if (currentTime >= planet.respawnTime) {
        planet.isVisible = true;
    }
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

// Función para verificar colisiones entre un asteroide y un planeta
bool checkCollision(Planet& planet, Asteroid& asteroid) {
    if (!planet.isVisible) return false;

    int planetX = planet.centerX + planet.orbitRadius * cos(planet.angle);
    int planetY = planet.centerY + planet.orbitRadius * sin(planet.angle);

    int deltaX = asteroid.x - planetX;
    int deltaY = asteroid.y - planetY;
    int distance = sqrt(deltaX * deltaX + deltaY * deltaY);

    return distance < (planet.size + asteroid.size);
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

// Función para dibujar un círculo delgado
void drawThinCircle(SDL_Renderer* renderer, int x, int y, int radius, SDL_Color color, int distortionAmount) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    
    // Dibujar solo los puntos en el borde del círculo
    for (int w = -radius; w <= radius; w++) {
        for (int h = -radius; h <= radius; h++) {
            if (w * w + h * h <= radius * radius && w * w + h * h > (radius - 1) * (radius - 1)) {
                // Agregar una pequeña distorsión aleatoria en cada punto
                int distortionX = (rand() % (distortionAmount * 2)) - distortionAmount;
                int distortionY = (rand() % (distortionAmount * 2)) - distortionAmount;
                SDL_RenderDrawPoint(renderer, x + w + distortionX, y + h + distortionY);
            }
        }
    }
}

// Función para dibujar un agujero negro con un disco de acreción inestable
void drawBlackHole(SDL_Renderer* renderer, int centerX, int centerY) {
    int blackHoleRadius = 50;              // Radio del agujero negro (centro completamente negro)
    int accretionDiskInnerRadius = 60;     // Radio interior del disco de acreción
    int accretionDiskOuterRadius = 80;    // Radio exterior del disco de acreción
    int distortionAmount = 2;              // Cantidad de distorsión para simular inestabilidad

    // 1. Dibujar el agujero negro en el centro (completamente negro)
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  // Color negro sólido
    for (int r = 0; r <= blackHoleRadius; r++) {
        for (int w = -r; w <= r; w++) {
            for (int h = -r; h <= r; h++) {
                if (w * w + h * h <= r * r) {  // Verifica que el punto esté dentro del círculo
                    SDL_RenderDrawPoint(renderer, centerX + w, centerY + h);
                }
            }
        }
    }

    // 2. Dibujar los aros delgados y distorsionados (disco de acreción)
    for (int r = accretionDiskInnerRadius; r <= accretionDiskOuterRadius; r += 5) {
        // Colores cálidos: amarillo a naranja
        int red = 255;
        int green = 165 + (r % 50);  // Degradado para obtener tonos naranja y amarillo
        int blue = 0;
        int alpha = 255 - (r - accretionDiskInnerRadius) * 2;  // Degradado de opacidad

        // Dibujar cada círculo de luz delgado con distorsión
        drawThinCircle(renderer, centerX, centerY, r, {Uint8(red), Uint8(green), Uint8(blue), Uint8(alpha)}, distortionAmount);
    }
}

// Función para actualizar y dibujar estrellas
void updateAndDrawStars(SDL_Renderer* renderer, std::vector<Star>& stars) {
    for (auto& star : stars) {
        star.y += star.speed;
        if (star.y >= 480) {
            star.y = 0;
            star.x = rand() % 640;
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        drawCircle(renderer, star.x, star.y, star.size, {255, 255, 255});
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
    std::vector<Star> stars;
    srand(time(0));
    bool running = true;
    Uint32 startTime = SDL_GetTicks();
    Uint32 frameCount = 0;
    Uint32 lastFPSTime = SDL_GetTicks();  // Para controlar la impresión del FPS cada segundo

    // Crear planetas en órbitas
    for (int i = 0; i < 8; i++) {
        Planet planet;
        planet.centerX = 320;
        planet.centerY = 240;
        planet.orbitRadius = 50 + i * 50;
        planet.angle = rand() % 360;
        planet.speed = 0.01f + (rand() % 10) * 0.001f;
        planet.size = 10 + rand() % 10;
        planet.color = { (Uint8)(rand() % 256), (Uint8)(rand() % 256), (Uint8)(rand() % 256) };
        planet.isVisible = true;
        planet.respawnTime = 0;
        planets.push_back(planet);
    }

    // Crear asteroides
    for (int i = 0; i < 5; i++) {
        Asteroid asteroid;
        asteroid.x = rand() % 640;
        asteroid.y = rand() % 480;
        asteroid.size = 5 + rand() % 10;
        asteroid.dx = 1 + rand() % 3;
        asteroid.dy = 1 + rand() % 3;
        asteroid.color = { 128, 128, 128 };
        asteroids.push_back(asteroid);
    }

    // Crear estrellas
    for (int i = 0; i < 100; i++) {
        Star star;
        star.x = rand() % 640;
        star.y = rand() % 480;
        star.size = 1 + rand() % 2;
        star.speed = 0.1f + (rand() % 10) * 0.01f; // Velocidad reducida
        stars.push_back(star);
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

        Uint32 currentTime = SDL_GetTicks();

        // Calcular FPS
        frameCount++;
        float fps = frameCount / ((currentTime - startTime) / 1000.0f);
        
        // Formatear FPS a dos enteros y dos decimales
        std::ostringstream fpsStream;
        fpsStream << std::fixed << std::setw(2) << std::setprecision(2) << fps;
        std::string fpsFormatted = fpsStream.str();

        // Actualizar título de la ventana
        std::string title = "Space Screensaver - FPS: " + fpsFormatted;
        SDL_SetWindowTitle(window, title.c_str());

        // Imprimir FPS en la terminal cada segundo
        if (currentTime - lastFPSTime >= 1000) {
            std::cout << "FPS: " << fpsFormatted << std::endl;
            lastFPSTime = currentTime;
        }

        // Limpiar la pantalla
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Dibujar estrellas
        updateAndDrawStars(renderer, stars);

        // Dibujar agujero negro en el centro de la pantalla
        drawBlackHole(renderer, 320, 240);

        // Dibujar nebulosas
        //if (rand() % 100 < 5) {
            //drawNebula(renderer);
        //}

        // Actualizar y dibujar planetas
        for (auto& planet : planets) {
            updateAndDrawPlanet(renderer, planet, currentTime);
        }

        // Actualizar y dibujar asteroides
        for (auto& asteroid : asteroids) {
            updateAndDrawAsteroid(renderer, asteroid);
        }

        // Verificar colisiones y gestionar la desaparición de planetas
        for (auto& planet : planets) {
            for (auto& asteroid : asteroids) {
                if (checkCollision(planet, asteroid)) {
                    planet.isVisible = false;
                    planet.respawnTime = currentTime + 5000; // Desaparece durante 5 segundos
                }
            }
        }

        // Actualizar la pantalla
        SDL_RenderPresent(renderer);

        // Controlar la tasa de cuadros por segundo
        Uint32 elapsedTime = SDL_GetTicks() - currentTime;
        if (elapsedTime < 16) {
            SDL_Delay(16 - elapsedTime);  // 60 FPS
        }
    }

    // Limpiar recursos
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
