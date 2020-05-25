

#include "multiplay_manager.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <thread>
#include <utility>
#include <winerror.h>
#include <unordered_map>

#include "output.h"
#include "debug.h"
#include "messages.h"
#include "string_formatter.h"
#include "game.h"
#include "rng.h"



bool multiplay_client::is_working() {
    return is_working_value;
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

    std::string send_msg_string = "Welcome to cdda!\r\n";
    send( sock, send_msg_string.c_str(), send_msg_string.length(), 0);

    send_msg_string = "Enter your name! ( Text cannot delete, please input carefully. )\r\n > ";
    send( sock, send_msg_string.c_str(), send_msg_string.length(), 0);

    std::string client_name = "";

    memset(recv_buffer, 0, sizeof(recv_buffer));

    // FIXME random id. maybe generate already exist id.
    int client_id = rng(10001,99999);

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
        client_name = string_format("Tom%d", client_id );
    }

    send_msg_string = "\r\n\r\n";
    send( sock, send_msg_string.c_str(), send_msg_string.length(), 0);


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

        client_command new_cmd( 0, client_command_msg ,message_string );

        g->multiplay_manager_ref.insert_command( client_name, new_cmd );

        memset(recv_buffer, 0, sizeof(recv_buffer));
    }

    client_work_finish_cleanup();
    return;
}
