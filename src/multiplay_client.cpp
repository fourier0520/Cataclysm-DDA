

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
#include "avatar.h"
#include "monster.h"


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

    std::string send_msg_string = "Welcome to cdda multiplayer!\r\n";
    send( sock, send_msg_string.c_str(), send_msg_string.length(), 0);

    send_msg_string = string_format( "Player name is '%s'.\r\n" , g->u.name );
    send( sock, send_msg_string.c_str(), send_msg_string.length(), 0);

    send_msg_string = "Enter your name! ( Text cannot delete, please input carefully. )\r\n >";
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

        std::string recv_str = recv_buffer;
        client_name.append( ( recv_str ));

        memset(recv_buffer, 0, sizeof(recv_buffer));
    }

    if( client_name == "") {
        client_name = string_format("Tom%d", client_id );
    }

    send_msg_string = string_format( "\r\nYour name is '%s'.\r\n" , client_name );
    send( sock, send_msg_string.c_str(), send_msg_string.length(), 0);

    send_msg_string = "\r\nHit [?] to help.\r\n";
    send( sock, send_msg_string.c_str(), send_msg_string.length(), 0);


    memset(recv_buffer, 0, sizeof(recv_buffer));

    client_mode mode = mode_default;

    std::string input_str = "";

    while( true ) {
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

        if( mode == mode_quit ) {
            if( recv_buffer[0] == 'y' ) {
                send_msg_string = "\r\nbye!\r\n";
                send( sock, send_msg_string.c_str(), send_msg_string.length(), 0);
                break;
            } else {
                mode = mode_default;
            }
        } else if( mode == mode_spawn ) {
            if( recv_buffer[0] == '\x0a' || recv_buffer[0] == '\x0d' ) {
                if( input_str == "" ) {
                    input_str = "mon_multiplayer";
                }
                mtype_id new_mtype_id(input_str);
                if ( new_mtype_id.is_valid() ) {
                    send_msg_string = string_format("\r\n'%s' will be spawn, you can control them with number key.\r\n >", input_str);
                    send( sock, send_msg_string.c_str(), send_msg_string.length(), 0);

                    std::string message_string( string_format( "%s: %s", client_name ,recv_buffer) );

                    client_command new_cmd( client_name, client_command_spawn , input_str );

                    g->multiplay_manager_ref.insert_command( client_name, new_cmd );

                } else {
                    send_msg_string = string_format("\r\n'%s' is no valid monster id.\r\n >", input_str);
                    send( sock, send_msg_string.c_str(), send_msg_string.length(), 0);
                }
                mode = mode_default;
            } else {
                std::string recv_str = recv_buffer;
                input_str.append( recv_str );
                send(sock, recv_buffer, sizeof(recv_buffer), 0);
            }
        } else if ( mode == mode_message ){

        } else {
            // default
            if( recv_buffer[0] == 'q' ) {
                send_msg_string = "\r\nreally quit? (y)\r\n";
                send( sock, send_msg_string.c_str(), send_msg_string.length(), 0);

                mode = mode_quit;
            } else if( recv_buffer[0] == 's' ) {
                send_msg_string = "\r\nEnter monster id to spawn as you, blank is default monster, '.'(period) to cancel. \r\n >";
                send( sock, send_msg_string.c_str(), send_msg_string.length(), 0);
                input_str = "";
                mode = mode_spawn;
            } else if( recv_buffer[0] == '2' ) {
                add_msg( m_debug, "two");
                client_command new_cmd( client_name, client_command_move , "S" );
                g->multiplay_manager_ref.insert_command( client_name, new_cmd );
            } else if( recv_buffer[0] == '4' ) {
                client_command new_cmd( client_name, client_command_move , "W" );
                g->multiplay_manager_ref.insert_command( client_name, new_cmd );
            } else if( recv_buffer[0] == '6' ) {
                client_command new_cmd( client_name, client_command_move , "E" );
                g->multiplay_manager_ref.insert_command( client_name, new_cmd );
            } else if( recv_buffer[0] == '8' ) {
                client_command new_cmd( client_name, client_command_move , "N" );
                g->multiplay_manager_ref.insert_command( client_name, new_cmd );
            } else if( recv_buffer[0] == '1' ) {
                client_command new_cmd( client_name, client_command_move , "SW" );
                g->multiplay_manager_ref.insert_command( client_name, new_cmd );
            } else if( recv_buffer[0] == '3' ) {
                client_command new_cmd( client_name, client_command_move , "SE" );
                g->multiplay_manager_ref.insert_command( client_name, new_cmd );
            } else if( recv_buffer[0] == '5' ) {

            } else if( recv_buffer[0] == '7' ) {
                client_command new_cmd( client_name, client_command_move , "NW" );
                g->multiplay_manager_ref.insert_command( client_name, new_cmd );
            } else if( recv_buffer[0] == '9' ) {
                client_command new_cmd( client_name, client_command_move , "NE" );
                g->multiplay_manager_ref.insert_command( client_name, new_cmd );
            } else if( recv_buffer[0] == '?' ) {
                send_msg_string = "\r\n-[1-9]: Move your monster.\r\n-[s]: Spawn monster as you.\r\n-[m]: Say message from your monster.\r\n-[k]: Kill your monster .\r\n-[q]: quit. \r\n >";
                send( sock, send_msg_string.c_str(), send_msg_string.length(), 0);
            }
        }

        memset(recv_buffer, 0, sizeof(recv_buffer));
    }

    client_work_finish_cleanup();
    return;
}
