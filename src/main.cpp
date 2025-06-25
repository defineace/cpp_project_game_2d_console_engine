/*
#################################################################
TITLE:
CPP_TEMPLATE_GAME_2D_BASIC

DESCRIPTION:
Basic 2D Game Engine, where it renders border 
and player. Player can be moved with arrow keys. Engine writes
to a buffer and then to the console buffer.

#################################################################
*/
#include <iostream>
#include <windows.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <chrono>

int MAP_WIDTH = 100;
int MAP_HEIGHT = 8;
bool GAMEACTIVE = true;

float PLAYER_XPOS = 10;
float PLAYER_YPOS = 3;
float PLAYER_SPEED = .075;

float ENEMY_XPOS = 75;
float ENEMY_YPOS = 3;

class Enemy
{
private:
    int enemy_type;
    float enemy_xPos;
    float enemy_yPos;
public:
    Enemy( float xPos, float yPos ){
        enemy_xPos = xPos;
        enemy_yPos = yPos;
    };

    void update_enemy_position( float xPos, float yPos ){
        enemy_xPos = xPos;
        enemy_yPos = yPos;
    };

    float return_enemy_xpos(){
        return enemy_xPos;
    };

    float return_enemy_ypos(){
        return enemy_yPos;
    };
};

class Bullet
{
private:
    float bullet_xPos;
    float bullet_yPos;
public:
    Bullet(float xPos, float yPos){
        bullet_xPos = xPos;
        bullet_yPos = yPos;
    };

    void update_bullet_position(float xPos, float yPos){
        bullet_xPos = xPos;
        bullet_yPos = yPos;
    };

    float return_bullet_xpos(){
        return bullet_xPos;
    };

    float return_bullet_ypos(){
        return bullet_yPos;
    };
};

std::vector<Enemy*> ENEMIES;
std::vector<Bullet*> PLAYER_BULLETS;

void collisions(){
    // Player Bullet
    if(PLAYER_BULLETS.size() > 0){
        for( int i=0; i<PLAYER_BULLETS.size(); i++){
            if( int(PLAYER_BULLETS[i]->return_bullet_xpos()) == MAP_WIDTH-5 ){
                delete PLAYER_BULLETS[i];
                PLAYER_BULLETS.erase(PLAYER_BULLETS.begin() + i);
            }else if(ENEMIES.size() > 0){
                for( int e=0; e<ENEMIES.size(); e++){
                    if( int(ENEMIES[e]->return_enemy_xpos()) == int(PLAYER_BULLETS[i]->return_bullet_xpos()) && int(ENEMIES[e]->return_enemy_ypos()) == int(PLAYER_BULLETS[i]->return_bullet_ypos()) ){
                        delete ENEMIES[e];
                        ENEMIES.erase(ENEMIES.begin() + e);
                    }
                }
            }
        }
    }


}

void physics(){
    // Player Bullet
    if(PLAYER_BULLETS.size() > 0){
        for( int i=0; i<PLAYER_BULLETS.size(); i++){
            float bullet_speed = 0.25f;
            float new_xPos = PLAYER_BULLETS[i]->return_bullet_xpos() + bullet_speed;
            float new_yPos = PLAYER_BULLETS[i]->return_bullet_ypos();
            PLAYER_BULLETS[i]->update_bullet_position( new_xPos,new_yPos );
        }
    }
}

void input(){
    bool pressed = false;

    // Quit Game
    if(GetAsyncKeyState(VK_ESCAPE) & 0x8000){
        GAMEACTIVE = false;
    }

    // Fire Bullet
    if(GetAsyncKeyState(VK_SPACE) & 0x8000 && !pressed){
        PLAYER_BULLETS.push_back(new Bullet(PLAYER_XPOS,PLAYER_YPOS));
    }

    // Spawn Enemy
    if(GetAsyncKeyState(VK_DELETE) & 0x8000){
        std::srand(static_cast<unsigned int>(std::time(0)));
        ENEMIES.push_back(new Enemy( 90 , std::rand() % (MAP_HEIGHT-2) + 2 ) );
    }

    // Player Movement
    if(GetAsyncKeyState(VK_RIGHT) & 0x8000 && !pressed){
        pressed = true;
        PLAYER_XPOS += PLAYER_SPEED;
    }else if(GetAsyncKeyState(VK_LEFT) & 0x8000 && !pressed){
        pressed = true;
        PLAYER_XPOS -= PLAYER_SPEED;
    }else if(GetAsyncKeyState(VK_DOWN) & 0x8000 && !pressed){
        pressed = true;
        PLAYER_YPOS += PLAYER_SPEED;
    }else if(GetAsyncKeyState(VK_UP) & 0x8000 && !pressed){
        pressed = true;
        PLAYER_YPOS -= PLAYER_SPEED;
    }else if(!GetAsyncKeyState(VK_RIGHT) & 0x8000 || !GetAsyncKeyState(VK_LEFT) & 0x8000 && pressed ){
        pressed = false;
    }
};

void render(){
    HANDLE handle_console = GetStdHandle( STD_OUTPUT_HANDLE );

    CONSOLE_CURSOR_INFO cursorInfo;
    cursorInfo.dwSize = 100;
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo( handle_console, &cursorInfo);

    COORD coord = { 0, 0 };
    SetConsoleCursorPosition( handle_console, coord);
    
    int buffer_gameboard[MAP_WIDTH][MAP_HEIGHT] = {0};
    
    // Write Buffer
    // !!! You'll add items here, that need to be drawn

    for( int y=0; y<MAP_HEIGHT; y++){
        for( int x=0; x<MAP_WIDTH; x++){

            // Border
            if( x==0 || x==MAP_WIDTH-1 || y==0 || y==MAP_HEIGHT-1){
                buffer_gameboard[x][y] = 1;
            }

            // Player
            if( x==int(PLAYER_XPOS) && y==int(PLAYER_YPOS) ){
                buffer_gameboard[x][y] = 2;
            }
            
            // Player Bullet
            if(PLAYER_BULLETS.size() > 0){
                for( int i=0; i<PLAYER_BULLETS.size(); i++){
                    if( x == int(PLAYER_BULLETS[i]->return_bullet_xpos()) && y == int(PLAYER_BULLETS[i]->return_bullet_ypos()) ){
                        buffer_gameboard[x][y] = 3;
                    }
                }
            }

            // Enemy
            if(ENEMIES.size() > 0){
                for( int i=0; i<ENEMIES.size(); i++){
                    if( x == int(ENEMIES[i]->return_enemy_xpos()) && y == int(ENEMIES[i]->return_enemy_ypos()) ){
                        buffer_gameboard[x][y] = 4;
                    }
                }
            }
        }
    }

    // Render Buffer
    for( int y=0; y<MAP_HEIGHT; y++){
        for( int x=0; x<MAP_WIDTH; x++){
            if(buffer_gameboard[x][y] == 0){
                std::cout << ' ';
            }else if(buffer_gameboard[x][y] == 1){
                std::cout << '#';
            }else if(buffer_gameboard[x][y] == 2){
                std::cout << 'X';
            }else if(buffer_gameboard[x][y] == 3){
                std::cout << '>';
            }else if(buffer_gameboard[x][y] == 4){
                std::cout << '@';
            }
        }
        std::cout << std::endl;
    }
};


int main( int argc, char* argv[])
{
    while(GAMEACTIVE)
    {
        collisions();
        physics();
        input();
        render();
        
        auto now = std::chrono::system_clock::now();
        std::this_thread::sleep_until( now + std::chrono::milliseconds(3) );
    }
    return 0;
};