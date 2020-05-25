
#include "multiplay_manager.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <thread>
#include <utility>
#include <winerror.h>

#include "output.h"
#include "debug.h"
#include "messages.h"
#include "string_formatter.h"


int multiplay_manager::start_server(){

    is_server_active = false;

//    for( int i = 0 ; i < CLIENT_ARY_LENGTH ; i++){
//        client_ary[ i ].initialize();
//    }

    if( is_server_active ) {
        popup( _("server is already started."));
    } else {
        static const int WSA_VERSION_MAJOR = 2;
        static const int WSA_VERSION_MINOR = 0;
        WSADATA wsaData;
        WSAStartup(MAKEWORD( WSA_VERSION_MAJOR, WSA_VERSION_MINOR ), &wsaData);

        is_server_active = true;

        server_thread = std::thread([this]() { this->server_thread_process(); } );

        popup( _("server started."));

    }
    return 0;

}
int multiplay_manager::stop_server(){

    if( is_server_active ) {
        server_thread.detach();

        is_server_active = false;

        WSACleanup();
    } else {
        popup( _("server is already started."));

    }
    return 0;
}

void multiplay_manager::server_thread_process(){

    SOCKET server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if( server_sock == INVALID_SOCKET ){
        debugmsg("socket creation error! %d", WSAGetLastError());
        stop_server();
        return;
    }

    sockaddr_in addr_in;
    memset(&addr_in, 0, sizeof(struct sockaddr_in));
    addr_in.sin_family = AF_INET;
    addr_in.sin_port = htons(4454);
    addr_in.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr_in.sin_addr.s_addr = INADDR_ANY;

    sockaddr *addr_ptr = reinterpret_cast<sockaddr *>(&addr_in);

    if( bind(server_sock, addr_ptr, sizeof(addr_in)) != 0 ) {
        debugmsg("socket bind error! :%d",  WSAGetLastError());
        stop_server();
        return;
    }

    static const int CLIENT_ARY_LENGTH = 10;
    multiplay_client client_ary[CLIENT_ARY_LENGTH];

    while( is_server_active ){
        static const int SOCKET_QUEUE_MAX = 5;
        if( listen(server_sock, SOCKET_QUEUE_MAX) != 0 ){
            debugmsg("socket listen error! : %d",  WSAGetLastError());
            stop_server();
            return;
        }

        sockaddr_in get_addr_in;
        socklen_t len = sizeof(struct sockaddr_in);
        sockaddr *get_addr_ptr = reinterpret_cast<sockaddr *>(&get_addr_in);

        SOCKET connected_socket = accept(server_sock, get_addr_ptr, &len);

        if( connected_socket == INVALID_SOCKET ){
            debugmsg("socket accept error! :%d",  WSAGetLastError());
            stop_server();
            return;
        }


        bool started = false;
        for( int i = 0 ; i < CLIENT_ARY_LENGTH ; i++){
            if ( !client_ary[ i ].is_working() ) {
                client_ary[ i ].start_client_thread( connected_socket );
                started = true;
                break;
            }
        }
        if( !started ) {
            add_msg("someone connected, but connection pool is full.");

            std::string welcome_msg = "server connection pool is full, try again later.";
            send( connected_socket, welcome_msg.c_str(), welcome_msg.length(), 0);
            closesocket(connected_socket);
        }
    }
    popup( _("server stoped succesfully."));
    return;
}

multiplay_client::multiplay_client() {
    client_thread = std::thread();
    is_working_value = false;
}

void multiplay_client::start_client_thread( SOCKET connected_socket ) {
    is_working_value = true;
    sock = connected_socket;
    client_thread = std::thread([this]() { this->client_thread_process(); } );

}

void multiplay_client::client_work_finish_cleanup(){
    closesocket(sock);
    client_thread.detach();
    is_working_value = false;
}

void multiplay_client::client_thread_process(){

    // const char telnet_command_iac_do_echo[] = {'\xFF','\xFD','\x01','\x00' };
    // send( sock, telnet_command_iac_do_echo, sizeof(telnet_command_iac_do_echo), 0);
    int recv_result = 0;

    std::string welcome_msg = "Welcome to cdda!\r\n";
    send( sock, welcome_msg.c_str(), welcome_msg.length(), 0);

    welcome_msg = "Enter your name! ( Text cannot delete, please input carefully. )\r\n > ";
    send( sock, welcome_msg.c_str(), welcome_msg.length(), 0);

    std::string client_name = "";

    memset(recv_buffer, 0, sizeof(recv_buffer));
    while( true ) {
        // loop until hit enter key
        recv_result = recv(sock, recv_buffer, sizeof(recv_buffer), 0);

        if( recv_result < 0 ){
            client_work_finish_cleanup();
            return;
        }

        recv_buffer[ sizeof(recv_buffer) - 1 ] = 0;
        send(sock, recv_buffer, sizeof(recv_buffer), 0);

        if( recv_buffer[0] == '\x0a' || recv_buffer[0] == '\x0d' ) {
            break;
        }

        std::string input_str = recv_buffer;
        client_name.append( ( input_str ));

        memset(recv_buffer, 0, sizeof(recv_buffer));
    }

    if( client_name == "") {
        client_name = "Tom";
    }

    memset(recv_buffer, 0, sizeof(recv_buffer));
    while( recv_buffer[0] != 'q' ) {
        recv_result = recv(sock, recv_buffer, sizeof(recv_buffer), 0);
        if( recv_result < 0) {
            int errorCode = WSAGetLastError();
            static const int ERROR_DISCONNECT = 10053;
            if ( errorCode == ERROR_DISCONNECT){

            } else {
                debugmsg("socket recv error! : %d", errorCode);
            }
            client_work_finish_cleanup();
            return;
        }
        recv_buffer[ sizeof(recv_buffer) - 1 ] = 0;
        send(sock, recv_buffer, sizeof(recv_buffer), 0);

        std::string message_string( string_format( "%s: %s", client_name ,recv_buffer) );
        add_msg( message_string );

        memset(recv_buffer, 0, sizeof(recv_buffer));
    }

    client_work_finish_cleanup();
    return;
}

int multiplay_manager::do_turn(){
    return 0;
}

int multiplay_manager::multiplay_menu() {

    enum maid_choices {
        cancel = 0,
        server_start,
        server_stop,
    };
    uilist amenu;
    amenu.text = string_format( _( "Multiplay menu" ) );

    amenu.addentry( cancel, true, 'c', _( "Cancel" ) );
    amenu.addentry( server_start, true, 'S', _( "Server Start" ) );
    amenu.addentry( server_stop, true, 'T', _( "Server Stop" ) );

    amenu.query();
    int choice = amenu.ret;

    switch( choice ) {
    case cancel:
        break;
    case server_start:
        start_server();
        break;
    case server_stop:
        stop_server();
        break;
    default:
        break;
    }
    return 0;
}

bool multiplay_client::is_working() {
    return is_working_value;
}
