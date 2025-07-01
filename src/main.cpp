/*
#################################################################
TITLE:
CPP_TEMPLATE_GAME_2D_BASIC

DESCRIPTION:
Basic 2D Game Engine

Gameobject can render within window bounds

#################################################################
*/
#include <iostream>
#include <windows.h>
#include <thread>
#include <chrono>
#include <cmath>
#include <string>

class Window
{
private:
    int WINDOW_WIDTH;
    int WINDOW_HEIGHT;

    char** BUFFER_FRONT;
    char** BUFFER_BACK;
    bool BUFFERSWAP = true;

public:
    Window(int width, int height){
        WINDOW_WIDTH = width;
        WINDOW_HEIGHT = height;

        BUFFER_FRONT = new char*[WINDOW_WIDTH];
        for (int i = 0; i < WINDOW_WIDTH; i++)
            BUFFER_FRONT[i] = new char[WINDOW_HEIGHT];

        BUFFER_BACK = new char*[WINDOW_WIDTH];
        for (int i = 0; i < WINDOW_WIDTH; i++)
            BUFFER_BACK[i] = new char[WINDOW_HEIGHT];

        for(int y=0; y<WINDOW_HEIGHT; y++){
            for(int x=0; x<WINDOW_WIDTH; x++){
                BUFFER_BACK[x][y] = ' ';
                BUFFER_FRONT[x][y] = ' ';
            }
        }
    };

    ~Window() {
        for (int i = 0; i < WINDOW_WIDTH; i++)
            delete[] BUFFER_FRONT[i];
        delete[] BUFFER_FRONT;

        for (int i = 0; i < WINDOW_WIDTH; i++)
            delete[] BUFFER_BACK[i];
        delete[] BUFFER_BACK;
    };

    void drawBorder(char character){
        for( int y=0; y<WINDOW_HEIGHT; y++){
            for( int x=0; x<WINDOW_WIDTH; x++){
                if( x==0 || x==WINDOW_WIDTH-1 || y==0 || y==WINDOW_HEIGHT-1)
                    if(BUFFERSWAP)             
                        BUFFER_BACK[x][y] = character;
                    else
                        BUFFER_FRONT[x][y] = character;
            }
        }
    };

    void draw(int xPos,int yPos,char character){
        if(BUFFERSWAP)             
            BUFFER_BACK[xPos][yPos] = character;
        else
            BUFFER_FRONT[xPos][yPos] = character;
    };

    void drawSquare(int xPos,int yPos,int width,int height,char character){
        for(int y=yPos; y<yPos+height; y++){
            for(int x=xPos; x<xPos+width; x++){ 
                if(BUFFERSWAP)             
                    BUFFER_BACK[x][y] = character;
                else
                    BUFFER_FRONT[x][y] = character;
            }
        }
    };

    void drawSprite(int xPos,int yPos,char sprite[5][5]){
        for (int y = yPos; y < yPos + 5; y++) {
            for (int x = xPos; x < xPos + 5; x++) {
                if (x < 0 || x >= WINDOW_WIDTH || y < 0 || y >= WINDOW_HEIGHT)
                    continue;
                
                int index_x = x - xPos;
                int index_y = y - yPos;

                if (BUFFERSWAP)             
                    BUFFER_BACK[x][y] = sprite[index_y][index_x];
                else
                    BUFFER_FRONT[x][y] = sprite[index_y][index_x];
            }
        }
    };

    void buffer_clear(){
        for(int y=0; y<WINDOW_HEIGHT; y++){
            for(int x=0; x<WINDOW_WIDTH; x++){
                if(BUFFERSWAP)
                    BUFFER_BACK[x][y] = ' ';
                else
                    BUFFER_FRONT[x][y] = ' ';
            }
        }
    };

    void buffer_swap(){ BUFFERSWAP = !BUFFERSWAP; };

    void render(){
        HANDLE handle_console = GetStdHandle( STD_OUTPUT_HANDLE );

        CONSOLE_CURSOR_INFO cursorInfo;
        cursorInfo.dwSize = 100;
        cursorInfo.bVisible = FALSE;
        SetConsoleCursorInfo( handle_console, &cursorInfo);

        COORD coord = { 0, 0 };
        SetConsoleCursorPosition( handle_console, coord);

        for(int y=0; y<WINDOW_HEIGHT; y++){
            for(int x=0; x<WINDOW_WIDTH; x++){
                if(BUFFERSWAP)
                    std::cout << BUFFER_BACK[x][y];
                else
                    std::cout << BUFFER_FRONT[x][y];
            }
            std::cout << std::endl;
        }
        std::cout << std::flush;
    };

    int return_int_window_width(){ return WINDOW_WIDTH; };
    int return_int_window_height(){ return WINDOW_HEIGHT; };
};

class GameObject
{
private:
    std::string OBJECT_LABEL;
    float OBJECT_XPOS;
    float OBJECT_YPOS;
    char OBJECT_SPRITE;

public:
    GameObject(std::string label,float xPos,float yPos,char sprite){
        OBJECT_LABEL = label;
        OBJECT_XPOS = xPos;
        OBJECT_YPOS = yPos;
        OBJECT_SPRITE = sprite;
    };

    void set_gameobject_xpos(float xPos){ OBJECT_XPOS = xPos; };

    void set_gameobject_ypos(float yPos){ OBJECT_YPOS = yPos; };

    void set_gameobject_sprite(char sprite){ OBJECT_SPRITE = sprite; };

    float return_float_gameobject_xpos(){ return OBJECT_XPOS; };

    float return_float_gameobject_ypos(){ return OBJECT_YPOS; };

    int return_int_gameobject_xpos(){ return int(OBJECT_XPOS); };

    int return_int_gameobject_ypos(){ return int(OBJECT_YPOS); };

    char return_char_gameobject_sprite(){ return OBJECT_SPRITE; };
};

class Physics
{
private:
    GameObject* GAMEOBJECT;
public:
    Physics(GameObject* gameObject){ GAMEOBJECT = gameObject; };

    void force_simple_x_axis(float force){ GAMEOBJECT->set_gameobject_xpos(GAMEOBJECT->return_float_gameobject_xpos()+force); };

    void force_simple_y_axis(float force){ GAMEOBJECT->set_gameobject_ypos(GAMEOBJECT->return_float_gameobject_ypos()+force); };
};

class Collision
{
private:
    GameObject* GAMEOBJECT;
    Window* HANDLE_WINDOW;
public:
    Collision(GameObject* gameObject, Window* window){
        GAMEOBJECT = gameObject;
        HANDLE_WINDOW = window;
    };

    void detect_collision_border(){
        if( GAMEOBJECT->return_float_gameobject_xpos() >= HANDLE_WINDOW->return_int_window_width()-2)
            GAMEOBJECT->set_gameobject_xpos(HANDLE_WINDOW->return_int_window_width()-2);
        if( GAMEOBJECT->return_float_gameobject_xpos() <= 2)
            GAMEOBJECT->set_gameobject_xpos(2);
        if( GAMEOBJECT->return_float_gameobject_ypos() >= HANDLE_WINDOW->return_int_window_height()-2)
            GAMEOBJECT->set_gameobject_ypos(HANDLE_WINDOW->return_int_window_height()-2);
        if( GAMEOBJECT->return_float_gameobject_ypos() <= 2)
            GAMEOBJECT->set_gameobject_ypos(2);
    };
};

class User
{
public:
    bool quit(){
        if(GetAsyncKeyState(VK_ESCAPE) & 0x8000)
            return true;
        else
            return false;
    };
};

class Game
{
private:
    bool GAMEACTIVE = true;
    Window HANDLE_WINDOW;
    
public:
    Game(): HANDLE_WINDOW(100,10){};

    void run(){
        
        User user;

        GameObject player_1("player_1",25,3,'X');
        GameObject player_2("player_2",75,3,'O');

        Physics player1_physics(&player_1);
        Physics player2_physics(&player_2);

        Collision player1_collision(&player_1, &HANDLE_WINDOW);
        Collision player2_collision(&player_2, &HANDLE_WINDOW);

        float x=20;
        bool forward = true;

        while(GAMEACTIVE){

            // Physics
            player1_physics.force_simple_x_axis(0.025f);
            player2_physics.force_simple_x_axis(-0.025f);

            // Collision
            player1_collision.detect_collision_border();
            player2_collision.detect_collision_border();

            // Input
            if(user.quit() == true){ GAMEACTIVE = false; };

            // Window
            HANDLE_WINDOW.buffer_clear();

            HANDLE_WINDOW.drawBorder('#');
            HANDLE_WINDOW.draw(player_1.return_int_gameobject_xpos(),player_1.return_int_gameobject_ypos(),player_1.return_char_gameobject_sprite());
            HANDLE_WINDOW.draw(player_2.return_int_gameobject_xpos(),player_2.return_int_gameobject_ypos(),player_2.return_char_gameobject_sprite());

            char sprite[5][5] = {
                {' ', '_', '_', '_', ' '},
                {'|', 'o', ' ', 'o', '|'},
                {'|', ' ', '_', ' ', '|'},
                {' ', '|', '_', '|', ' '},
                {' ', ' ', ' ', ' ', ' '}
            };

            if(forward){
                if(x>80){
                    forward = false;
                }
                HANDLE_WINDOW.drawSprite(x,3,sprite);
                x = x+0.25f;
            }else{
                if(x<20){    
                    forward = true;
                }
                HANDLE_WINDOW.drawSprite(x,3,sprite);
                x = x-0.25f;
            }

            HANDLE_WINDOW.buffer_swap();
            HANDLE_WINDOW.render();
        }
    };
};

int main(int argc, char* argv[])
{
    Game game;
    game.run();
    return 0;
}