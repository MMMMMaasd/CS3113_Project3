/**
* Author: [Michael Bian]
* Assignment: Lunar Lander
* Date due: 2024-10-27, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define LOG(argument) std::cout << argument << '\n'
#define STB_IMAGE_IMPLEMENTATION
#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1

#ifdef _WINDOWS
    #include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "Entity.h"
#include <vector>
#include <ctime>
#include "cmath"

// ————— CONSTANTS ————— //
// constexpr float ACC_OF_GRAVITY = -9.81f;
constexpr float ACC_OF_GRAVITY = -0.52f;

constexpr int WINDOW_WIDTH  = 640 * 2,
              WINDOW_HEIGHT = 480 * 2;

constexpr float LEFT_BORDER = -4.55;
constexpr float RIGHT_BORDER = 4.55;

constexpr float BG_RED     = 0.9765625f,
                BG_GREEN   = 0.97265625f,
                BG_BLUE    = 0.9609375f,
                BG_OPACITY = 1.0f;

constexpr int VIEWPORT_X = 0,
              VIEWPORT_Y = 0,
              VIEWPORT_WIDTH  = WINDOW_WIDTH,
              VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
               F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

constexpr float MILLISECONDS_IN_SECOND = 1000.0;
constexpr char SUBMARINE_FILEPATH[] = "assets/submarine.png",
               DEEPOCEAN_FILEPATH[] = "assets/deep-ocean.jpg",
MISSIONACCOMPLISH_FILEPATH[] = "assets/mission-Accomplish.png",
MISSIONFAIL_FILEPATH[] = "assets/eaten.png";
            

constexpr char PLATFORM_FILEPATH[]    = "assets/winPlatform.png",
LOSE_PLATFORM_FILEPATH[] = "assets/losePlatform.png";
 
constexpr glm::vec3 SUBMARINE_INITSCALE = glm::vec3(1.37f, 1.0f, 0.0f),
                    BACKGROUND_INITSCALE = glm::vec3(15.8f, 8.0f, 0.0f),
                    WIN_PLATFORMS_INITSCALE = glm::vec3(1.5f, 1.5f, 0.0f),
                    WIN_MESSAGE_INITSCALE = glm::vec3(4.53f, 3.0f, 0.0f),
                    LOSE_MESSAGE_INITSCALE = glm::vec3(3.93f, 3.0f, 0.0f),
                    LOSE_PLATFORMS_INITSCALE = glm::vec3(4.5f, 0.5f, 0.0f);
                 


constexpr GLint NUMBER_OF_TEXTURES = 1,
                LEVEL_OF_DETAIL    = 0,
                TEXTURE_BORDER     = 0;

constexpr int PLATFORM_COUNT = 3;
constexpr int PLATFORM_LOSE_COUNT = 7;

bool ifGameEnd = false;
bool ifWin = false;
bool ifLose = false;

// ————— STRUCTS AND ENUMS —————//
enum AppStatus  { RUNNING, TERMINATED };
enum FilterType { NEAREST, LINEAR     };

struct GameState
{
    Entity* player;
    Entity* platforms;
    Entity* background;
    Entity** Platforms_lost;
    Entity* lose_message;
    Entity* win_message;
    // Entity** npcs;
};

// ————— VARIABLES ————— //
GameState g_game_state;

SDL_Window* g_display_window;
AppStatus g_app_status = RUNNING;

ShaderProgram g_shader_program;
glm::mat4 g_view_matrix, g_projection_matrix;

float g_previous_ticks = 0.0f;

void initialise();
void process_input();
void update();
void render();
void shutdown();

GLuint load_texture(const char* filepath);

// ———— GENERAL FUNCTIONS ———— //
GLuint load_texture(const char* filepath, FilterType filterType)
{
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);
    
    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }
    
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER,
                 GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    filterType == NEAREST ? GL_NEAREST : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                    filterType == NEAREST ? GL_NEAREST : GL_LINEAR);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    stbi_image_free(image);
    
    return textureID;
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("Hello, Entities!",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL);
    
    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);
    
    if (g_display_window == nullptr)
    {
        std::cerr << "Error: SDL window could not be created.\n";
        shutdown();
    }
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);
    
    g_view_matrix       = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);
    
    glUseProgram(g_shader_program.get_program_id());
    
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    // ————— PLAYER ————— //
    // GLuint player_texture_id = load_texture(GEORGE_FILEPATH, NEAREST);
    /*
     int player_walking_animation[4][4] =
     {
         { 1, 5, 9, 13 },  // for George to move to the left,
         { 3, 7, 11, 15 }, // for George to move to the right,
         { 2, 6, 10, 14 }, // for George to move upwards,
         { 0, 4, 8, 12 }   // for George to move downwards
     };
     
     g_game_state.player = new Entity(
         player_texture_id,         // texture id
         1.0f,                      // speed
         player_walking_animation,  // animation index sets
         0.0f,                      // animation time
         4,                         // animation frame amount
         0,                         // current animation index
         4,                         // animation column amount
         4                          // animation row amount
     );
     
     */
    GLuint player_texture_id = load_texture(SUBMARINE_FILEPATH, NEAREST);
    
    g_game_state.player = new Entity(player_texture_id, 1.0f);
    
    //g_game_state.player->face_down();
    g_game_state.player->set_acceleration(glm::vec3(0.0f, ACC_OF_GRAVITY * 0.1, 0.0f));
    g_game_state.player->set_position(glm::vec3(0.0f, 4.0f, 0.0f));
    g_game_state.player->set_scale(SUBMARINE_INITSCALE);
    g_game_state.player->set_width(SUBMARINE_INITSCALE.x);
    g_game_state.player->set_height(SUBMARINE_INITSCALE.y);
    
    GLuint background_texture_id = load_texture(DEEPOCEAN_FILEPATH, NEAREST);
    g_game_state.background = new Entity(background_texture_id, 1.0f);
    g_game_state.background->set_scale(BACKGROUND_INITSCALE);
    g_game_state.background->update(0.0f);

    
    // ————— PLATFORM FOR WIN ————— //
    g_game_state.platforms = new Entity[PLATFORM_COUNT];
    for (int i = 0; i < PLATFORM_COUNT; i++)
    {
            if(i == 1){
                g_game_state.platforms[i].set_texture_id(load_texture(PLATFORM_FILEPATH, NEAREST));
                g_game_state.platforms[i].set_position(glm::vec3(-0.5f, -2.8f, 0.0f));
            }else if(i == 2){
                g_game_state.platforms[i].set_texture_id(load_texture(PLATFORM_FILEPATH, NEAREST));
                g_game_state.platforms[i].set_position(glm::vec3(1.6f, 1.5f, 0.0f));
            }else{
                g_game_state.platforms[i].set_texture_id(load_texture(PLATFORM_FILEPATH, NEAREST));
                g_game_state.platforms[i].set_position(glm::vec3(3.2f, -2.5f, 0.0f));
            }
            g_game_state.platforms[i].set_scale(WIN_PLATFORMS_INITSCALE);
            g_game_state.platforms[i].set_width((WIN_PLATFORMS_INITSCALE.x)-1.2);
            g_game_state.platforms[i].set_height((WIN_PLATFORMS_INITSCALE.y)-1.2);
            g_game_state.platforms[i].update(0.0f, nullptr, nullptr, 0, 0, ifGameEnd, ifLose, ifWin);
    }
    // ————— PLATFORM FOR LOSE ————— //
    
    g_game_state.Platforms_lost = new Entity* [PLATFORM_LOSE_COUNT];
    
    for (int i = 0; i < PLATFORM_LOSE_COUNT; ++i){
        g_game_state.Platforms_lost[i] = nullptr;
    }
    
    g_game_state.Platforms_lost[0] = new Entity();
    g_game_state.Platforms_lost[0]->set_texture_id(load_texture(LOSE_PLATFORM_FILEPATH, NEAREST));
    g_game_state.Platforms_lost[0]->set_position(glm::vec3(-4.0f, -1.0f, 0.0f));
    g_game_state.Platforms_lost[0]->set_rotate_angle(glm::radians(-30.0f));
    g_game_state.Platforms_lost[0]->set_rotate_vec(glm::vec3(0.0f, 0.0f, 1.0f));
    g_game_state.Platforms_lost[0]->set_scale(LOSE_PLATFORMS_INITSCALE);
    g_game_state.Platforms_lost[0]->set_width((LOSE_PLATFORMS_INITSCALE.x)-0.1f);
    g_game_state.Platforms_lost[0]->set_height((LOSE_PLATFORMS_INITSCALE.y)-0.1f);
    
    g_game_state.Platforms_lost[1] = new Entity();
    g_game_state.Platforms_lost[1]->set_texture_id(load_texture(LOSE_PLATFORM_FILEPATH, NEAREST));
    g_game_state.Platforms_lost[1]->set_position(glm::vec3(-3.1f, -1.3f, 0.0f));
    g_game_state.Platforms_lost[1]->set_rotate_angle(glm::radians(70.0f));
    g_game_state.Platforms_lost[1]->set_rotate_vec(glm::vec3(0.0f, 0.0f, 1.0f));
    g_game_state.Platforms_lost[1]->set_scale(glm::vec3(1.2f, 0.5f, 0.0f));
    g_game_state.Platforms_lost[1]->set_width(1.1f);
    g_game_state.Platforms_lost[1]->set_height(0.4f);
    
    g_game_state.Platforms_lost[2] = new Entity();
    g_game_state.Platforms_lost[2]->set_texture_id(load_texture(LOSE_PLATFORM_FILEPATH, NEAREST));
    g_game_state.Platforms_lost[2]->set_position(glm::vec3(-2.6f, -1.1f, 0.0f));
    g_game_state.Platforms_lost[2]->set_rotate_angle(glm::radians(-20.0f));
    g_game_state.Platforms_lost[2]->set_rotate_vec(glm::vec3(0.0f, 0.0f, 1.0f));
    g_game_state.Platforms_lost[2]->set_scale(glm::vec3(1.2f, 0.5f, 0.0f));
    g_game_state.Platforms_lost[2]->set_width(1.1f);
    g_game_state.Platforms_lost[2]->set_height(0.4f);
    
    g_game_state.Platforms_lost[3] = new Entity();
    g_game_state.Platforms_lost[3]->set_texture_id(load_texture(LOSE_PLATFORM_FILEPATH, NEAREST));
    g_game_state.Platforms_lost[3]->set_position(glm::vec3(-1.62f, -2.2f, 0.0f));
    g_game_state.Platforms_lost[3]->set_rotate_angle(glm::radians(-55.0f));
    g_game_state.Platforms_lost[3]->set_rotate_vec(glm::vec3(0.0f, 0.0f, 1.0f));
    g_game_state.Platforms_lost[3]->set_scale(glm::vec3(4.8f, 0.5f, 0.0f));
    g_game_state.Platforms_lost[3]->set_width(1.0f);
    g_game_state.Platforms_lost[3]->set_height(0.4f);
    
    g_game_state.Platforms_lost[4] = new Entity();
    g_game_state.Platforms_lost[4]->set_texture_id(load_texture(LOSE_PLATFORM_FILEPATH, NEAREST));
    g_game_state.Platforms_lost[4]->set_position(glm::vec3(1.2f, -3.1f, 0.0f));
    g_game_state.Platforms_lost[4]->set_rotate_angle(glm::radians(0.0f));
    g_game_state.Platforms_lost[4]->set_rotate_vec(glm::vec3(0.0f, 0.0f, 1.0f));
    g_game_state.Platforms_lost[4]->set_scale(glm::vec3(4.8f, 0.5f, 0.0f));
    g_game_state.Platforms_lost[4]->set_width(4.7f);
    g_game_state.Platforms_lost[4]->set_height(0.4f);
    
    g_game_state.Platforms_lost[5] = new Entity();
    g_game_state.Platforms_lost[5]->set_texture_id(load_texture(LOSE_PLATFORM_FILEPATH, NEAREST));
    g_game_state.Platforms_lost[5]->set_position(glm::vec3(2.4f, -3.0f, 0.0f));
    g_game_state.Platforms_lost[5]->set_rotate_angle(glm::radians(50.0f));
    g_game_state.Platforms_lost[5]->set_rotate_vec(glm::vec3(0.0f, 0.0f, 1.0f));
    g_game_state.Platforms_lost[5]->set_scale(glm::vec3(1.1f, 0.5f, 0.0f));
    g_game_state.Platforms_lost[5]->set_width(1.0f);
    g_game_state.Platforms_lost[5]->set_height(0.4f);
    
    g_game_state.Platforms_lost[6] = new Entity();
    g_game_state.Platforms_lost[6]->set_texture_id(load_texture(LOSE_PLATFORM_FILEPATH, NEAREST));
    g_game_state.Platforms_lost[6]->set_position(glm::vec3(4.4f, -2.8f, 0.0f));
    g_game_state.Platforms_lost[6]->set_scale(glm::vec3(2.6f, 0.5f, 0.0f));
    g_game_state.Platforms_lost[6]->set_width(2.5f);
    g_game_state.Platforms_lost[6]->set_height(0.4f);
    
    for(int i = 0; i < PLATFORM_LOSE_COUNT; ++i){
        g_game_state.Platforms_lost[i]->update(0.0f, nullptr, nullptr, 0, 0, ifGameEnd, ifLose, ifWin);
    }
    
    // ————— WIN MESSAGE ————— //
    g_game_state.win_message = new Entity();
    g_game_state.win_message->set_texture_id(load_texture(MISSIONACCOMPLISH_FILEPATH, NEAREST));
    g_game_state.win_message->set_position(glm::vec3(0.0f, 0.0f, 0.0f));
    g_game_state.win_message->set_scale(WIN_MESSAGE_INITSCALE);
    if(ifGameEnd && ifWin){
        g_game_state.win_message->update(0.0f, nullptr, nullptr, 0, 0, ifGameEnd, ifLose, ifWin);
    }
    
    g_game_state.lose_message = new Entity();
    g_game_state.lose_message->set_texture_id(load_texture(MISSIONFAIL_FILEPATH, NEAREST));
    g_game_state.lose_message->set_position(glm::vec3(0.0f, 0.0f, 0.0f));
    g_game_state.lose_message->set_scale(LOSE_MESSAGE_INITSCALE);
    if(ifGameEnd && ifLose){
        g_game_state.lose_message->update(0.0f, nullptr, nullptr, 0, 0, ifGameEnd, ifLose, ifWin);
    }
    
    // ————— GENERAL ————— //
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    // VERY IMPORTANT: If nothing is pressed, we don't want to go anywhere
    g_game_state.player->set_movement(glm::vec3(0.0f));
    g_game_state.player->set_acceleration(glm::vec3(0.0f, ACC_OF_GRAVITY * 0.1, 0.0f));
    
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                g_app_status = TERMINATED;
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_q: g_app_status = TERMINATED;
                    default:     break;
                }
                
            default:
                break;
        }
    }
    
    const Uint8 *key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_A])
    {
        if((g_game_state.player->get_position()).x >= LEFT_BORDER){
            g_game_state.player->accelerate_left();
            g_game_state.player->set_rotate_angle(glm::radians(0.0f));
        }
    }
    else if (key_state[SDL_SCANCODE_D])
    {
        if((g_game_state.player->get_position()).x <= RIGHT_BORDER) {
            g_game_state.player->accelerate_right();
            g_game_state.player->set_rotate_angle(-glm::radians(180.0f));
        }
    }
    
    if (key_state[SDL_SCANCODE_W])
    {
        g_game_state.player->accelerate_up();
    }
    else if (key_state[SDL_SCANCODE_S])
    {
        g_game_state.player->accelerate_down();
    }
    
    if (glm::length(g_game_state.player->get_movement()) > 1.0f)
        g_game_state.player->normalise_movement();
}


constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;

float g_time_accumulator = 0.0f;

void update()
{
    
    // ————— DELTA TIME ————— //
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND; // get the current number of ticks
    float delta_time = ticks - g_previous_ticks; // the delta time is the difference from the last frame
    g_previous_ticks = ticks;

    // ————— FIXED TIMESTEP ————— //
    delta_time += g_time_accumulator;

    if (delta_time < FIXED_TIMESTEP)
    {
        g_time_accumulator = delta_time;
        return;
    }

    while (delta_time >= FIXED_TIMESTEP)
    {
        
        g_game_state.player->update(FIXED_TIMESTEP, g_game_state.platforms, g_game_state.Platforms_lost,
                                    PLATFORM_COUNT, PLATFORM_LOSE_COUNT, ifGameEnd, ifLose, ifWin);
        // for (int i = 0; i < NUMBER_OF_NPCS; i++) g_game_state.npcs[i]->update(delta_time);
        if(ifGameEnd && ifWin){
            g_game_state.win_message->update(0.0f, nullptr, nullptr, 0, 0, ifGameEnd, ifLose, ifWin);
        } else if(ifGameEnd && ifLose){
            g_game_state.lose_message->update(0.0f, nullptr, nullptr, 0, 0, ifGameEnd, ifLose, ifWin);
        }
        delta_time -= FIXED_TIMESTEP;
        
            
    }

    g_time_accumulator = delta_time;
    // g_game_state.player->update(delta_time);
}


void render()
{
    glClear(GL_COLOR_BUFFER_BIT);
    
    g_game_state.background->render(&g_shader_program);
    
    g_game_state.player->render(&g_shader_program);
    
    /*
    for (int i = 0; i < NUMBER_OF_NPCS; i++)
        g_game_state.npcs[i]->render(&g_shader_program);
    */
    for (int i = 0; i < PLATFORM_COUNT; i++){
        g_game_state.platforms[i].render(&g_shader_program);
    }
    
    for (int i = 0; i < PLATFORM_LOSE_COUNT; ++i){
        g_game_state.Platforms_lost[i]->render(&g_shader_program);
    }
    
    if(ifGameEnd && ifWin){
        g_game_state.win_message->render(&g_shader_program);
    } else if(ifGameEnd && ifLose){
        g_game_state.lose_message->render(&g_shader_program);
    }
    SDL_GL_SwapWindow(g_display_window);
}


void shutdown()
{
    SDL_Quit();
    delete   g_game_state.player;
    delete   g_game_state.background;
    delete   g_game_state.win_message;
    delete[]   g_game_state.platforms;
    for(int i = 0; i < PLATFORM_LOSE_COUNT; ++i){
        delete   g_game_state.Platforms_lost[i];
    }
    delete[] g_game_state.Platforms_lost;

}


int main(int argc, char* argv[])
{
    initialise();
    
    while (g_app_status == RUNNING)
    {
        process_input();
        update();
        render();
    }
    
    shutdown();
    return 0;
}

