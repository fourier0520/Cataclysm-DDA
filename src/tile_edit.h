#pragma once
#ifndef TILE_EDIT_H

#include "json.h"
#include "enums.h"
#include "item.h"

struct user_tile_info
{
    std::string id;
    std::string fg_file;
    std::string bg_file;
    int w;
    int h;
    int R = -1;
    int G = -1;
    int B = -1;
    int offset_x = 0;
    int offset_y = 0;
};

class user_tile
{
    public:
        user_tile();
        ~user_tile();

        std::vector<user_tile_info> load();
        bool save();
        void serialize( JsonOut &json ) const;
        user_tile_info get_user_tile_info_from_json( JsonObject jo );
        std::vector<std::string> get_user_image_files();
        void remove_tile_info_by_id( std::string id );
        void update_tile_info( user_tile_info new_ut );
        std::string get_tile_info_msg( std::string id );

        std::tuple<bool, user_tile_info> set_tile_menu( std::string id, std::vector<std::string> img_files );

        void item_tile_edit();

        std::vector<user_tile_info> info;
        std::vector<user_tile_info> new_info;
};

#endif // TILE_EDIT_H
