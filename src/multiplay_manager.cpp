
#include "multiplay_manager.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <thread>

#include "output.h"
#include "debug.h"


int multiplay_manager::start_server(){

    if( is_server_active ) {
        popup( _("server is already started."));
    } else {
        is_server_active = true;

        server_thread = std::thread([this]() { this->server_thread_process(); } );

        popup( _("server started succesfully."));

    }
    return 0;

}
int multiplay_manager::stop_server(){

    if( is_server_active ) {
        server_thread.detach();

        is_server_active = false;

    } else {
        popup( _("server is already started."));

    }
    return 0;
}

void multiplay_manager::server_thread_process(){

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        debugmsg("socket creation error!");
        stop_server();
        return;
    }

    sockaddr_in addr_in;
    memset(&addr_in, 0, sizeof(struct sockaddr_in));
    addr_in.sin_family = AF_INET;
    addr_in.sin_port = htons(4454);
    addr_in.sin_addr.s_addr = inet_addr("127.0.0.1");

    sockaddr *addr_ptr = reinterpret_cast<sockaddr *>(&addr_in);

    if(bind(sockfd, addr_ptr, sizeof(addr_in)) < 0) {
        debugmsg("socket bind error!");
        stop_server();
        return;
    }

    while( is_server_active ){
        static const int SOCKET_QUEUE_MAX = 5;
        if(listen(sockfd, SOCKET_QUEUE_MAX) < 0){
            debugmsg("socket listen error!");
            stop_server();
            return;
        }

        sockaddr_in get_addr_in;
        socklen_t len = sizeof(struct sockaddr_in);
        sockaddr *get_addr_ptr = reinterpret_cast<sockaddr *>(&get_addr_in);

        int connected_socket = accept(sockfd, get_addr_ptr, &len);

        if(connected_socket < 0){
            debugmsg("socket accept error!");
            stop_server();
            return;
        }

        multiplay_client new_client;
        new_client.start_client( connected_socket );

    }
    popup( _("server stoped succesfully."));
    return;
}

multiplay_client::multiplay_client(){

}

void multiplay_client::start_client(int connected_socket) {

    sock = connected_socket;
    client_thread =  std::thread([this]() { this->client_thread_process(); } );

}

void multiplay_client::client_thread_process(){

    recv(sock, recv_buffer, 12, 0);

    send(sock, recv_buffer, 12, 0);

    closesocket(sock);

    client_thread.detach();
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
