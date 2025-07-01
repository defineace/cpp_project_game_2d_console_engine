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

public:
    GameObject(std::string label,float xPos,float yPos,std::vector<std::string> sprite){
        OBJECT_LABEL = label;
        OBJECT_XPOS = xPos;
        OBJECT_YPOS = yPos;
        OBJECT_SPRITE = sprite;
    };

    void set_gameobject_xpos(float xPos){ OBJECT_XPOS = xPos; };

    void set_gameobject_ypos(float yPos){ OBJECT_YPOS = yPos; };

    void set_gameobject_sprite(std::vector<std::string> sprite){ OBJECT_SPRITE = sprite; };

    float return_float_gameobject_xpos(){ return OBJECT_XPOS; };

    float return_float_gameobject_ypos(){ return OBJECT_YPOS; };

    int return_int_gameobject_xpos(){ return int(OBJECT_XPOS); };

    int return_int_gameobject_ypos(){ return int(OBJECT_YPOS); };

    std::vector<std::string> return_gameobject_sprite(){ return OBJECT_SPRITE; };
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
    
public:
    Game(): HANDLE_WINDOW(150,30){};
    void run(){
        Server server;
        Sprite sprite_player("./assets/sprite_player.txt");
        User user;

        GameObject player_1("player_1",20.0f,10.0f,sprite_player.return_sprite());
        GameObject player_2("player_2",130.0f,10.0f,sprite_player.return_sprite());

        // ######################################################################################
        // Main Loop
        // ######################################################################################
        while(GAMEACTIVE)
        {   
            //Framerate Limiter
            auto now = std::chrono::system_clock::now();
            std::this_thread::sleep_until( now + std::chrono::milliseconds(3) );

            // Movement
            player_2.set_gameobject_xpos(server.return_player_position()[0]);
            player_2.set_gameobject_ypos(server.return_player_position()[1]);
            
            // Input
            if(user.quit()==1){GAMEACTIVE=!GAMEACTIVE;}

            // Render
            HANDLE_WINDOW.buffer_clear();

            // for(int i=0;i<server.return_server_messages().size();i++){
            //     HANDLE_WINDOW.drawText(50,5+i,server.return_server_messages()[i]);
            // }

            HANDLE_WINDOW.drawBorder('#');
            HANDLE_WINDOW.drawSprite(player_1.return_int_gameobject_xpos(),player_1.return_int_gameobject_ypos(),player_1.return_gameobject_sprite());
            HANDLE_WINDOW.drawSprite(player_2.return_int_gameobject_xpos(),player_2.return_int_gameobject_ypos(),player_2.return_gameobject_sprite());
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