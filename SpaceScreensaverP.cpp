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
#include <omp.h>

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

struct Explosion {
    int x, y;              // Posición de la explosión
    int radiusOrange;       // Radio actual del círculo naranja
    int radiusYellow;       // Radio actual del círculo amarillo
    int radiusWhite;        // Radio actual del círculo blanco
    int maxRadius;          // Radio máximo de la explosión
    bool isActive;          // Si la explosión está activa o no
};

struct NebulaPoint {
    float x, y;               // Posici�n del punto
    float dx, dy;             // Velocidad en x e y
    int size;                 // Tama�o del punto
    SDL_Color color;          // Color actual del punto
    float colorTransition;    // Controla la transici�n suave entre colores
    int colorStage;           // Etapa de color actual (0: verde, 1: celeste, etc.)
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

std::vector<NebulaPoint> nebulaPoints;
void initNebula(int numPoints) {
    nebulaPoints.resize(numPoints);

    #pragma omp parallel for
    for (auto& point : nebulaPoints) {
        // Generar posici�n aleatoria en los bordes exteriores de la pantalla
        point.x = (rand() % 640);
        point.y = (rand() % 480);
        
        // Tama�o peque�o
        point.size = 1 + rand() % 2;

        // Velocidad inicial (muy lenta, se acelerar� gradualmente)
        point.dx = 0;
        point.dy = 0;

        // Generar colores en la gama de azul, morado, rosado
        int red = 150 + rand() % 106;
        int green = 50 + rand() % 100;
        int blue = 150 + rand() % 106;
        point.color = { Uint8(red), Uint8(green), Uint8(blue), 128 }; // Semi-transparente
    }
}

void updateAndDrawNebula(SDL_Renderer* renderer) {
    const float centerX = 320.0f;
    const float centerY = 240.0f;

    // Colores clave para la interpolaci�n
    SDL_Color colors[] = {
        {128, 0, 128, 255},  // Morado
        {194, 52, 158, 255},  // M Morado
        {255, 105, 180, 255}, // Rosado
        {0, 255, 255, 255},  // Celeste
        {0, 128, 255, 255},  // M Celeste
        {64, 0, 194, 255}    // M azul
    };
    const int numColors = 6;

    // Funci�n de interpolaci�n de color
    auto interpolateColor = [](SDL_Color colorA, SDL_Color colorB, float t) -> SDL_Color {
        SDL_Color result;
        result.r = Uint8(colorA.r + t * (colorB.r - colorA.r));
        result.g = Uint8(colorA.g + t * (colorB.g - colorA.g));
        result.b = Uint8(colorA.b + t * (colorB.b - colorA.b));
        result.a = 255;  // Mantener la opacidad fija
        return result;
    };

    for (auto& point : nebulaPoints) {
        // Calcular la direcci�n hacia el centro
        float directionX = centerX - point.x;
        float directionY = centerY - point.y;

        // Calcular la distancia al centro
        float distance = sqrt(directionX * directionX + directionY * directionY);

        // Acelerar el punto mientras se acerca al centro
        if (distance > 0) {
            point.dx += (directionX / distance) * 0.05f;  // Aumentar velocidad hacia el centro
            point.dy += (directionY / distance) * 0.05f;
        }

        // Actualizar la posici�n del punto
        point.x += point.dx;
        point.y += point.dy;

        // Si el punto ha llegado cerca del centro, regenerarlo
        if (distance < 5) {
            // Reaparecer en una nueva posici�n aleatoria en los bordes exteriores
            point.x = (rand() % 640);
            point.y = (rand() % 480);
            point.dx = 0;
            point.dy = 0;
            point.colorStage = rand() % numColors; // Comienza en una etapa de color aleatoria
        }

        // Interpolar el color suavemente
        SDL_Color startColor = colors[point.colorStage];
        SDL_Color endColor = colors[(point.colorStage + 1) % numColors];
        point.color = interpolateColor(startColor, endColor, point.colorTransition);

        // Incrementar la transici�n de color
        point.colorTransition += 0.02f;  // Ajustar velocidad de cambio de color
        if (point.colorTransition >= 1.0f) {
            point.colorTransition = 0.0f;
            point.colorStage = (point.colorStage + 1) % numColors;  // Cambiar al siguiente color
        }

        // Dibujar el punto de la nebulosa
        SDL_SetRenderDrawColor(renderer, point.color.r, point.color.g, point.color.b, point.color.a);
        SDL_Rect rect = { int(point.x), int(point.y), point.size, point.size };
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

void drawThinLine(SDL_Renderer* renderer, int centerX, int centerY, int length, int thickness, SDL_Color color, int distortionAmount) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    // Dibujar la l�nea horizontal con distorsi�n
    for (int x = -length / 2; x <= length / 2; x++) {
        for (int y = -thickness / 2; y <= thickness / 2; y++) {
            // Agregar una peque�a distorsi�n aleatoria en cada punto de la l�nea
            int distortionX = (rand() % (distortionAmount * 2)) - distortionAmount;
            int distortionY = (rand() % (distortionAmount * 2)) - distortionAmount;
            SDL_RenderDrawPoint(renderer, centerX + x + distortionX, centerY + y + distortionY);
        }
    }
}

void drawAccretionDisk(SDL_Renderer* renderer) {
    SDL_Color diskColor = {255, 165, 0, 255};  // Naranja
    int diskLength = 200;  // Longitud del disco
    int diskThickness = 7; // Grosor del disco
    int distortionAmount = 3;  // Cantidad de distorsi�n

    // Dibujar el disco de acreci�n frente al agujero negro
    drawThinLine(renderer, 320, 240, diskLength, diskThickness, diskColor, distortionAmount);
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
    
    drawAccretionDisk(renderer);
}

// Función para actualizar y dibujar estrellas
void updateAndDrawStars(SDL_Renderer* renderer, std::vector<Star>& stars) {
    #pragma omp parallel for
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

// Función para iniciar una explosión
void startExplosion(Explosion& explosion, int x, int y) {
    explosion.x = x;
    explosion.y = y;
    explosion.radiusOrange = 1;    // Comienza con un radio pequeño
    explosion.radiusYellow = 0;    // El círculo amarillo tiene un pequeño retraso
    explosion.radiusWhite = 0;     // El círculo blanco tiene más retraso
    explosion.maxRadius = 30;      // Radio máximo de la explosión
    explosion.isActive = true;     // Activar la explosión
}

// Función para dibujar una explosión con degradado de tres colores
void drawExplosion(SDL_Renderer* renderer, Explosion& explosion) {
    if (explosion.isActive) {
        // Incrementar el tamaño de los círculos
        if (explosion.radiusOrange < explosion.maxRadius) {
            explosion.radiusOrange += 2;  // Círculo naranja crece más rápido
        }
        if (explosion.radiusOrange > 5 && explosion.radiusYellow < explosion.maxRadius) {
            explosion.radiusYellow += 2;  // Círculo amarillo comienza un poco después
        }
        if (explosion.radiusYellow > 10 && explosion.radiusWhite < explosion.maxRadius) {
            explosion.radiusWhite += 2;  // Círculo blanco comienza después del amarillo
        }

        // Dibujar el círculo naranja (capa más externa)
        SDL_SetRenderDrawColor(renderer, 255, 69, 0, 255);  // Naranja
        drawCircle(renderer, explosion.x, explosion.y, explosion.radiusOrange, {255, 69, 0, 255});

        // Dibujar el círculo amarillo (capa intermedia)
        if (explosion.radiusYellow > 0) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);  // Amarillo
            drawCircle(renderer, explosion.x, explosion.y, explosion.radiusYellow, {255, 255, 0, 255});
        }

        // Dibujar el círculo blanco (capa más interna)
        if (explosion.radiusWhite > 0) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);  // Blanco
            drawCircle(renderer, explosion.x, explosion.y, explosion.radiusWhite, {255, 255, 255, 255});
        }

        // Desactivar la explosión cuando todos los radios alcanzan su tamaño máximo
        if (explosion.radiusWhite >= explosion.maxRadius) {
            explosion.isActive = false;
        }
    }
}

void updateAndDrawPlanets(SDL_Renderer* renderer, std::vector<Planet>& planets, Uint32 currentTime){
    for (size_t i = 0; i < planets.size(); i++) {
        updateAndDrawPlanet(renderer, planets[i], currentTime);
    }
}

void updateAndDrawAsteroids(SDL_Renderer* renderer, std::vector<Asteroid>& asteroids){
    for (size_t i = 0; i < asteroids.size(); i++){
        updateAndDrawAsteroid(renderer, asteroids[i]);
    }    
}

// Función para verificar colisiones entre un asteroide y un planeta
bool checkCollision(Planet& planet, Asteroid& asteroid, Explosion& explosion) {
    if (!planet.isVisible) return false;

    int planetX = planet.centerX + planet.orbitRadius * cos(planet.angle);
    int planetY = planet.centerY + planet.orbitRadius * sin(planet.angle);

    int deltaX = asteroid.x - planetX;
    int deltaY = asteroid.y - planetY;
    int distance = sqrt(deltaX * deltaX + deltaY * deltaY);

    if (distance < (planet.size + asteroid.size)) {
        startExplosion(explosion, planetX, planetY);  // Color rojo para la explosión
        planet.isVisible = false;
        planet.respawnTime = SDL_GetTicks() + 5000;  // El planeta reaparece en 5 segundos
        return true;
    }
    return false;
}

void drawAllExplosions(SDL_Renderer* renderer, std::vector<Explosion>& explosions){
    for (size_t i = 0; i < explosions.size(); i++){
        drawExplosion(renderer, explosions[i]);
    }
}

void checkAllCollisions(std::vector<Planet>& planets, std::vector<Asteroid>& asteroids, std::vector<Explosion>& explosions, Uint32 currentTime){
    #pragma omp parallel for    
    for (size_t i = 0; i < planets.size(); i++) {
        for (auto& asteroid : asteroids) {
            if (checkCollision(planets[i], asteroid, explosions[i])) {
                planets[i].isVisible = false;
                planets[i].respawnTime = currentTime + 5000; // Desaparece durante 5 segundos
            }
        }
    }
}

// Función para verificar si una cadena es un número
bool isNumber(const std::string& str) {
    // Verifica que no esté vacía y que todos los caracteres sean dígitos
    if (str.empty()) return false;
    for (char const &c : str) {
        if (!std::isdigit(c)) return false;
    }
    return true;
}

int main(int argc, char* argv[]) {

        // Comprobación de parámetros
    if (argc != 5) {  // Verificar si el número de parámetros es exactamente 4
        std::cerr << "Error: Se requieren exactamente 4 parámetros: <Número de nebulosas> <Número de estrellas> <Número de planetas> <Número de asteroides>" << std::endl;
        return 1;
    }

    // Validar que cada parámetro sea un número
    for (int i = 1; i < argc; i++) {
        if (!isNumber(argv[i])) {
            std::cerr << "Error: El parámetro " << i << " ('" << argv[i] << "') no es un número válido." << std::endl;
            return 1;
        }
    }

    // Convertir los argumentos a enteros
    int numNebulaPoints = std::stoi(argv[1]);
    int numStars = std::stoi(argv[2]);
    int numPlanets = std::stoi(argv[3]);
    int numAsteroids = std::stoi(argv[4]);

    // Inicialización de SDL
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Space Screensaver",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          640, 480, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Inicializar nebulosa con 300 puntos
    initNebula(numNebulaPoints);
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
    for (int i = 0; i < numPlanets; i++) {
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
    for (int i = 0; i < numAsteroids; i++) {
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
    for (int i = 0; i < numStars; i++) {
        Star star;
        star.x = rand() % 640;
        star.y = rand() % 480;
        star.size = 1 + rand() % 2;
        star.speed = 0.1f + (rand() % 10) * 0.01f; // Velocidad reducida
        stars.push_back(star);
    }

    std::vector<Explosion> explosions(planets.size());

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
        
        // Dibujar Nebulosa
        updateAndDrawNebula(renderer);
        
        // Dibujar estrellas
        updateAndDrawStars(renderer, stars);

        // Dibujar agujero negro en el centro de la pantalla
        drawBlackHole(renderer, 320, 240);

        // Actualizar y dibujar planetas
        updateAndDrawPlanets(renderer, planets, currentTime);
        drawAllExplosions(renderer, explosions);

        // Actualizar y dibujar asteroides
        updateAndDrawAsteroids(renderer, asteroids);

        // Verificar colisiones y gestionar la desaparición de planetas
        checkAllCollisions(planets, asteroids, explosions, currentTime);

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
