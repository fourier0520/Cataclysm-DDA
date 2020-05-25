
#include "multiplay_manager.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <thread>
#include <utility>
#include <winerror.h>
#include <unordered_map>
#include <mutex>

#include "output.h"
#include "debug.h"
#include "messages.h"
#include "string_formatter.h"
#include "game.h"
#include "avatar.h"
#include "monster.h"

static const efftype_id effect_pet( "pet" );

namespace
{
    std::unordered_map<std::string, client_command> command_map;
    std::mutex map_mtx;
}

int multiplay_manager::start_server(){

    is_server_active = false;

//    for( int i = 0 ; i < CLIENT_ARY_LENGTH ; i++){
//        client_ary[ i ].initialize();
//    }

    if( is_server_active ) {
        popup( _("server is already started."));
    } else {

        //command_map.emplace( 42 ,"n_ope" );

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
        if( started ) {
            add_msg("someone connected to multiplayer.");

        } else {
            add_msg("someone connected, but connection pool is full.");

            std::string welcome_msg = "server connection pool is full, try again later.";
            send( connected_socket, welcome_msg.c_str(), welcome_msg.length(), 0);
            closesocket(connected_socket);
        }
    }
    popup( _("server stoped succesfully."));
    return;
}



int multiplay_manager::do_turn(){

    for(const auto iter : command_map ){
        if( iter.second.c_type == client_command_message ) {
            add_msg( m_debug, "client_name:%s, message:%s;", iter.second.client_name, iter.second.command_argument );

            // erase_command(iter.first);
        } else if( iter.second.c_type == client_command_spawn ) {
            // add_msg( iter.second.command_argument );

            monster *mon = g->place_critter_around(
                    mtype_id( iter.second.command_argument ), g->u.pos() , 3);
            if( mon != nullptr ) {
                mon->multiplay_client_name = iter.second.client_name;
                mon->no_extra_death_drops = true;
                mon->unique_name = iter.second.client_name;
                mon->add_effect( effect_pet, 1_turns, num_bp, true );
                mon->friendly = -1;
            }
            erase_command(iter.first);
        }
    }

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


void multiplay_manager::insert_command(std::string key, client_command c_command) {
    std::lock_guard<std::mutex> lock(map_mtx);
    const auto iter = command_map.find( key );
    if( iter != command_map.end() ) {
        command_map.erase(iter);
    }
    command_map.emplace( key, c_command );
    return;
}

client_command multiplay_manager::find_command(std::string key) {
    std::lock_guard<std::mutex> lock(map_mtx);
    const auto iter = command_map.find( key );
    if( iter == command_map.end() ) {
        client_command nop( "_noname_", client_command_nop ,"nop" );
        return nop;
    }
    return iter->second;
}

void multiplay_manager::erase_command(std::string key) {
    std::lock_guard<std::mutex> lock(map_mtx);
    command_map.erase(key);
}
