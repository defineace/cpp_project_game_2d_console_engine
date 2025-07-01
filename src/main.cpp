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

class Server
{
private:
    std::vector<std::string> SERVER_MESSAGES;
    std::vector<float> PLAYER_POSTION = {130.0,10.0};

    void run(SOCKET serverSocket){

        // ######################################################################################
        // Client Handle
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

            // First: name
            std::getline(ss, name, ',');

            // Second: x
            std::getline(ss, token, ',');
            x = std::stof(token);

            // Third: y
            std::getline(ss, token, ',');
            y = std::stof(token);

            if(name=="quit")
                break;
            PLAYER_POSTION[0] = x;
            PLAYER_POSTION[1] = y;
        }

        closesocket(clientSocket);
        SERVER_MESSAGES.push_back("CLIENT::DISCONNECTED");
        
        // ######################################################################################
        // End of Client Handle
        // ######################################################################################

        closesocket(serverSocket);
        WSACleanup();
        SERVER_MESSAGES.push_back("SERVER::SERVER_ENDED");
    };
    
public:
    Server(){
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

        std::thread thread_run(&Server::run,this,serverSocket);
        thread_run.detach();
    };

    std::vector<std::string> return_server_messages(){return SERVER_MESSAGES;};
    std::vector<float> return_player_position(){return PLAYER_POSTION;};
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

    void detect_collision_wall_stop(Window* handle_window){
        float height = GAMEOBJECT->return_gameobject_sprite().size()-1;
        float width = GAMEOBJECT->return_gameobject_sprite()[0].length()-1;
        float padding = 1;

        if( GAMEOBJECT->return_float_gameobject_xpos()+width >= handle_window->return_int_window_width()-padding)
            GAMEOBJECT->set_gameobject_xpos(handle_window->return_int_window_width()-padding-width);
        if( GAMEOBJECT->return_float_gameobject_xpos() <= padding)
            GAMEOBJECT->set_gameobject_xpos(padding);
        if( GAMEOBJECT->return_float_gameobject_ypos()+height >= handle_window->return_int_window_height()-2)
            GAMEOBJECT->set_gameobject_ypos(handle_window->return_int_window_height()-padding-height);
        if( GAMEOBJECT->return_float_gameobject_ypos() <= padding)
            GAMEOBJECT->set_gameobject_ypos(padding);
    };

    void detect_collision_wall_bounce(Window* handle_window){

        float height = GAMEOBJECT->return_gameobject_sprite().size()-1;
        float width = GAMEOBJECT->return_gameobject_sprite()[0].length()-1;
        float padding = 1;

        if( GAMEOBJECT->return_float_gameobject_xpos()+width >= handle_window->return_int_window_width()-padding)
            GAMEOBJECT->set_gameobject_velocity_xaxis(GAMEOBJECT->return_float_gameobject_velocity_xaxis()*-1);
        if( GAMEOBJECT->return_float_gameobject_xpos() <= padding)
            GAMEOBJECT->set_gameobject_velocity_xaxis(GAMEOBJECT->return_float_gameobject_velocity_xaxis()*-1);
        if( GAMEOBJECT->return_float_gameobject_ypos()+height >= handle_window->return_int_window_height()-2)
            GAMEOBJECT->set_gameobject_velocity_yaxis(GAMEOBJECT->return_float_gameobject_velocity_yaxis()*-1);
        if( GAMEOBJECT->return_float_gameobject_ypos() <= padding)
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

class User
{
private:
    bool PRESSED = !PRESSED;

public:
    bool quit(){
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

class Game
{
private:
    bool GAMEACTIVE = true;
    Window HANDLE_WINDOW;
    User USER;

public:
    Game(): HANDLE_WINDOW(150,30){};

    void run(){

        mainMenu();
 
        HANDLE_WINDOW.buffer_clear();
        HANDLE_WINDOW.drawBorder('#');
        HANDLE_WINDOW.drawText(65,10,"GAME OVER");
        HANDLE_WINDOW.render();
    };

    void mainMenu(){
        std::string path_title = "./assets/sprite_mainmenu_title.txt";
        Sprite sprite_title(path_title);
        std::string path_options = "./assets/sprite_mainmenu_options.txt";
        Sprite sprite_options(path_options);

        User user;
        
        int selection = 0;

        // ##############################################################################
        // Start MainMenu Loop
        // ##############################################################################
        while(GAMEACTIVE){
            // Input
            if(user.quit()==1){
                GAMEACTIVE=!GAMEACTIVE;
            }

            if(user.mainMenu_scroll() == 11){
                if(selection<=0)
                    selection=0;
                else
                    selection = selection - 1;
            }
            if(user.mainMenu_scroll() == 10){
                if(selection>=3)
                    selection=3;
                else
                    selection = selection + 1;
            }
            if(user.mainMenu_select()==1){
                if(selection == 0){gameLoop();}
                if(selection == 1){}
                if(selection == 2){}
                if(selection == 3){
                    GAMEACTIVE = !GAMEACTIVE;
                }
            };
            
            // Render

            std::vector<std::string> render_options;
            
            if(selection == 0){
                render_options.push_back(sprite_options.return_sprite()[0]);
                render_options.push_back(sprite_options.return_sprite()[3]);
                render_options.push_back(sprite_options.return_sprite()[5]);
                render_options.push_back(sprite_options.return_sprite()[7]);
            }
            if(selection == 1){
                render_options.push_back(sprite_options.return_sprite()[1]);
                render_options.push_back(sprite_options.return_sprite()[2]);
                render_options.push_back(sprite_options.return_sprite()[5]);
                render_options.push_back(sprite_options.return_sprite()[7]);
            }
            if(selection == 2){
                render_options.push_back(sprite_options.return_sprite()[1]);
                render_options.push_back(sprite_options.return_sprite()[3]);
                render_options.push_back(sprite_options.return_sprite()[4]);
                render_options.push_back(sprite_options.return_sprite()[7]);
            }
            if(selection == 3){
                render_options.push_back(sprite_options.return_sprite()[1]);
                render_options.push_back(sprite_options.return_sprite()[3]);
                render_options.push_back(sprite_options.return_sprite()[5]);
                render_options.push_back(sprite_options.return_sprite()[6]);
            }
            
            HANDLE_WINDOW.buffer_clear();
            HANDLE_WINDOW.drawBorder('#');
            HANDLE_WINDOW.drawSprite(int(HANDLE_WINDOW.return_int_window_width()/2.8),2,sprite_title.return_sprite());
            HANDLE_WINDOW.drawSprite(int(HANDLE_WINDOW.return_int_window_width()/2.5),10,render_options);
            HANDLE_WINDOW.buffer_swap();
            HANDLE_WINDOW.render();
        }
        // ##############################################################################
        // End MainMenu Loop
        // ##############################################################################
    };
    
    void gameLoop(){
        Sprite sprite_player("./assets/sprite_player.txt");
        Sprite sprite_ball("./assets/sprite_ball.txt");
        
        GameObject player_1("player_1",20.0f,10.0f,sprite_player.return_sprite());
        GameObject player_2("player_2",130.0f,10.0f,sprite_player.return_sprite());
        GameObject ball("ball",75.0f,10.0f,sprite_ball.return_sprite());

        player_1.set_gameobject_velocity_yaxis(0.25);
        player_2.set_gameobject_velocity_yaxis(-0.25);
        ball.set_gameobject_velocity_xaxis(0.25);
        ball.set_gameobject_velocity_yaxis(0.25);

        Physics player1_physics(&player_1);
        Physics player2_physics(&player_2);
        Physics ball_physics(&ball);

        Collision player1_collision(&player_1);
        Collision player2_collision(&player_2);
        Collision ball_collision(&ball);
        
        // ######################################################################################
        // Main Loop
        // ######################################################################################
        while(GAMEACTIVE)
        {   
            //Framerate Limiter
            auto now = std::chrono::system_clock::now();
            std::this_thread::sleep_until( now + std::chrono::milliseconds(3) );

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

            // Input
            if(USER.quit()==1){GAMEACTIVE=!GAMEACTIVE;}

            // Render
            HANDLE_WINDOW.buffer_clear();

            HANDLE_WINDOW.drawBorder('#');
            HANDLE_WINDOW.drawSprite(player_1.return_int_gameobject_xpos(),player_1.return_int_gameobject_ypos(),player_1.return_gameobject_sprite());
            HANDLE_WINDOW.drawSprite(player_2.return_int_gameobject_xpos(),player_2.return_int_gameobject_ypos(),player_2.return_gameobject_sprite());
            HANDLE_WINDOW.drawSprite(ball.return_int_gameobject_xpos(),ball.return_int_gameobject_ypos(),ball.return_gameobject_sprite());
            HANDLE_WINDOW.render();
        }
        // ######################################################################################
        // End of Main Loop
        // ######################################################################################
    };
};

int main(int argc,char* argv[])
{
    Game game;
    game.run();
    return 0;
};