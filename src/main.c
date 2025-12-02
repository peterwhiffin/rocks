#include <SDL3/SDL_events.h>
#include <time.h>
#include <math.h>

#include "SDL3/SDL_video.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_keyboard.h"
#include "SDL3/SDL_scancode.h"
#include "SDL3/SDL_timer.h"

#include "glad.c"

#define SHADER_PATH "../../../src/shaders/"

#define TOP_LEFT0   -1.0f, 1.0f
#define TOP_MIDDLE1 0.0f, 1.0f
#define TOP_RIGHT2  1.0f, 1.0f

#define MIDDLE_LEFT3   -1.0f, 0.0f
#define MIDDLE_MIDDLE4 0.0f, 0.0f
#define MIDDLE_RIGHT5  1.0f, 0.0f

#define BOTTOM_LEFT6   -1.0f, -1.0f
#define BOTTOM_MIDDLE7 0.0f, -1.0f
#define BOTTOM_RIGHT8  1.0f, -1.0f

#define A 9
#define B 10
#define C 11
#define D 12
#define E 13
#define F 14
#define G 15
#define H 16
#define I 17
#define J 18
#define K 19
#define L 20
#define M 21
#define N 22
#define O 23
#define P 24
#define Q 25
#define R 26
#define S 27
#define T 28
#define U 29
#define V 30
#define W 31
#define X 32
#define Y 33
#define Z 34

float textVerts[] = {
    // clang-format off
	TOP_LEFT0, TOP_MIDDLE1, TOP_RIGHT2, 
	MIDDLE_LEFT3, MIDDLE_MIDDLE4, MIDDLE_RIGHT5, 
	BOTTOM_LEFT6, BOTTOM_MIDDLE7, BOTTOM_RIGHT8
    // clang-format on
};

typedef struct {
    unsigned int* data;
    size_t        size;
} TextIndices;

unsigned int zeroInd[8] = {
    // clang-format off
	0, 2,
	2, 8,
	8, 6,
	6, 0
    // clang-format on
};

unsigned int oneInd[2] = {
    // clang-format off
	1, 7
    // clang-format on
};

unsigned int twoInd[10] = {
    // clang-format off
	0, 2,
	2, 5,
	5, 3,
	3, 6,
	6, 8
    // clang-format on
};

unsigned int threeInd[8] = {
    // clang-format off
	0, 2,
	2, 8,
	8, 6,
	5, 3
    // clang-format on
};

unsigned int fourInd[6] = {
    // clang-format off
	0, 3,
	3, 5,
	2, 8
    // clang-format on
};

unsigned int fiveInd[10] = {
    // clang-format off
	2, 0,
	0, 3,
	3, 5,
	5, 8,
	8, 6
    // clang-format on
};

unsigned int sixInd[10] = {
    // clang-format off
	2, 0,
	0, 6,
	6, 8,
	8, 5,
	5, 3
    // clang-format on
};

unsigned int sevenInd[4] = {
    // clang-format off
	0, 2,
	2, 8
    // clang-format on
};

unsigned int eightInd[10] = {
    // clang-format off
	0, 2,
	2, 8,
	8, 6,
	6, 0,
	3, 5
    // clang-format on
};

unsigned int nineInd[8] = {
    // clang-format off
	0, 2,
	2, 8,
	5, 3,
	3, 0
    // clang-format on
};

TextIndices textIndices[10] = {
    // clang-format off
	{zeroInd, sizeof(zeroInd)},
	{oneInd, sizeof(oneInd)},
	{twoInd, sizeof(twoInd)},
	{threeInd, sizeof(threeInd)},
	{fourInd, sizeof(fourInd)},
	{fiveInd, sizeof(fiveInd)},
	{sixInd, sizeof(sixInd)},
	{sevenInd, sizeof(sevenInd)},
	{eightInd, sizeof(eightInd)},
	{nineInd, sizeof(nineInd)}
    // clang-format on
};

float shipVerts[8] = {
    // clang-format off
		0.0f, 0.9f,
		-0.6f, -1.0f,
		0.0f, -0.5f,
		0.6f, -0.9f,
    // clang-format on
};

float quadVerts[8] = {
    // clang-format off
		-1.0f, -1.0f,
		1.0f, -1.0f,
		-1.0f, 1.0f,
		1.0f, 1.0f,
    // clang-format on
};

unsigned int shipIndices[8] = {0, 1, 1, 2, 2, 3, 3, 0};
unsigned int quadIndices[6] = {0, 1, 2, 2, 3, 1};
unsigned int bulletIndices[1] = {0};
float        bulletVerts[2] = {0.0f, 0.0f};

typedef struct {
    float x;
    float y;
} vec2;

typedef struct {
    float x;
    float y;
    float z;
} vec3;

typedef struct {
    GLintptr indOffset;
    GLuint   indCount;
    GLuint   vao;
    GLuint   buf;
    GLenum   drawMode;
    vec3     color;
    vec3     nextColor;
    float    colorTimer;
    float    colorDuration;
    float    maxRadius;
} DrawInfo;

typedef struct {
    SDL_Window*   ptr;
    SDL_GLContext glContext;
    vec2          res;
    vec2          screenSize;
    bool          shouldClose;
} Window;

typedef struct {
    vec2        move;
    bool        shoot;
    const bool* keyStates;
} Input;

typedef struct {
    DrawInfo asteroidPolys[16];
    DrawInfo bullet;
    DrawInfo quad;
    DrawInfo ship;
    DrawInfo text[34];
    DrawInfo textVert;
    vec2     lifePos;
    vec2     scorePos;
    float    lifeScale;
    float    scoreScale;
    float    lifePadding;
    float    scorePadding;
    GLuint   shader;
} Renderer;

typedef struct {
    DrawInfo*    poly;
    vec2         pos;
    vec2         vel;
    float        angle;
    float        scale;
    unsigned int level;
} Asteroid;

typedef struct {
    vec2         pos;
    vec2         vel;
    float        accel;
    float        angle;
    float        scale;
    float        turnRate;
    float        maxVel;
    unsigned int lives;
    unsigned int score;
    DrawInfo*    poly;
    bool         canShoot;
} Ship;

typedef struct {
    vec2  vel;
    vec2  pos;
    float lifetime;
} Bullet;

typedef struct {
    Asteroid     asteroids[128];
    Bullet       bullets[128];
    Ship         ship;
    float        time;
    float        deltaTime;
    float        bulletLifetime;
    float        bulletSpeed;
    float        levelTimer;
    float        levelLoadTime;
    unsigned int numAsteroids;
    unsigned int numBullets;
    unsigned int level;
    bool         newLevel;
} Scene;

float lerp(float a, float b, float t) { return a + (b - a) * t; }
float randomFloatNormal() { return rand() / (float)RAND_MAX; }
float randomRange(float min, float max) { return lerp(min, max, randomFloatNormal()); }
float vec2_magnitude(vec2 v) { return sqrt(v.x * v.x + v.y * v.y); }
float vec3_magnitude(vec3 v) { return sqrt(v.x * v.x + v.y * v.y + v.z * v.z); }

vec3 vec3_lerp(vec3 a, vec3 b, float t) {
    if (t > 1) {
        t = 1;
    } else if (t < 0) {
        t = 0;
    }

    return (vec3){lerp(a.x, b.x, t), lerp(a.y, b.y, t), lerp(a.z, b.z, t)};
}

vec2 normalize(vec2 v) {
    float mag = vec2_magnitude(v);
    v.x = v.x / mag;
    v.y = v.y / mag;
    return v;
}

vec3 randomColor() { return (vec3){randomRange(0.1f, 1.0f), randomRange(0.1f, 1.0f), randomRange(0.1f, 1.0f)}; }

void initWindow(Window* w, Input* i) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("failed to init sdl");
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);

    SDL_WindowFlags flags = SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE;
    SDL_Window*     ptr = SDL_CreateWindow("petes practice", 800, 800, flags);
    w->glContext = SDL_GL_CreateContext(ptr);

    SDL_GL_MakeCurrent(ptr, w->glContext);
    SDL_GL_SetSwapInterval(1);
    SDL_SetWindowPosition(ptr, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(ptr);

    i->keyStates = SDL_GetKeyboardState(NULL);
    w->ptr = ptr;
    w->res = (vec2){800, 800};
    w->shouldClose = false;
}

// should probably check for errors here
char* readFile(const char* path) {
    char* buf;
    long  len;
    FILE* f = fopen(path, "r");

    fseek(f, 0, SEEK_END);
    len = ftell(f);
    fseek(f, 0, SEEK_SET);
    buf = malloc(len + 1);
    fread(buf, sizeof(char), len, f);
    buf[len] = '\0';
    fclose(f);

    return buf;
}

void checkShader(GLuint shader) {
    GLint success;

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success) {
        char log[1024];
        glGetShaderInfoLog(shader, 1024, NULL, log);
        printf("Shader compilation error: %s", log);
    }
}

void checkProgram(GLuint program) {
    GLint success;

    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success) {
        char log[1024];
        glGetShaderInfoLog(program, 1024, NULL, log);
        printf("Shader link error: %s", log);
    }
}

GLuint loadShader(const char* vertFile, const char* fragFile) {
    GLuint vertShader;
    GLuint fragShader;
    GLuint program;

    char vertPath[512];
    char fragPath[512];

    snprintf(vertPath, sizeof(vertPath), "%s%s", SHADER_PATH, vertFile);
    snprintf(fragPath, sizeof(fragPath), "%s%s", SHADER_PATH, fragFile);

    const char* vertBuf = readFile(vertPath);
    const char* fragBuf = readFile(fragPath);

    vertShader = glCreateShader(GL_VERTEX_SHADER);
    fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    program = glCreateProgram();

    glShaderSource(vertShader, 1, &vertBuf, NULL);
    glShaderSource(fragShader, 1, &fragBuf, NULL);

    glCompileShader(vertShader);
    glCompileShader(fragShader);
    checkShader(vertShader);
    checkShader(fragShader);

    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);
    checkProgram(program);

    free((void*)vertBuf);
    free((void*)fragBuf);
    return program;
}

void createVAO(Renderer* r, DrawInfo* d, float* verts, unsigned int* indices, size_t vertSize, size_t indSize) {
    GLsizeiptr     vertLen = vertSize;
    GLsizeiptr     indLen = indSize;
    GLintptr       indOffset = vertLen;
    unsigned char* data = malloc(vertLen + indLen);

    memcpy(data, verts, vertLen);
    memcpy(data + indOffset, indices, indLen);

    GLuint vao;
    GLuint buf;

    glCreateVertexArrays(1, &vao);
    glCreateBuffers(1, &buf);

    glNamedBufferStorage(buf, vertLen + indLen, data, 0);

    glVertexArrayVertexBuffer(vao, 0, buf, 0, 2 * sizeof(float));
    glVertexArrayElementBuffer(vao, buf);

    glEnableVertexArrayAttrib(vao, 0);
    glVertexArrayAttribFormat(vao, 0, 2, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(vao, 0, 0);

    d->vao = vao;
    d->buf = buf;
    d->indOffset = indOffset;
    d->indCount = indSize / sizeof(unsigned int);
    free(data);
}

void createText(Renderer* r) {
    size_t sizeOffset = sizeof(textVerts);
    size_t countOffset = 0;
    size_t indSize = 0;

    for (int i = 0; i < 10; i++) {
        indSize += textIndices[i].size;
    }

    unsigned int* indices = malloc(indSize);

    for (int i = 0; i < 10; i++) {
        DrawInfo* d = &r->text[i];
        size_t    size = textIndices[i].size;
        size_t    count = size / sizeof(unsigned int);

        d->indOffset = sizeOffset;
        d->indCount = count;

        memcpy(indices + countOffset, textIndices[i].data, size);

        sizeOffset += size;
        countOffset += count;
    }

    createVAO(r, &r->textVert, textVerts, indices, sizeof(textVerts), indSize);

    for (int i = 0; i < 10; i++) {
        r->text[i].vao = r->textVert.vao;
        r->text[i].buf = r->textVert.buf;
    }

    free(indices);
}

void createRandomPolygon(Renderer* r, DrawInfo* p) {
    float        x, y, radius, angle;
    unsigned int ind1, ind2;
    unsigned int numSides = (int)floor(randomRange(5.0f, 10.0f));
    float        polyVerts[512];
    unsigned int polyIndices[512];
    float        angleStep = (2 * M_PI) / numSides;

    for (int i = 0; i < numSides; i++) {
        radius = randomRange(0.1, 1.0f);
        angle = angleStep * i;
        x = radius * cos(angle);
        y = radius * sin(angle);
        ind1 = i + i;
        ind2 = i + i + 1;
        polyVerts[ind1] = x;
        polyVerts[ind2] = y;
        polyIndices[ind1] = i;
        polyIndices[ind2] = i + 1;

        if (radius > p->maxRadius) {
            p->maxRadius = radius;
        }
    }

    polyIndices[(numSides * 2) - 1] = 0;
    createVAO(r, p, polyVerts, polyIndices, sizeof(polyVerts[0]) * numSides * 2, sizeof(polyIndices[0]) * numSides * 2);
}

void initRenderer(Renderer* r) {
    gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);

    for (int i = 0; i < sizeof(r->asteroidPolys) / sizeof(DrawInfo); i++) {
        createRandomPolygon(r, &r->asteroidPolys[i]);
    }

    createText(r);
    createVAO(r, &r->ship, shipVerts, shipIndices, sizeof(shipVerts), sizeof(shipIndices));
    createVAO(r, &r->bullet, bulletVerts, bulletIndices, sizeof(bulletVerts), sizeof(bulletIndices));
    createVAO(r, &r->quad, quadVerts, quadIndices, sizeof(quadVerts), sizeof(quadIndices));

    r->quad.drawMode = GL_TRIANGLE_STRIP;
    r->bullet.drawMode = GL_POINTS;
    r->ship.drawMode = GL_LINES;
    r->textVert.color = randomColor();
    r->textVert.nextColor = randomColor();
    r->textVert.colorTimer = 0.0f;
    r->textVert.colorDuration = 1.5f;
    r->shader = loadShader("shader.vert", "shader.frag");
    r->lifePos = (vec2){-0.9, 0.8f};
    r->scorePos = (vec2){-0.9, 0.9f};
    r->lifeScale = 0.032f;
    r->scoreScale = 0.03f;
    r->lifePadding = 0.055f;
    r->scorePadding = 0.07f;

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_PROGRAM_POINT_SIZE);
}

void initScene(Scene* s, Renderer* r) {
    s->deltaTime = 0.0f;
    s->numAsteroids = 0;
    s->numBullets = 0;
    s->time = 0.0f;
    s->level = 1;
    s->levelLoadTime = 3.0f;
    s->levelTimer = 0.0f;
    s->ship.pos = (vec2){0.0f, 0.0f};
    s->ship.scale = 0.05f;
    s->ship.maxVel = 0.3f;
    s->ship.turnRate = 4.0f;
    s->ship.accel = 0.007f;
    s->ship.canShoot = true;
    s->ship.lives = 4;
    s->ship.score = 0;
    s->bulletSpeed = 2.0f;
    s->bulletLifetime = 1.0f;
    s->ship.poly = &r->ship;
    s->ship.poly->color = randomColor();
    s->ship.poly->nextColor = randomColor();
    s->ship.poly->colorDuration = 1.5f;
    s->ship.poly->colorTimer = 0.0f;
}

void windowResized(Window* w, vec2 res) {
    w->screenSize = res;

    if (res.x > res.y) {
        res.x = res.y;
    } else {
        res.y = res.x;
    }

    w->res = res;
    glViewport((w->screenSize.x - res.x) * 0.5f, (w->screenSize.y - res.y) * 0.5f, res.x, res.y);
}

void pollEvents(Window* w) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                w->shouldClose = true;
                break;
            case SDL_EVENT_WINDOW_RESIZED:
                windowResized(w, (vec2){event.window.data1, event.window.data2});
                break;
        }
    }
}

void wrapScreen(vec2* pos) {
    if (pos->x > 1 || pos->x < -1) {
        pos->x *= -1;
    }

    if (pos->y > 1 || pos->y < -1) {
        pos->y *= -1;
    }
}

void addBullet(Scene* s, Renderer* r, vec2 pos, vec2 vel) {
    Bullet* b = &s->bullets[s->numBullets];
    b->pos = pos;
    b->vel = vel;
    b->lifetime = s->bulletLifetime;
    s->numBullets++;
}

void removeAsteroid(Scene* s, Asteroid* a) {
    if (a != &s->asteroids[s->numAsteroids - 1]) {
        Asteroid* lastA = &s->asteroids[s->numAsteroids - 1];
        a->pos = lastA->pos;
        a->angle = lastA->angle;
        a->level = lastA->level;
        a->scale = lastA->scale;
        a->vel = lastA->vel;
        a->poly = lastA->poly;
    }
    s->numAsteroids--;
}

Asteroid* getNewAsteroid(Scene* s) {
    Asteroid* a = &s->asteroids[s->numAsteroids];
    s->numAsteroids++;
    return a;
}

void loadLevel(Scene* s, Renderer* r) {
    for (int i = 0; i < 4 + s->level; i++) {
        Asteroid* ast = getNewAsteroid(s);
        ast->poly = &r->asteroidPolys[(int)floor(randomRange(0, 16))];
        ast->vel = (vec2){randomRange(-0.3f, 0.3f), randomRange(-0.3f, 0.3f)};
        ast->scale = 0.3f;
        ast->pos = (vec2){randomRange(-1.0f, 1.0f), randomRange(-1.0f, 1.0f)};
        ast->level = 3;
        ast->poly->color = randomColor();
        ast->poly->nextColor = randomColor();
        ast->poly->colorDuration = 1.5f;
        ast->poly->colorTimer = 0.0f;
    }

    s->newLevel = false;
    s->levelTimer = 0.0f;
}

void hitAsteroid(Scene* s, Asteroid* a, Renderer* r) {
    a->level -= 1;
    s->ship.score += 100;

    if (a->level != 0) {
        int   mod = 1;
        float scale = a->level == 2 ? 0.1f : 0.05f;

        for (int i = 0; i < 2; i++) {
            Asteroid* child = getNewAsteroid(s);
            child->poly = &r->asteroidPolys[(int)floor(randomRange(0, 16))];
            child->pos = a->pos;
            child->poly->color = randomColor();
            child->poly->nextColor = randomColor();
            child->poly->colorTimer = 0.0f;
            child->poly->colorDuration = 1.5f;
            child->vel = (vec2){a->vel.x * mod, a->vel.y * mod};
            child->scale = scale;
            child->angle = a->angle * mod;
            child->level = a->level;
            mod = -1;
        }
    }

    removeAsteroid(s, a);

    if (s->numAsteroids == 0) {
        s->newLevel = true;
    }
}

void removeBullet(Scene* s, Bullet* b) {
    size_t index = s->numBullets - 1;

    if (b != &s->bullets[index]) {
        *b = s->bullets[index];
    }

    s->numBullets--;
}

void checkCollisions(Scene* s, Renderer* r) {
    for (int i = s->numBullets - 1; i >= 0; i--) {
        Bullet* b = &s->bullets[i];

        for (int k = s->numAsteroids - 1; k >= 0; k--) {
            Asteroid* a = &s->asteroids[k];
            vec2      v = (vec2){a->pos.x - b->pos.x, a->pos.y - b->pos.y};

            if (vec2_magnitude(v) < (a->poly->maxRadius * a->scale)) {
                hitAsteroid(s, a, r);
                removeBullet(s, b);
                break;
            }
        }
    }
}

void lerpColor(DrawInfo* poly, float dt) {
    poly->colorTimer += dt;
    poly->color = vec3_lerp(poly->color, poly->nextColor, poly->colorTimer / poly->colorDuration);

    if (poly->colorTimer >= poly->colorDuration) {
        poly->colorTimer = 0.0f;
        poly->nextColor = randomColor();
    }
}

void updateScene(Scene* s, Renderer* r, Input* input) {
    float dt = s->deltaTime;
    if (s->newLevel) {
        if (s->levelTimer >= s->levelLoadTime) {
            s->level++;
            loadLevel(s, r);
        }

        s->levelTimer += dt;
    }

    vec2  move = input->move;
    vec2  fwd = {cos(s->ship.angle + M_PI * 0.5f), sin(s->ship.angle + M_PI * 0.5f)};
    vec2  moveDir = {move.y * fwd.x, move.y * fwd.y};
    vec2  vel = {s->ship.vel.x + moveDir.x * s->ship.accel, s->ship.vel.y + moveDir.y * s->ship.accel};
    float mag = vec2_magnitude(vel);
    vec2  dir = normalize(vel);

    if (mag > s->ship.maxVel) {
        vel.x = dir.x * s->ship.maxVel;
        vel.y = dir.y * s->ship.maxVel;
    }

    vel.x = lerp(vel.x, 0.0f, 0.4f * dt);
    vel.y = lerp(vel.y, 0.0f, 0.4f * dt);

    s->ship.vel = vel;
    s->ship.pos.x += vel.x * dt;
    s->ship.pos.y += vel.y * dt;
    s->ship.angle += move.x * s->ship.turnRate * dt;
    wrapScreen(&s->ship.pos);
    lerpColor(s->ship.poly, dt);

    if (input->shoot) {
        if (s->ship.canShoot) {
            s->ship.canShoot = false;
            vec2 pos = {s->ship.pos.x + fwd.x * 0.1f, s->ship.pos.y + fwd.y * 0.1f};
            vec2 vel = {fwd.x * s->bulletSpeed, fwd.y * s->bulletSpeed};
            addBullet(s, r, pos, vel);
        }
    } else {
        s->ship.canShoot = true;
    }

    for (int i = 0; i < s->numAsteroids; i++) {
        Asteroid* ast = &s->asteroids[i];
        ast->pos.x += ast->vel.x * dt;
        ast->pos.y += ast->vel.y * dt;
        wrapScreen(&ast->pos);
        lerpColor(ast->poly, dt);
    }

    for (int i = s->numBullets - 1; i >= 0; i--) {
        Bullet* b = &s->bullets[i];
        b->pos = (vec2){b->pos.x + b->vel.x * dt, b->pos.y + b->vel.y * dt};
        b->lifetime -= dt;
        lerpColor(&r->bullet, dt);

        if (b->lifetime <= 0) {
            removeBullet(s, b);
        } else {
            wrapScreen(&b->pos);
        }
    }

    lerpColor(&r->textVert, dt);
    checkCollisions(s, r);
}

void updateTime(Scene* s) {
    float currentTime = (float)SDL_GetTicks() * .001;
    s->deltaTime = currentTime - s->time;
    s->time = currentTime;
}

void updateInput(Input* i) {
    i->move.x = 0;
    i->move.y = 0;
    i->move.x += i->keyStates[SDL_SCANCODE_RIGHT] ? -1 : 0;
    i->move.x += i->keyStates[SDL_SCANCODE_LEFT] ? 1 : 0;
    i->move.y += i->keyStates[SDL_SCANCODE_UP] ? 1 : 0;
    i->move.y += i->keyStates[SDL_SCANCODE_DOWN] ? 0 : 0;
    i->shoot = i->keyStates[SDL_SCANCODE_SPACE];
}

void drawScene(Scene* s, Renderer* r) {
    Ship*     ship = &s->ship;
    DrawInfo* ren = ship->poly;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(r->shader);
    glUniform3fv(3, 1, &ren->color.x);
    glUniform1fv(7, 1, &s->time);

    glUniform2fv(1, 1, &ship->pos.x);
    glUniform1f(2, ship->scale);
    glUniform1f(4, ship->angle);
    glBindVertexArray(ren->vao);
    glDrawElements(GL_LINES, ren->indCount, GL_UNSIGNED_INT, (void*)ren->indOffset);

    vec2 pos = r->lifePos;
    glUniform1f(4, 0.0f);
    glUniform1f(2, s->ship.scale - 0.018f);

    for (int i = 0; i < s->ship.lives; i++) {
        glUniform2fv(1, 1, &pos.x);
        glDrawElements(GL_LINES, ren->indCount, GL_UNSIGNED_INT, (void*)ren->indOffset);
        pos.x += r->lifePadding;
    }

    glUniform1f(4, 0.0f);
    glUniform1f(5, 2.2f);

    for (int i = 0; i < s->numAsteroids; i++) {
        Asteroid* a = &s->asteroids[i];
        ren = a->poly;
        glUniform2fv(1, 1, &a->pos.x);
        glUniform1f(2, a->scale);
        glUniform3fv(3, 1, &ren->color.x);
        glBindVertexArray(ren->vao);
        glDrawElements(GL_LINES, ren->indCount, GL_UNSIGNED_INT, (void*)ren->indOffset);
        glDrawArrays(GL_POINTS, 0, ren->indCount * 0.5f);
    }

    for (int i = 0; i < s->numBullets; i++) {
        glBindVertexArray(r->bullet.vao);
        glUniform2fv(1, 1, &s->bullets[i].pos.x);
        glUniform1f(2, 1.0f);
        glUniform3fv(3, 1, &r->bullet.color.x);
        glUniform1f(4, 0.0f);
        glUniform1f(5, 2.0f);
        glDrawArrays(GL_POINTS, 0, 1);
    }

    int base = s->ship.score;
    int count = 0;
    int digits[8];
    digits[0] = 0;

    do {
        digits[count] = base % 10;
        // base /= 10;
        count++;
    } while (base /= 10);

    glBindVertexArray(r->textVert.vao);
    glUniform1f(2, 0.03);
    pos = r->scorePos;

    for (int i = count - 1; i >= 0; i--) {
        glUniform2fv(1, 1, &pos.x);
        glUniform3fv(3, 1, &r->textVert.color.x);
        glDrawElements(GL_LINES, r->text[digits[i]].indCount, GL_UNSIGNED_INT, (void*)r->text[digits[i]].indOffset);
        pos.x += r->scorePadding;
    }
}

int main() {
    Window*   window = malloc(sizeof(Window));
    Renderer* renderer = malloc(sizeof(Renderer));
    Input*    input = malloc(sizeof(Input));
    Scene*    scene = malloc(sizeof(Scene));

    srand(time(NULL));
    initWindow(window, input);
    initRenderer(renderer);
    initScene(scene, renderer);
    loadLevel(scene, renderer);

    while (!window->shouldClose) {
        updateTime(scene);
        pollEvents(window);
        updateInput(input);
        updateScene(scene, renderer, input);
        drawScene(scene, renderer);
        SDL_GL_SwapWindow(window->ptr);
    }

    return 0;
}
