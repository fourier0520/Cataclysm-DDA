
#ifndef MULTIPLAY_MANAGER_H_
#define MULTIPLAY_MANAGER_H_

#include <string>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <thread>
#include <vector>
#include <mutex>

#include "type_id.h"
#include "itype.h"
#include "output.h"
#include "debug.h"

class multiplay_client {

    // std::mutex client_mtx;

    public:
        multiplay_client( );
        // ~multiplay_client() = delete;

    private:
        std::thread client_thread;
        SOCKET sock;
        char recv_buffer[8];
        bool is_working_value = false;
    public:
        void start_client_thread( SOCKET );
        void client_work_finish_cleanup();
        void client_thread_process();
        bool is_working();

};

class multiplay_manager {

    private:
        std::thread server_thread;
        // std::vector<multiplay_client> client_list;

        bool is_server_active = false;
    public:
        int start_server();
        int stop_server();

        int multiplay_menu();

        void server_thread_process();

        int do_turn();

};


#endif /* MULTIPLAY_MANAGER_H_ */
