/*
#################################################################
TITLE:
CPP_TEMPLATE_GAME_2D_BASIC

DESCRIPTION:
- Basic 2D Game Engine
- Demo is Pong based

TODO:
- Need to add NetworkClient Class

#################################################################
*/
#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <vector>
#include <chrono>
#include <thread>
#include <fstream>

#include <winsock2.h>
#include <windows.h>

#pragma comment(lib,"ws2_32.lib")

class NetworkServer
{
private:
    std::vector<std::string> SERVER_MESSAGES;
    std::vector<float> PLAYER_1_POSTION = {20.0,10.0};
    std::vector<float> PLAYER_2_POSTION = {130.0,10.0};

    void run(SOCKET serverSocket){

        // ######################################################################################
        // Start Client Handle
        // ######################################################################################

        sockaddr_in clientAddress;
        int clientSize = sizeof(clientAddress);
        SOCKET clientSocket = accept( serverSocket, (sockaddr*)&clientAddress, &clientSize);
        SERVER_MESSAGES.push_back("CLIENT::CONNECTED");
        
        while(true){
            char buffer[1024] = {};
            recv( clientSocket, buffer, sizeof(buffer), 0);

            std::string data = buffer;
            SERVER_MESSAGES.push_back("CLIENT::" + data);

            std::stringstream ss(data);
            std::string token;

            std::string name;
            float x = 0.0f;
            float y = 0.0f;

            std::getline(ss, name, ',');

            std::getline(ss, token, ',');
            x = std::stof(token);

            std::getline(ss, token, ',');
            y = std::stof(token);

            if(name=="quit"){break;}
            if(name=="player_1"){set_server_player_position(0,x,y);}
            if(name=="player_2"){set_server_player_position(1,x,y);}
        }

        closesocket(clientSocket);
        SERVER_MESSAGES.push_back("CLIENT::DISCONNECTED");
        
        // ######################################################################################
        // End Client Handle
        // ######################################################################################

        closesocket(serverSocket);
        WSACleanup();
        SERVER_MESSAGES.push_back("SERVER::SERVER_ENDED");
    };

public:
    NetworkServer(){
        WSADATA wsaData;
        if(WSAStartup(MAKEWORD(2,2),&wsaData)!=0)
        {
            SERVER_MESSAGES.push_back("ERROR::WSA_STARTUP_FAILED");
        }

        SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);

        if(serverSocket == -1)
        {
            WSACleanup();
            SERVER_MESSAGES.push_back("ERROR::FAILED_TO_CREATE_SOCKET");
        }

        sockaddr_in serverAddress;
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(8080);
        serverAddress.sin_addr.s_addr = INADDR_ANY;
        
        if( bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR )
        {
            closesocket(serverSocket);
            WSACleanup();
            SERVER_MESSAGES.push_back("ERROR::FAILED_TO_BIND, Code: " + std::to_string(WSAGetLastError()));
        }
        
        if(listen(serverSocket, 5) == SOCKET_ERROR)
        {
            SERVER_MESSAGES.push_back("ERROR::FAILED_TO_LISTEN, Code: " + std::to_string(WSAGetLastError()));
            closesocket(serverSocket);
            WSACleanup();
        }

        SERVER_MESSAGES.push_back("SERVER::SERVER_STARTED");
        SERVER_MESSAGES.push_back("SERVER::AWAITING_CONNECTIONS");

        std::thread thread_run(&NetworkServer::run,this,serverSocket);
        thread_run.detach();
    };

    void set_server_player_position(int selectPlayer, float xPos, float yPos){
        if(selectPlayer == 0){
            PLAYER_1_POSTION[0] = xPos;
            PLAYER_1_POSTION[1] = yPos;
        }else if(selectPlayer == 1){
            PLAYER_2_POSTION[0] = xPos;
            PLAYER_2_POSTION[1] = yPos;
        }
    };

    std::vector<float> return_server_player_position(int selectPlayer){
        if(selectPlayer==0){return PLAYER_1_POSTION;}
        else if(selectPlayer==1){return PLAYER_2_POSTION;}
    };

    std::vector<std::string> return_server_messages(){return SERVER_MESSAGES;};
};

class Window
{
private:
    int WINDOW_WIDTH;
    int WINDOW_HEIGHT;
    
    std::vector<std::string> BUFFER_FRONT;
    std::vector<std::string> BUFFER_BACK;

    bool BUFFERSWAP = true;

public:
    Window(int width,int height){
        WINDOW_WIDTH = width;
        WINDOW_HEIGHT = height;
        
        for(int y=0;y<WINDOW_HEIGHT;y++){
            std::string line(WINDOW_WIDTH,' ');
            BUFFER_FRONT.push_back(line);
            BUFFER_BACK.push_back(line);
        }
    };

    ~Window(){
        BUFFER_FRONT.clear();
        BUFFER_BACK.clear();
    };

    void draw(int xPos,int yPos,char character){
        if(BUFFERSWAP)             
            BUFFER_BACK[yPos][xPos] = character;
        else
            BUFFER_FRONT[yPos][xPos] = character;
    };

    void drawSprite(int xPos,int yPos,std::vector<std::string> sprite){
        for (int y=0;y<sprite.size();y++) {
            for (int x=0;x<sprite[0].size() ;x++) {
                if(BUFFERSWAP)
                    BUFFER_BACK[yPos+y][xPos+x] = sprite[y][x];
                else
                    BUFFER_FRONT[yPos+y][xPos+x] = sprite[y][x];
            }
        }
    };
    
    void drawText(int xPos,int yPos,std::string text){
        for(int i=0;i<text.length();i++){
            if(BUFFERSWAP)
                BUFFER_BACK[yPos][xPos+i] = text[i];
            else
                BUFFER_FRONT[yPos][xPos+i] = text[i];
        }
    };

    void drawBorder(char character){
        for( int y=0; y<WINDOW_HEIGHT; y++){
            for( int x=0; x<WINDOW_WIDTH; x++){
                if( x==0 || x==WINDOW_WIDTH-1 || y==0 || y==WINDOW_HEIGHT-1)
                    if(BUFFERSWAP)             
                        BUFFER_BACK[y][x] = character;
                    else
                        BUFFER_FRONT[y][x] = character;
            }
        }
    };

    void buffer_clear(){
        for(int y=0;y<WINDOW_HEIGHT;y++){
            for(int x=0;x<WINDOW_WIDTH;x++){
                if(BUFFERSWAP)
                    BUFFER_BACK[y][x] = ' ';
                else
                    BUFFER_FRONT[y][x] = ' ';
            }
        }
    }

    void buffer_swap(){ BUFFERSWAP = !BUFFERSWAP; };

    void render(){
        HANDLE handle_console = GetStdHandle( STD_OUTPUT_HANDLE );
        CONSOLE_CURSOR_INFO cursorInfo;
        cursorInfo.dwSize = 100;
        cursorInfo.bVisible = FALSE;
        SetConsoleCursorInfo(handle_console,&cursorInfo);

        COORD coord = {0,0};
        SetConsoleCursorPosition(handle_console,coord);

        for(int y=0;y<WINDOW_HEIGHT;y++){
            for(int x=0;x<WINDOW_WIDTH;x++){
                if(BUFFERSWAP)
                    std::cout << BUFFER_BACK[y][x];
                else
                    std::cout << BUFFER_FRONT[y][x];
            }
            std::cout << std::endl;
        }
        std::cout << std::flush;
    };

    int return_int_window_width(){ return WINDOW_WIDTH; };
    int return_int_window_height(){ return WINDOW_HEIGHT; };
};

class Sprite
{
private:
    std::string FILE_PATH;
    std::vector<std::string> SPRITE;

public:
    Sprite(std::string path){
        FILE_PATH = path;

        std::ifstream file_read(path);
        
        if(file_read.is_open()){
            std::string line;

            while(std::getline(file_read, line)){
                SPRITE.push_back(line);
            }
        }
        file_read.close();
    };

    std::vector<std::string> return_sprite(){return SPRITE;};
};

class GameObject
{
private:
    std::string OBJECT_LABEL;
    float OBJECT_XPOS;
    float OBJECT_YPOS;
    std::vector<std::string> OBJECT_SPRITE;

    float OBJECT_VELOCITY_XAXIS = 0;
    float OBJECT_VELOCITY_YAXIS = 0;
public:
    GameObject(std::string label,float xPos,float yPos,std::vector<std::string> sprite){
        OBJECT_LABEL = label;
        OBJECT_XPOS = xPos;
        OBJECT_YPOS = yPos;
        OBJECT_SPRITE = sprite;
    };

    float return_float_gameobject_xpos(){ return OBJECT_XPOS; };

    float return_float_gameobject_ypos(){ return OBJECT_YPOS; };

    void set_gameobject_xpos(float xPos){ OBJECT_XPOS = xPos; };

    void set_gameobject_ypos(float yPos){ OBJECT_YPOS = yPos; };

    int return_int_gameobject_xpos(){ return int(OBJECT_XPOS); };

    int return_int_gameobject_ypos(){ return int(OBJECT_YPOS); };

    void set_gameobject_velocity_xaxis(float force){ OBJECT_VELOCITY_XAXIS = force; };

    void set_gameobject_velocity_yaxis(float force){ OBJECT_VELOCITY_YAXIS = force; };

    float return_float_gameobject_velocity_xaxis(){ return OBJECT_VELOCITY_XAXIS; };

    float return_float_gameobject_velocity_yaxis(){ return OBJECT_VELOCITY_YAXIS; };

    void set_gameobject_sprite(std::vector<std::string> sprite){ OBJECT_SPRITE = sprite; };

    std::vector<std::string> return_gameobject_sprite(){ return OBJECT_SPRITE; };
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

public:
    Collision(GameObject* gameObject){
        GAMEOBJECT = gameObject;
    };

    int detect_collision_wall_identify(Window* handle_window){
        float height = GAMEOBJECT->return_gameobject_sprite().size()-1;
        float width = GAMEOBJECT->return_gameobject_sprite()[0].length()-1;
        float padding = 1;

        if( GAMEOBJECT->return_float_gameobject_xpos()+width >= handle_window->return_int_window_width()-padding)
            return 11; // Right Wall
        if( GAMEOBJECT->return_float_gameobject_xpos() <= padding)
            return 12; // Left Wall
        if( GAMEOBJECT->return_float_gameobject_ypos()+height >= handle_window->return_int_window_height()-2)
            return 13; // Bottom Wall
        if( GAMEOBJECT->return_float_gameobject_ypos() <= padding)
            return 14; // Top Wall
        else
            return 10;
    };

    void detect_collision_wall_stop(Window* handle_window){
        float height = GAMEOBJECT->return_gameobject_sprite().size()-1;
        float width = GAMEOBJECT->return_gameobject_sprite()[0].length()-1;
        float padding = 1;

        if( GAMEOBJECT->return_float_gameobject_xpos()+width >= handle_window->return_int_window_width()-padding) // Right Wall
            GAMEOBJECT->set_gameobject_xpos(handle_window->return_int_window_width()-padding-width);
        if( GAMEOBJECT->return_float_gameobject_xpos() <= padding) // Left Wall
            GAMEOBJECT->set_gameobject_xpos(padding);
        if( GAMEOBJECT->return_float_gameobject_ypos()+height >= handle_window->return_int_window_height()-2) // Bottom Wall
            GAMEOBJECT->set_gameobject_ypos(handle_window->return_int_window_height()-padding-height);
        if( GAMEOBJECT->return_float_gameobject_ypos() <= padding) // Top Wall
            GAMEOBJECT->set_gameobject_ypos(padding+1);
    };

    void detect_collision_wall_bounce(Window* handle_window){

        float height = GAMEOBJECT->return_gameobject_sprite().size()-1;
        float width = GAMEOBJECT->return_gameobject_sprite()[0].length()-1;
        float padding = 1;

        if( GAMEOBJECT->return_float_gameobject_xpos()+width >= handle_window->return_int_window_width()-padding) // Right Wall
            GAMEOBJECT->set_gameobject_velocity_xaxis(GAMEOBJECT->return_float_gameobject_velocity_xaxis()*-1);
        if( GAMEOBJECT->return_float_gameobject_xpos() <= padding) // Left Wall
            GAMEOBJECT->set_gameobject_velocity_xaxis(GAMEOBJECT->return_float_gameobject_velocity_xaxis()*-1);
        if( GAMEOBJECT->return_float_gameobject_ypos()+height >= handle_window->return_int_window_height()-2) // Bottom Wall
            GAMEOBJECT->set_gameobject_velocity_yaxis(GAMEOBJECT->return_float_gameobject_velocity_yaxis()*-1);
        if( GAMEOBJECT->return_float_gameobject_ypos() <= padding+1) // Top Wall
            GAMEOBJECT->set_gameobject_velocity_yaxis(GAMEOBJECT->return_float_gameobject_velocity_yaxis()*-1);
    };

    void detect_collision_player_bounce(GameObject* player){
        float height = GAMEOBJECT->return_gameobject_sprite().size()-1;
        float width = GAMEOBJECT->return_gameobject_sprite()[0].length()-1;

        if( 
            GAMEOBJECT->return_float_gameobject_xpos() >= player->return_int_gameobject_xpos() &&
            GAMEOBJECT->return_float_gameobject_xpos()+width <= player->return_float_gameobject_xpos() + player->return_gameobject_sprite()[0].length() &&
            GAMEOBJECT->return_float_gameobject_ypos() >= player->return_float_gameobject_ypos() &&
            GAMEOBJECT->return_float_gameobject_ypos()+height <= player->return_float_gameobject_ypos() + player->return_gameobject_sprite().size()
        ){
            GAMEOBJECT->set_gameobject_velocity_xaxis(GAMEOBJECT->return_float_gameobject_velocity_xaxis()*-1);
        }
    };
};

class UserInput
{
private:
    bool PRESSED = !PRESSED;

public:
    bool quit(){
        auto now = std::chrono::system_clock::now();
        std::this_thread::sleep_until( now + std::chrono::milliseconds(25) );

        if(GetAsyncKeyState(VK_ESCAPE) & 0x8000)
            return true;
        else
            return false;
    };

    bool mainMenu_select(){
        if(GetAsyncKeyState(VK_SPACE) &0x8000)
            return true;
        else
            return false;
    };

    int mainMenu_scroll(){
        auto now = std::chrono::system_clock::now();
        std::this_thread::sleep_until( now + std::chrono::milliseconds(25) );

        if(GetAsyncKeyState(VK_UP) & 0x8000){
            return 11;
        }else if(GetAsyncKeyState(VK_DOWN) & 0x8000){
            return 10;
        }else{
            return 5;
        }
    };
};

class Menu
{
private:
    Window* HANDLE_WINDOW;
    UserInput* HANDLE_USER_INPUT;

    Sprite* SPRITE_TITLE;
    Sprite* SPRITE_OPTIONS;

    int selection = 0;

public:
    Menu(UserInput* handle_userInput,Window* handle_window,Sprite* sprite_title, Sprite* sprite_options)
    {
        HANDLE_USER_INPUT = handle_userInput;
        HANDLE_WINDOW = handle_window;

        SPRITE_TITLE = sprite_title;
        SPRITE_OPTIONS = sprite_options;
    };

    int run()
    {
        // ######################################################################################
        // Start Menu Loop
        // ######################################################################################
        while(true)
        {
            if(HANDLE_USER_INPUT->quit()==1){return 100;}
            if(HANDLE_USER_INPUT->mainMenu_scroll() == 11){
                if(selection>0)
                    selection--;
            }
            if(HANDLE_USER_INPUT->mainMenu_scroll() == 10){
                if(selection<3)
                    selection++;
            }
            if(HANDLE_USER_INPUT->mainMenu_select()==1){
                if(selection == 0){return 101;}
                if(selection == 1){return 102;}
                if(selection == 2){return 103;}
                if(selection == 3){return 104;}
            }
            
            std::vector<std::string> render_options;
            if(selection == 0){
                render_options.push_back(SPRITE_OPTIONS->return_sprite()[0]);
                render_options.push_back(SPRITE_OPTIONS->return_sprite()[3]);
                render_options.push_back(SPRITE_OPTIONS->return_sprite()[5]);
                render_options.push_back(SPRITE_OPTIONS->return_sprite()[7]);
            }
            if(selection == 1){
                render_options.push_back(SPRITE_OPTIONS->return_sprite()[1]);
                render_options.push_back(SPRITE_OPTIONS->return_sprite()[2]);
                render_options.push_back(SPRITE_OPTIONS->return_sprite()[5]);
                render_options.push_back(SPRITE_OPTIONS->return_sprite()[7]);
            }
            if(selection == 2){
                render_options.push_back(SPRITE_OPTIONS->return_sprite()[1]);
                render_options.push_back(SPRITE_OPTIONS->return_sprite()[3]);
                render_options.push_back(SPRITE_OPTIONS->return_sprite()[4]);
                render_options.push_back(SPRITE_OPTIONS->return_sprite()[7]);
            }
            if(selection == 3){
                render_options.push_back(SPRITE_OPTIONS->return_sprite()[1]);
                render_options.push_back(SPRITE_OPTIONS->return_sprite()[3]);
                render_options.push_back(SPRITE_OPTIONS->return_sprite()[5]);
                render_options.push_back(SPRITE_OPTIONS->return_sprite()[6]);
            }
           
            int margin = 5;
            HANDLE_WINDOW->buffer_clear();
            HANDLE_WINDOW->drawBorder('#');
            HANDLE_WINDOW->drawSprite(int(HANDLE_WINDOW->return_int_window_width()/2) - int(SPRITE_TITLE->return_sprite()[0].length()/2),margin,SPRITE_TITLE->return_sprite());
            HANDLE_WINDOW->drawSprite(int(HANDLE_WINDOW->return_int_window_width()/2) - int(SPRITE_OPTIONS->return_sprite()[0].length()/2),int(margin+SPRITE_TITLE->return_sprite().size()+3),render_options);
            HANDLE_WINDOW->buffer_swap();
            HANDLE_WINDOW->render();
        }
        // ######################################################################################
        // End Menu Loop
        // ######################################################################################
    };
    
};


class Game
{
private:
    bool GAMEACTIVE = true;
    Window HANDLE_WINDOW;
    UserInput HANDLE_USER_INPUT;

    int PLAYER_1_SCORE = 0;
    int PLAYER_2_SCORE = 0;

    float PLAYER_1_XPOS = 20.0f;
    float PLAYER_1_YPOS = 10.0f;
    float PLAYER_1_VELOCITY_YAXIS = 0.75f;

    float PLAYER_2_XPOS = 130.0f;
    float PLAYER_2_YPOS = 10.0f;
    float PLAYER_2_VELOCITY_YAXIS = -0.75f;

    float BALL_XPOS = 75.0f;
    float BALL_YPOS = 10.0f;
    float BALL_VELOCITY_XAXIS = 0.9f;
    float BALL_VELOCITY_YAXIS = 0.9f;

public:
    Game(): HANDLE_WINDOW(150,30){};

    void gameLoop(){
        Sprite sprite_player("./assets/sprite_player.txt");
        Sprite sprite_ball("./assets/sprite_ball.txt");
        
        GameObject player_1("player_1",PLAYER_1_XPOS,PLAYER_1_YPOS,sprite_player.return_sprite());
        GameObject player_2("player_2",PLAYER_2_XPOS,PLAYER_2_YPOS,sprite_player.return_sprite());
        GameObject ball("ball",BALL_XPOS,BALL_YPOS,sprite_ball.return_sprite());

        player_1.set_gameobject_velocity_yaxis(PLAYER_1_VELOCITY_YAXIS);
        player_2.set_gameobject_velocity_yaxis(PLAYER_2_VELOCITY_YAXIS);
        ball.set_gameobject_velocity_xaxis(BALL_VELOCITY_XAXIS);
        ball.set_gameobject_velocity_yaxis(BALL_VELOCITY_YAXIS);

        Physics player1_physics(&player_1);
        Physics player2_physics(&player_2);
        Physics ball_physics(&ball);

        Collision player1_collision(&player_1);
        Collision player2_collision(&player_2);
        Collision ball_collision(&ball);
        
        // ######################################################################################
        // Start Gameloop
        // ######################################################################################
        while(true)
        {   
            // Framerate Limiter
            auto now = std::chrono::system_clock::now();
            std::this_thread::sleep_until( now + std::chrono::milliseconds(3) );

            // Input
            if(HANDLE_USER_INPUT.quit()==1){break;}

            // Physics
            player1_physics.force_simple_y_axis(player_1.return_float_gameobject_velocity_yaxis());
            player2_physics.force_simple_y_axis(player_2.return_float_gameobject_velocity_yaxis());
            ball_physics.force_simple_x_axis(ball.return_float_gameobject_velocity_xaxis());
            ball_physics.force_simple_y_axis(ball.return_float_gameobject_velocity_yaxis());

            // Collision
            player1_collision.detect_collision_wall_bounce(&HANDLE_WINDOW);
            player2_collision.detect_collision_wall_bounce(&HANDLE_WINDOW);

            ball_collision.detect_collision_wall_bounce(&HANDLE_WINDOW);
            ball_collision.detect_collision_player_bounce(&player_1);
            ball_collision.detect_collision_player_bounce(&player_2);

            // Game Logic
            if(ball_collision.detect_collision_wall_identify(&HANDLE_WINDOW) == 12){PLAYER_2_SCORE++;}
            if(ball_collision.detect_collision_wall_identify(&HANDLE_WINDOW) == 11){PLAYER_1_SCORE++;}

            std::string scoreboard_player_1 = "Player 1: " + std::to_string(PLAYER_1_SCORE);
            std::string scoreboard_player_2 = "Player 2: " + std::to_string(PLAYER_2_SCORE);
            std::string score_line = scoreboard_player_1 + "   VS   " + scoreboard_player_2;

            // Render
            HANDLE_WINDOW.buffer_clear();
            HANDLE_WINDOW.drawBorder('#');
            HANDLE_WINDOW.drawSprite(player_1.return_int_gameobject_xpos(),player_1.return_int_gameobject_ypos(),player_1.return_gameobject_sprite());
            HANDLE_WINDOW.drawSprite(player_2.return_int_gameobject_xpos(),player_2.return_int_gameobject_ypos(),player_2.return_gameobject_sprite());
            HANDLE_WINDOW.drawSprite(ball.return_int_gameobject_xpos(),ball.return_int_gameobject_ypos(),ball.return_gameobject_sprite());
            HANDLE_WINDOW.drawText(int(HANDLE_WINDOW.return_int_window_width()/2)-(score_line.length()/2),10,score_line);
            HANDLE_WINDOW.render();
        }
        // ######################################################################################
        // End Gameloop
        // ######################################################################################
    };

    void play(){
        Sprite sprite_title("./assets/sprite_mainmenu_title.txt");
        Sprite sprite_options("./assets/sprite_mainmenu_options.txt");

        Menu mainmenu(&HANDLE_USER_INPUT, &HANDLE_WINDOW, &sprite_title, &sprite_options);

        // ######################################################################################
        // Start Main Loop
        // ######################################################################################
        while(GAMEACTIVE)
        {
            if(mainmenu.run() == 100){GAMEACTIVE = !GAMEACTIVE;}
            if(mainmenu.run() == 101){gameLoop();}
            if(mainmenu.run() == 102){}
            if(mainmenu.run() == 103){}
            if(mainmenu.run() == 104){GAMEACTIVE = !GAMEACTIVE;}
        }
        // ######################################################################################
        // End Main Loop
        // ######################################################################################

        // Closing Screen...
        HANDLE_WINDOW.buffer_clear();
        HANDLE_WINDOW.drawBorder('#');
        HANDLE_WINDOW.drawText(int(HANDLE_WINDOW.return_int_window_width()/2) - int(sizeof("GAME OVER")/2),5,"GAME OVER");
        HANDLE_WINDOW.render();
    };
};

int main(int argc,char* argv[])
{
    Game game;
    game.play();
    return 0;
};