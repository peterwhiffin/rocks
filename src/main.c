#include <stdalign.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#include "SDL3/SDL_video.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_keyboard.h"
#include "SDL3/SDL_scancode.h"
#include "SDL3/SDL_timer.h"

#include "AL/al.h"
#include "AL/alc.h"
#include "AL/alext.h"

#include "glad.c"
#include "alhelpers.c"
#include "soundtest.c"

#define SHADER_PATH "../../../src/shaders/"

#define U_POS       2
#define U_SCALE     3
#define U_ANGLE     4
#define U_POINTSIZE 5
#define U_COLOR     8
#define U_TIME      9

#define TOP_LEFT0   -1.0f, 1.0f
#define TOP_MIDDLE1 0.0f, 1.0f
#define TOP_RIGHT2  1.0f, 1.0f

#define MIDDLE_LEFT3   -1.0f, 0.0f
#define MIDDLE_MIDDLE4 0.0f, 0.0f
#define MIDDLE_RIGHT5  1.0f, 0.0f

#define BOTTOM_LEFT6   -1.0f, -1.0f
#define BOTTOM_MIDDLE7 0.0f, -1.0f
#define BOTTOM_RIGHT8  1.0f, -1.0f

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

float quadVerts[16] = {
    // clang-format off
		-1.0f, -1.0f, 0.0f, 0.0f,
		1.0f, -1.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f
    // clang-format on
};

float buttonVerts[16] = {
    // clang-format off
		-1.0f, -1.0f, 0.0f, 0.0f,
		1.0f, -1.0f, 1.0f, 0.0f,
		-1.0f, 0.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 1.0f
    // clang-format on
};

unsigned int shipIndices[8] = {0, 1, 1, 2, 2, 3, 3, 0};
unsigned int quadIndices[6] = {0, 1, 2, 2, 3, 1};
unsigned int boxIndices[8] = {0, 1, 1, 3, 3, 2, 2, 0};
unsigned int bulletIndices[1] = {0};
float        bulletVerts[2] = {0.0f, 0.0f};

typedef enum {
    ALIVE = 1 << 0,
    CAN_SHOOT = 1 << 1,
    NEW_LEVEL = 1 << 2,
    GAME_OVER = 1 << 3,
} Flags;

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
    float x;
    float y;
    float z;
    float w;
} vec4;

typedef struct {
    GLintptr indOffset;
    GLuint   indCount;
    GLuint   vao;
    GLuint   buf;
    GLenum   drawMode;
    vec3     color;
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
    vec2        cursorPosition;
    bool        shoot;
    bool        mouse0;
    const bool* keyStates;
} Input;

typedef struct {
    DrawInfo asteroidDrawInfos[16];
    DrawInfo bulletDrawInfo;
    DrawInfo quadDrawInfo;
    DrawInfo boxDrawInfo;
    DrawInfo buttonDrawInfo;
    DrawInfo shipDrawInfo;
    DrawInfo text[34];
    DrawInfo textVert;
    vec2     lifePos;
    vec2     scorePos;
    vec4     clearColor;
    float    clearDepth;
    float    lifeScale;
    float    scoreScale;
    float    lifePadding;
    float    scorePadding;
    GLuint   shader;
    GLuint   fullScreenShader;
    GLuint   fbo;
    GLuint   renderBuffer;
    GLuint   renderTarget;
} Renderer;

typedef struct {
    vec2  pos;
    vec2  vel;
    float angle;
    float scale;
} Transform;

typedef struct {
    Transform    transform;
    DrawInfo     drawInfo;
    unsigned int level;
    unsigned int pad;
} Asteroid;

typedef struct {
    Transform transform;
    DrawInfo  drawInfo;
    float     lifetime;
    bool      hasWrapped;
} Bullet;

typedef struct {
    Transform transform;
    DrawInfo  drawInfo;
    float     accel;
    float     turnRate;
    float     maxVel;
} Ship;

typedef struct {
    Transform transform;
    DrawInfo  drawInfo;
} Button;

typedef struct {
    Asteroid      asteroids[128];
    Bullet        bullets[128];
    Ship          ship;
    float         time;
    float         deltaTime;
    float         deathTimer;
    float         deathTime;
    float         levelTimer;
    float         levelLoadTime;
    float         bulletLifetime;
    float         bulletSpeed;
    float         shootSoundTimer;
    float         shootSoundTime;
    ALuint        shootSoundSource;
    unsigned int  numAsteroids;
    unsigned int  numBullets;
    unsigned int  level;
    unsigned int  score;
    unsigned int  lives;
    unsigned char flags;
    bool          shootSoundPlaying;
} Scene;

// float lerp(float a, float b, float t) { return a + (b - a) * t; }
// float randomFloatNormal() { return rand() / (float)RAND_MAX; }
// float randomRangeF(float min, float max) { return lerp(min, max, randomFloatNormal()); }
int   randomRangeI(int min, int max) { return (int)floor(randomRangeF(min, max)); }
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

vec3 randomColor() { return (vec3){randomRangeF(0.1f, 1.0f), randomRangeF(0.1f, 1.0f), randomRangeF(0.1f, 1.0f)}; }

void setFlag(Scene* s, Flags f) { s->flags |= f; }
void unsetFlag(Scene* s, Flags f) { s->flags &= ~f; }

void initWindow(Window* w, Input* i) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("failed to init sdl\n");
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);

    SDL_WindowFlags flags = SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE;
    // SDL_WindowFlags flags = SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN;
    SDL_Window* ptr = SDL_CreateWindow("petes practice", 800, 800, flags);
    w->glContext = SDL_GL_CreateContext(ptr);

    SDL_GL_MakeCurrent(ptr, w->glContext);
    SDL_GL_SetSwapInterval(1);
    SDL_SetWindowPosition(ptr, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(ptr);

    i->keyStates = SDL_GetKeyboardState(NULL);
    w->ptr = ptr;
    w->res = (vec2){800.0f, 800.0f};
    w->screenSize = (vec2){800.0f, 800.0f};
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
        printf("Shader compilation error: %s\n", log);
    }
}

void checkProgram(GLuint program) {
    GLint success;

    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success) {
        char log[1024];
        glGetShaderInfoLog(program, 1024, NULL, log);
        printf("Shader link error: %s\n", log);
    }
}

void loadShader(GLuint* shader, const char* vertFile, const char* fragFile) {
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
    *shader = program;
}

void createVAO(Renderer* r, DrawInfo* d, float* verts, unsigned int* indices, size_t vertSize, size_t indSize, bool hasTexCoords) {
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

    GLsizei size = hasTexCoords ? 4 * sizeof(float) : 2 * sizeof(float);

    glVertexArrayVertexBuffer(vao, 0, buf, 0, size);
    glVertexArrayElementBuffer(vao, buf);

    glEnableVertexArrayAttrib(vao, 0);
    glVertexArrayAttribFormat(vao, 0, 2, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(vao, 0, 0);

    if (hasTexCoords) {
        glEnableVertexArrayAttrib(vao, 1);
        glVertexArrayAttribFormat(vao, 1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float));
        glVertexArrayAttribBinding(vao, 1, 0);
    }

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

    createVAO(r, &r->textVert, textVerts, indices, sizeof(textVerts), indSize, false);
    r->textVert.drawMode = GL_LINES;

    for (int i = 0; i < 10; i++) {
        r->text[i].vao = r->textVert.vao;
        r->text[i].buf = r->textVert.buf;
    }

    free(indices);
}

void createRandomPolygon(Renderer* r, DrawInfo* p) {
    float        x, y, radius, angle;
    unsigned int ind1, ind2;
    unsigned int numSides = (int)floor(randomRangeF(12.0f, 16.0f));
    float        polyVerts[512];
    unsigned int polyIndices[512];
    float        angleStep = (2 * M_PI) / numSides;

    for (int i = 0; i < numSides; i++) {
        radius = randomRangeF(0.7, 1.0f);
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
    createVAO(r, p, polyVerts, polyIndices, sizeof(polyVerts[0]) * numSides * 2, sizeof(polyIndices[0]) * numSides * 2, false);
}

void createFramebuffer(Renderer* r, Window* w) {
    glCreateTextures(GL_TEXTURE_2D, 1, &r->renderTarget);
    glTextureParameteri(r->renderTarget, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(r->renderTarget, GL_TEXTURE_WRAP_R, GL_REPEAT);
    glTextureParameteri(r->renderTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(r->renderTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureStorage2D(r->renderTarget, 1, GL_RGBA16F, w->res.x, w->res.y);

    glCreateRenderbuffers(1, &r->renderBuffer);
    glNamedRenderbufferStorage(r->renderBuffer, GL_DEPTH_COMPONENT, w->res.x, w->res.y);

    glCreateFramebuffers(1, &r->fbo);
    glNamedFramebufferTexture(r->fbo, GL_COLOR_ATTACHMENT0, r->renderTarget, 0);

    glNamedFramebufferDrawBuffer(r->fbo, GL_COLOR_ATTACHMENT0);
    glNamedFramebufferRenderbuffer(r->fbo, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, r->renderBuffer);

    if (glCheckNamedFramebufferStatus(r->fbo, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        printf("ERROR::FRAMEBUFFER::Framebuffer not complete\n");
    } else {
        printf("frame buffer created\n");
    }
}

void resizeFramebuffer(Renderer* r, Window* w) {
    printf("frame buffer DELETED\n");
    glDeleteTextures(1, &r->renderTarget);
    glDeleteRenderbuffers(1, &r->renderBuffer);
    glDeleteFramebuffers(1, &r->fbo);
    createFramebuffer(r, w);
}

void initRenderer(Renderer* r, Window* w) {
    gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);

    for (int i = 0; i < sizeof(r->asteroidDrawInfos) / sizeof(DrawInfo); i++) {
        createRandomPolygon(r, &r->asteroidDrawInfos[i]);
        r->asteroidDrawInfos[i].drawMode = GL_LINES;
        r->asteroidDrawInfos[i].color = randomColor();
    }

    createText(r);
    createVAO(r, &r->shipDrawInfo, shipVerts, shipIndices, sizeof(shipVerts), sizeof(shipIndices), false);
    createVAO(r, &r->bulletDrawInfo, bulletVerts, bulletIndices, sizeof(bulletVerts), sizeof(bulletIndices), false);
    createVAO(r, &r->quadDrawInfo, quadVerts, quadIndices, sizeof(quadVerts), sizeof(quadIndices), true);
    createVAO(r, &r->boxDrawInfo, quadVerts, boxIndices, sizeof(quadVerts), sizeof(boxIndices), true);
    createVAO(r, &r->buttonDrawInfo, buttonVerts, boxIndices, sizeof(buttonVerts), sizeof(boxIndices), true);
    loadShader(&r->shader, "shader.vert", "shader.frag");
    loadShader(&r->fullScreenShader, "fullscreenshader.vert", "fullscreenshader.frag");
    createFramebuffer(r, w);

    r->quadDrawInfo.drawMode = GL_TRIANGLE_STRIP;
    r->bulletDrawInfo.drawMode = GL_POINTS;
    r->shipDrawInfo.drawMode = GL_LINES;
    r->boxDrawInfo.drawMode = GL_LINES;
    r->buttonDrawInfo.drawMode = GL_LINES;

    r->textVert.color = randomColor();
    r->lifePos = (vec2){-0.9, 0.8f};
    r->scorePos = (vec2){-0.9, 0.9f};
    r->lifeScale = 0.032f;
    r->scoreScale = 0.03f;
    r->lifePadding = 0.055f;
    r->scorePadding = 0.07f;

    r->clearColor = (vec4){0.0f, 0.0f, 0.0f, 0.0f};
    r->clearDepth = 1.0f;

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_PROGRAM_POINT_SIZE);
}

void initScene(Scene* s, Renderer* r) {
    s->deathTime = 3.0f;
    s->deathTimer = 0.0f;
    s->level = 1;
    s->levelLoadTime = 3.0f;
    s->levelTimer = 0.0f;
    s->bulletSpeed = 2.0f;
    s->bulletLifetime = 0.5f;
    s->lives = 4;
    s->score = 0;
    s->shootSoundPlaying = false;
    s->shootSoundTime = 5.05f;

    s->numAsteroids = 0;
    s->numBullets = 0;

    s->ship.transform.pos = (vec2){0.0f, 0.0f};
    s->ship.transform.vel = (vec2){0.0f, 0.0f};
    s->ship.transform.angle = 0.0f;
    s->ship.transform.scale = 0.05f;
    s->ship.drawInfo = r->shipDrawInfo;
    s->ship.drawInfo.color = randomColor();

    s->ship.maxVel = 0.3f;
    s->ship.turnRate = 4.0f;
    s->ship.accel = 0.007f;
    s->flags = 0;

    setFlag(s, CAN_SHOOT);
    setFlag(s, ALIVE);
}

void windowResized(Window* w, Renderer* r, vec2 res) {
    w->screenSize = res;

    if (res.x > res.y) {
        res.x = res.y;
    } else {
        res.y = res.x;
    }

    w->res = res;
    resizeFramebuffer(r, w);
    glViewport((w->screenSize.x - res.x) * 0.5f, (w->screenSize.y - res.y) * 0.5f, res.x, res.y);
    glUseProgram(r->fullScreenShader);
    glUniform2fv(3, 1, &res.x);
}

void pollEvents(Window* w, Renderer* r) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                w->shouldClose = true;
                break;
            case SDL_EVENT_WINDOW_RESIZED:
                windowResized(w, r, (vec2){event.window.data1, event.window.data2});
                break;
        }
    }
}

void updateTime(Scene* s) {
    float currentTime = (float)SDL_GetTicks() * .001;
    s->deltaTime = currentTime - s->time;
    s->time = currentTime;
}

void updateInput(Input* i) {
    i->move.x = 0;
    i->move.y = 0;

    SDL_MouseButtonFlags mouseButtonMask = SDL_GetMouseState(&i->cursorPosition.x, &i->cursorPosition.y);
    i->mouse0 = mouseButtonMask & SDL_BUTTON_LMASK;
    i->move.x += i->keyStates[SDL_SCANCODE_RIGHT] ? -1 : 0;
    i->move.x += i->keyStates[SDL_SCANCODE_LEFT] ? 1 : 0;
    i->move.y += i->keyStates[SDL_SCANCODE_UP] ? 1 : 0;
    i->move.y += i->keyStates[SDL_SCANCODE_DOWN] ? 0 : 0;
    i->shoot = i->keyStates[SDL_SCANCODE_SPACE];
}

bool wrapScreen(vec2* pos) {
    bool wrapped = false;

    if (pos->x > 1) {
        pos->x = -1;
        wrapped = true;
    } else if (pos->x < -1) {
        pos->x = 1;
        wrapped = true;
    }

    if (pos->y > 1) {
        pos->y = -1;
        wrapped = true;
    } else if (pos->y < -1) {
        pos->y = 1;
        wrapped = true;
    }

    return wrapped;
}

void removeBullet(Scene* s, Bullet* e) {
    Bullet* lastB = &s->bullets[s->numBullets - 1];

    if (e != lastB) {
        *e = *lastB;
    }

    s->numBullets--;
}

void newBullet(Scene* s, Renderer* r, vec2 pos, vec2 vel) {
    Bullet* b = &s->bullets[s->numBullets];
    b->drawInfo = r->bulletDrawInfo;
    b->transform.pos = pos;
    b->transform.vel = vel;
    b->hasWrapped = false;

    b->lifetime = s->bulletLifetime;
    s->numBullets++;
}

void removeAsteroid(Scene* s, Asteroid* a) {
    Asteroid* lastA = &s->asteroids[s->numAsteroids - 1];

    if (a != lastA) {
        *a = *lastA;
    }

    s->numAsteroids--;
}

Asteroid* getNewAsteroid(Scene* s, Renderer* r) {
    Asteroid* a = &s->asteroids[s->numAsteroids];
    a->drawInfo = r->asteroidDrawInfos[randomRangeI(0, 16)];
    s->numAsteroids++;
    return a;
}

void loadLevel(Scene* s, Renderer* r) {
    s->numAsteroids = 0;
    s->numBullets = 0;

    float angleStep = (2 * M_PI) / (4 + s->level);

    for (int i = 0; i < 3 + s->level; i++) {
        Asteroid* e = getNewAsteroid(s, r);
        float     radius = randomRangeF(0.3f, 1.0f);
        float     angle = angleStep * i;

        float x = s->ship.transform.pos.x + radius * cos(angle);
        float y = s->ship.transform.pos.y + radius * sin(angle);

        e->transform.vel = (vec2){randomRangeF(-0.3f, 0.3f), randomRangeF(-0.3f, 0.3f)};
        e->transform.scale = 0.18f;
        e->transform.pos = (vec2){x, y};
        e->level = 3;
    }

    unsetFlag(s, NEW_LEVEL);
    s->levelTimer = 0.0f;
}

void hitAsteroid(Scene* s, Asteroid* hitAsteroid, Renderer* r) {
    hitAsteroid->level -= 1;
    unsigned int score = 100;

    if (hitAsteroid->level == 1) {
        score = 50;
    } else if (hitAsteroid->level == 2) {
        score = 25;
    }

    s->score += score;

    if (s->score % 10000 == 0) {
        s->lives += 1;
    }

    if (hitAsteroid->level != 0) {
        int   mod = 1;
        float scale = hitAsteroid->level == 2 ? 0.07f : 0.03f;
        float minVel = hitAsteroid->level == 2 ? 0.1f : 0.50f;

        if (vec2_magnitude(hitAsteroid->transform.vel) < minVel) {
            vec2 dir = normalize(hitAsteroid->transform.vel);
            hitAsteroid->transform.vel = (vec2){dir.x * minVel, dir.y * minVel};
        }

        for (int i = 0; i < 2; i++) {
            Asteroid* newAsteroid = getNewAsteroid(s, r);
            newAsteroid->level = hitAsteroid->level;
            newAsteroid->transform.pos = hitAsteroid->transform.pos;
            newAsteroid->transform.vel = (vec2){hitAsteroid->transform.vel.x * mod, hitAsteroid->transform.vel.y * mod};
            newAsteroid->transform.scale = scale;
            newAsteroid->transform.angle = hitAsteroid->transform.angle * mod;
            newAsteroid->drawInfo.color = randomColor();
            mod = -1;
        }
    }

    removeAsteroid(s, hitAsteroid);

    if (s->numAsteroids == 0) {
        setFlag(s, NEW_LEVEL);
    }
}

void hitShip(Scene* s, Renderer* r) {
    unsetFlag(s, ALIVE);
    s->deathTimer = 0.0f;
    s->lives--;
    if (s->lives == 0) {
        setFlag(s, GAME_OVER);
        s->levelTimer = 0.0f;
    }
}

void checkCollisions(Scene* s, Renderer* r) {
    Ship* ship = &s->ship;

    for (int i = s->numBullets - 1; i >= 0; i--) {
        Bullet* b = &s->bullets[i];

        for (int k = s->numAsteroids - 1; k >= 0; k--) {
            Asteroid* a = &s->asteroids[k];
            vec2      v = (vec2){a->transform.pos.x - b->transform.pos.x, a->transform.pos.y - b->transform.pos.y};

            if (vec2_magnitude(v) < (a->drawInfo.maxRadius * a->transform.scale)) {
                hitAsteroid(s, a, r);
                removeBullet(s, b);
                break;
            }

            v = (vec2){ship->transform.pos.x - b->transform.pos.x, ship->transform.pos.y - b->transform.pos.y};

            if (vec2_magnitude(v) < (0.9 * ship->transform.scale)) {
                hitShip(s, r);
                removeBullet(s, b);
                break;
            }
        }
    }

    if (s->flags & ALIVE) {
        for (int i = 0; i < s->numAsteroids; i++) {
            Asteroid* a = &s->asteroids[i];
            vec2      v = (vec2){ship->transform.pos.x - a->transform.pos.x, ship->transform.pos.y - a->transform.pos.y};

            if (vec2_magnitude(v) < (a->drawInfo.maxRadius * a->transform.scale * 1.1f)) {
                hitShip(s, r);
                break;
            }
        }
    }
}

bool checkSpawn(Scene* s) {
    for (int i = 0; i < s->numAsteroids; i++) {
        Asteroid* a = &s->asteroids[i];

        if (vec2_magnitude(a->transform.pos) < (a->drawInfo.maxRadius + 0.01) * a->transform.scale) {
            return false;
        }
    }

    return true;
}

void updateDeathTimer(Scene* s, float dt) {
    if (s->deathTimer >= s->deathTime) {
        if (checkSpawn(s)) {
            s->ship.transform.pos = (vec2){0.0f, 0.0f};
            s->ship.transform.angle = 0.0f;
            s->ship.transform.vel = (vec2){0.0f, 0.0f};
            setFlag(s, ALIVE);
        }
    } else {
        s->deathTimer += dt;
    }
}

void playShootSound(Scene* s) {
    s->shootSoundPlaying = true;
    s->shootSoundTimer = 0.0f;
    alSourcePlay(s->shootSoundSource);
}

void stopShootSound(Scene* s) {
    s->shootSoundPlaying = false;
    alSourceStop(s->shootSoundSource);
}

void updateShootSoundTimer(Scene* s) {
    s->shootSoundTimer += s->deltaTime;

    if (s->shootSoundTimer >= s->shootSoundTime) {
        stopShootSound(s);
    }
}

void updateShip(Scene* s, Renderer* r, Input* input, float dt) {
    if (!(s->flags & ALIVE)) {
        updateDeathTimer(s, dt);
        return;
    }

    vec2  move = input->move;
    vec2  fwd = {cos(s->ship.transform.angle + M_PI * 0.5f), sin(s->ship.transform.angle + M_PI * 0.5f)};
    vec2  moveDir = {move.y * fwd.x, move.y * fwd.y};
    vec2  vel = {s->ship.transform.vel.x + moveDir.x * s->ship.accel, s->ship.transform.vel.y + moveDir.y * s->ship.accel};
    float mag = vec2_magnitude(vel);
    vec2  dir = normalize(vel);

    if (mag > s->ship.maxVel) {
        vel.x = dir.x * s->ship.maxVel;
        vel.y = dir.y * s->ship.maxVel;
    }

    vel.x = lerp(vel.x, 0.0f, 0.4f * dt);
    vel.y = lerp(vel.y, 0.0f, 0.4f * dt);

    s->ship.transform.vel = vel;
    s->ship.transform.pos.x += vel.x * dt;
    s->ship.transform.pos.y += vel.y * dt;
    s->ship.transform.angle += move.x * s->ship.turnRate * dt;
    wrapScreen(&s->ship.transform.pos);

    if (input->shoot) {
        if (s->flags & CAN_SHOOT) {
            unsetFlag(s, CAN_SHOOT);
            playShootSound(s);

            vec2 pos = {s->ship.transform.pos.x + fwd.x * 0.1f, s->ship.transform.pos.y + fwd.y * 0.1f};
            vec2 vel = {fwd.x * s->bulletSpeed, fwd.y * s->bulletSpeed};
            newBullet(s, r, pos, vel);
        }
    } else {
        setFlag(s, CAN_SHOOT);
    }

    if (s->shootSoundPlaying) {
        updateShootSoundTimer(s);
    }
}

void updateAsteroids(Scene* s, float dt) {
    for (int i = 0; i < s->numAsteroids; i++) {
        Asteroid* ast = &s->asteroids[i];
        ast->transform.pos.x += ast->transform.vel.x * dt;
        ast->transform.pos.y += ast->transform.vel.y * dt;
        wrapScreen(&ast->transform.pos);
    }
}

void updateBullets(Scene* s, float dt) {
    for (int i = s->numBullets - 1; i >= 0; i--) {
        Bullet* b = &s->bullets[i];
        b->lifetime -= dt;

        if (b->lifetime <= 0 && b->hasWrapped) {
            removeBullet(s, b);
            continue;
        }

        b->transform.pos = (vec2){b->transform.pos.x + b->transform.vel.x * dt, b->transform.pos.y + b->transform.vel.y * dt};
        if (wrapScreen(&b->transform.pos)) {
            b->hasWrapped = true;
        }
    }
}

void updateGameOverScreen(Scene* s, Renderer* r, Input* input) {
    if (s->levelTimer < s->levelLoadTime) {
        s->levelTimer += s->deltaTime;
        return;
    }

    initScene(s, r);
    loadLevel(s, r);
}

void updateScene(Scene* s, Renderer* r, Input* input) {
    float dt = s->deltaTime;

    updateAsteroids(s, dt);
    updateBullets(s, dt);

    // could switch on game state and have separate update functions
    if (s->flags & GAME_OVER) {
        updateGameOverScreen(s, r, input);
        return;
    }

    if (s->flags & NEW_LEVEL) {
        if (s->levelTimer >= s->levelLoadTime) {
            s->level++;
            loadLevel(s, r);
        }

        s->levelTimer += dt;
    }

    updateShip(s, r, input, dt);
    checkCollisions(s, r);
}

void drawCmd(Transform* t, DrawInfo* d) {
    glUniform3fv(U_COLOR, 1, &d->color.x);
    glUniform2fv(U_POS, 1, &t->pos.x);
    glUniform1f(U_SCALE, t->scale);
    glUniform1f(U_ANGLE, t->angle);

    glBindVertexArray(d->vao);
    glDrawElements(d->drawMode, d->indCount, GL_UNSIGNED_INT, (void*)d->indOffset);
}

void drawBorder(Renderer* r) {
    vec2 pos = (vec2){0.0f, 0.0f};

    glUniform2fv(U_POS, 1, &pos.x);
    glUniform1f(U_SCALE, 0.999f);
    glUniform1f(U_ANGLE, 0.0f);

    glBindVertexArray(r->boxDrawInfo.vao);
    glDrawElements(r->boxDrawInfo.drawMode, r->boxDrawInfo.indCount, GL_UNSIGNED_INT, (void*)r->boxDrawInfo.indOffset);
}

void drawScene(Scene* s, Renderer* r, Window* w) {
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0.0f, 0.0f, w->res.x, w->res.y);
    glBindFramebuffer(GL_FRAMEBUFFER, r->fbo);
    glClearNamedFramebufferfv(r->fbo, GL_COLOR, 0, &r->clearColor.x);
    glClearNamedFramebufferfv(r->fbo, GL_DEPTH, 0, &r->clearDepth);

    glUseProgram(r->shader);
    glUniform1f(U_POINTSIZE, 2.2f);
    glUniform1f(U_TIME, s->time);

    for (int i = 0; i < s->numAsteroids; i++) {
        Asteroid* a = &s->asteroids[i];
        drawCmd(&a->transform, &a->drawInfo);
    }

    for (int i = 0; i < s->numBullets; i++) {
        Bullet* b = &s->bullets[i];
        drawCmd(&b->transform, &b->drawInfo);
    }

    if (s->flags & ALIVE) {
        Ship* ship = &s->ship;
        drawCmd(&ship->transform, &ship->drawInfo);
    }

    glUniform1f(U_ANGLE, 0.0f);
    glUniform1f(U_SCALE, r->lifeScale);

    glBindVertexArray(r->shipDrawInfo.vao);

    vec2 pos = r->lifePos;
    for (int i = 0; i < s->lives; i++) {
        glUniform2fv(U_POS, 1, &pos.x);
        glDrawElements(GL_LINES, r->shipDrawInfo.indCount, GL_UNSIGNED_INT, (void*)r->shipDrawInfo.indOffset);
        pos.x += r->lifePadding;
    }

    int base = s->score;
    int count = 0;
    int digits[8];
    digits[0] = 0;

    do {
        digits[count] = base % 10;
        count++;
    } while (base /= 10);

    glUniform1f(U_SCALE, 0.03);
    glBindVertexArray(r->textVert.vao);

    pos = r->scorePos;

    for (int i = count - 1; i >= 0; i--) {
        glUniform2fv(U_POS, 1, &pos.x);
        glUniform3fv(U_COLOR, 1, &r->textVert.color.x);
        glDrawElements(GL_LINES, r->text[digits[i]].indCount, GL_UNSIGNED_INT, (void*)r->text[digits[i]].indOffset);
        pos.x += r->scorePadding;
    }

    drawBorder(r);
}

void drawFullScreenQuad(Renderer* r, Window* w) {
    glViewport((w->screenSize.x - w->res.x) * 0.5f, (w->screenSize.y - w->res.y) * 0.5f, w->res.x, w->res.y);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearNamedFramebufferfv(0, GL_COLOR, 0, &r->clearColor.x);
    glClearNamedFramebufferfv(0, GL_DEPTH, 0, &r->clearDepth);
    glUseProgram(r->fullScreenShader);
    glBindTextureUnit(0, r->renderTarget);
    glBindVertexArray(r->quadDrawInfo.vao);
    glDrawElements(GL_TRIANGLE_STRIP, r->quadDrawInfo.indCount, GL_UNSIGNED_INT, (void*)r->boxDrawInfo.indOffset);
}

int main(int argc, char* argv[]) {
    Window*   window = malloc(sizeof(Window));
    Renderer* renderer = malloc(sizeof(Renderer));
    Input*    input = malloc(sizeof(Input));
    Scene*    scene = malloc(sizeof(Scene));

    srand(time(NULL));
    initWindow(window, input);
    initRenderer(renderer, window);
    initScene(scene, renderer);
    loadLevel(scene, renderer);

    scene->shootSoundSource = getToneSource(argc, argv);

    updateTime(scene);
    scene->deltaTime = 0.0f;
    while (!window->shouldClose) {
        updateTime(scene);
        pollEvents(window, renderer);
        updateInput(input);
        updateScene(scene, renderer, input);
        drawScene(scene, renderer, window);
        drawFullScreenQuad(renderer, window);
        SDL_GL_SwapWindow(window->ptr);
        ALint processedBufs;
        alGetSourcei(scene->shootSoundSource, AL_BUFFERS_PROCESSED, &processedBufs);
        // printf("processed: %i\n", processedBufs);
    }

    return 0;
}
