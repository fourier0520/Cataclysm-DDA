#include "tile_edit.h"

#include "json.h"
#include "path_info.h"
#include "filesystem.h"
#include "game.h"
#include "player.h"
#include "ui.h"
#include "uistate.h"
#include "input.h"
#include "output.h"
#include "item_factory.h"
#include "cata_tiles.h"
#include "lightmap.h"

#include <SDL_image.h>

extern std::unique_ptr<cata_tiles> tilecontext;
extern std::unique_ptr<user_tile> user_tile_setting;

user_tile::~user_tile() = default;

user_tile::user_tile()
{
    info.clear();
}

user_tile_info user_tile::get_user_tile_info_from_json( JsonObject jo )
{
    std::string id = jo.get_string( "id" );
    std::string fg_file = jo.get_string( "fg_file" );
    std::string bg_file = jo.get_string( "bg_file" );

    user_tile_info tile_info;
    tile_info.id = id;
    tile_info.fg_file = fg_file;
    tile_info.bg_file = bg_file;
    tile_info.offset_x = 0;
    tile_info.offset_y = 0;

    return tile_info;
}

std::vector<user_tile_info> user_tile::load()
{
    info.clear();

    std::string user_tile_conf = FILENAMES["user_tile_dir"] + FILENAMES["user_tile_conf"];

    if( !assure_dir_exist( FILENAMES["user_tile_dir"] ) ) {
        popup( _( "Unable to make config directory. Check permissions." ) );
        return info;
    }
    std::ifstream config_file( user_tile_conf.c_str(), std::ifstream::in | std::ifstream::binary );
    if (!config_file.good()) {
        return info;
    }

    JsonIn config_json( config_file );
    JsonArray config_array = config_json.get_array();
    while (config_array.has_more()) {
        JsonObject config = config_array.next_object();
        info.push_back( user_tile::get_user_tile_info_from_json( config ) );
    }

    return info;
}

void tileset_loader::load_user_tiles()
{
    std::string img_dir = FILENAMES["user_tile_dir"];
    // Enumurate all image files
    std::vector<std::string> img_files;
    for(auto &t : user_tile_setting->info) {
        std::vector<std::string>::iterator iter = std::find( img_files.begin(), img_files.end(), t.fg_file );
        size_t index = std::distance( img_files.begin(), iter );
        if (index == img_files.size()) {
            img_files.push_back( t.fg_file );
        }
        iter = std::find( img_files.begin(), img_files.end(), t.bg_file );
        index = std::distance( img_files.begin(), iter );
        if (index == img_files.size()) {
            img_files.push_back( t.bg_file );
        }
    }

    std::map<std::string, int> user_sprite_map;
    // Load user image files
    for( auto &img_path : img_files ) {
        // Check image file size
        auto full_path = img_dir + img_path;
        SDL_Surface_Ptr tile_atlas( IMG_Load( full_path.c_str() ) );
        if ( !tile_atlas ) {
            throw std::runtime_error( "Could not load tile image \"" + full_path + "\": " + IMG_GetError() );
        }
        // @todo: allow multi tile file?
        sprite_width = tile_atlas->w;
        sprite_height = tile_atlas->h;
        load_tileset( full_path );
        user_sprite_map[img_path] = offset;
        offset += size;
    }

    // Add user tilesets
    for( auto &t : user_tile_setting->info) {
        tile_type curr_subtile;
        std::vector<int> fg, bg;
        fg.push_back( user_sprite_map[t.fg_file] );
        bg.push_back( user_sprite_map[t.bg_file] );
        curr_subtile.fg.add( fg, 1 );
        curr_subtile.bg.add( bg, 1 );
        ts.create_tile_type( t.id, std::move( curr_subtile ) );
    }
}

std::vector<std::string> user_tile::get_user_image_files()
{
    auto dir = FILENAMES["user_tile_dir"];
    auto files = get_files_from_path( ".png", dir, true, true );
    std::vector<std::string> ret;
    for( auto file : files ) {
        if( file.find( dir ) == 0 ) {
            ret.push_back( file.replace( 0, dir.size(), "" ) );
        }
    }
    return ret;
}

void user_tile::serialize( JsonOut &json ) const
{
    json.start_array();
    for ( user_tile_info t : info ) {
        json.start_object();

        json.member( "id", t.id );
        json.member( "fg_file", t.fg_file );
        json.member( "bg_file", t.bg_file );

        json.end_object();
    }
    json.end_array();
}

bool user_tile::save()
{
    const auto savefile = FILENAMES["user_tile_dir"] + FILENAMES["user_tile_conf"];

    if (!assure_dir_exist( FILENAMES["user_tile_dir"] )) {
        popup( _( "Unable to make config directory. Check permissions." ) );
        return false;
    }
    return write_to_file( savefile, [&]( std::ostream &fout ) {
        JsonOut jout( fout, true );
        serialize( jout );
    }, _( "user tiles" ) );
}

std::string user_tile::get_tile_info_msg( std::string id )
{
    std::string msg;
    bool found = false;
    for (auto t : new_info) {
        bool ret = false;
        if( t.id == id ) {
            msg += "Normal item tile \n";
            ret = true;
        }
        if( ( "overlay_wielded_" + id ) == t.id ) {
            msg += "Wielded item tile \n";
            ret = true;
        }
        if( ( "overlay_worn_" + id ) == t.id ) {
            msg += "Worn item tile \n";
            ret = true;
        }
        if( ret ) {
            msg += " Forground: " + t.fg_file + "\n";
            msg += " Background: " + t.bg_file + "\n";
            found = true;
        }
    }

    if( !found ) {
        msg = "Default tile";
    }
    return msg;
}

class item_tile_edit_callback: public uimenu_callback
{
    public:
        std::string msg;
        const std::vector<const itype *> &standard_itype_ids;
        item_tile_edit_callback( const std::vector<const itype *> &ids ) :
            msg(), standard_itype_ids( ids ) {
        }
        void select( int entnum, uimenu *menu ) override {
            const int starty = 3;
            const int startx = menu->w_width - menu->pad_right;
            const std::string padding( menu->pad_right, ' ' );
            for( int y = 2; y < menu->w_height - 1; y++ ) {
                mvwprintw( menu->window, y, startx - 1, padding );
            }
            item tmp( standard_itype_ids[entnum], calendar::turn );
            std::string id = standard_itype_ids[entnum]->get_id();
            mvwhline( menu->window, 1, startx, ' ', menu->pad_right - 1 );
            const std::string header = string_format( "#%d: %s", entnum, id.c_str() );
            mvwprintz( menu->window, 1, startx + ( menu->pad_right - 1 - header.size() ) / 2, c_cyan,
                       header );

            fold_and_print( menu->window, starty, startx, menu->pad_right - 1, c_light_gray,
                            user_tile_setting->get_tile_info_msg( id ) );

            mvwprintz( menu->window, menu->w_height - 3, startx, c_green, msg );
            msg.erase();

            input_context ctxt( "UIMENU" );
            mvwprintw( menu->window, menu->w_height - 2, startx, _( "[%s] find, [%s] quit" ),
                       ctxt.get_desc( "FILTER" ).c_str(), ctxt.get_desc( "QUIT" ).c_str() );
        }
};

std::tuple<bool, user_tile_info> user_tile::set_tile_menu( std::string id, std::vector<std::string> img_files )
{
    user_tile_info ut;
    ut.id = id;

    uimenu filemenu;
    filemenu.w_x = 4;
    filemenu.w_width = TERMX - 8;
    filemenu.return_invalid = true;
    filemenu.title = _( "Select forground image file" );
    for( size_t i = 0; i < img_files.size(); i++ ) {
        filemenu.addentry( i, true, 0, _( img_files[i].c_str() ) );
    }
    filemenu.query();
    if( filemenu.ret < 0 ) {
        return std::forward_as_tuple( false, ut );
    }
    ut.fg_file = img_files[filemenu.ret];

    filemenu.title = _( "Select background image file" );
    for (size_t i = 0; i < img_files.size(); i++) {
        filemenu.addentry( i, true, 0, _( img_files[i].c_str() ) );
    }
    filemenu.query();
    if (filemenu.ret < 0) {
        return std::forward_as_tuple( false, ut );
    }
    ut.bg_file = img_files[filemenu.ret];

    return std::forward_as_tuple( true, ut );
}

std::tuple<bool, bool, std::string> select_change_type( item ity )
{
    std::string prefix = "";
    bool ret = true;
    bool clear = false;

    uimenu menu;
    menu.return_invalid = true;
    menu.addentry( 0, true, 0, _( "Change normal tile" ) );
    menu.addentry( 1, true, 0, _( "Change wielded tile" ) );
    if (ity.is_armor()) {
        menu.addentry( 2, true, 0, _( "Change worn tile" ) );
    }
    menu.addentry( 3, true, 0, _( "Restore default tile" ) );
    menu.addentry( 4, true, 0, _( "Cancel" ) );
    menu.query();

    switch( menu.ret ) {
        case 0:
            break;
        case 1:
            prefix = "overlay_wielded_";
            break;
        case 2:
            prefix = "overlay_worn_";
            break;
        case 3:
            clear = true;
            break;
        default:
            ret = false;
            break;
    }

    return std::forward_as_tuple( ret, clear, prefix );
}

void user_tile::remove_tile_info_by_id( std::string id )
{
    std::vector<user_tile_info>::iterator it = new_info.begin();
    while( it != new_info.end() ) {
        if( id == it->id || ("overlay_wielded_" + id) == it->id
            || ("overlay_worn_" + id) == it->id) {
            it = new_info.erase( it );
        }
        else ++it;
    }
}

void user_tile::update_tile_info( user_tile_info new_ut )
{
    std::vector<user_tile_info>::iterator it = new_info.begin();
    while( it != new_info.end() ) {
        if( new_ut.id == it->id ) {
            it = new_info.erase( it );
        }
        else ++it;
    }
    new_info.push_back( new_ut );
}

void user_tile::item_tile_edit()
{
    const auto opts = item_controller->all();

    int prev_amount = 1;
    int amount = 1;
    uimenu menu;
    menu.w_x = 0;
    menu.w_width = TERMX;
    menu.pad_right = ( TERMX / 2 > 40 ? TERMX - 40 : TERMX / 2 );
    menu.return_invalid = true;
    item_tile_edit_callback cb( opts );
    menu.callback = &cb;

    for( size_t i = 0; i < opts.size(); i++ ) {
        item ity( opts[i], 0 );
        menu.addentry( i, true, 0, string_format( _( "%.*s" ), menu.pad_right - 5,
                        ity.tname( 1, false ).c_str() ) );
        menu.entries[i].extratxt.txt = ity.symbol();
        menu.entries[i].extratxt.color = ity.color();
        menu.entries[i].extratxt.left = 1;
    }

    auto img_files = get_user_image_files();
    if( img_files.empty() ) {
        popup( _( "No image file found in user tile folder." ) );
        return;
    }

    new_info.clear();
    std::copy( info.begin(), info.end(), std::back_inserter( new_info ) );
    do {
        menu.query();
        if( menu.ret >= 0 ) {
            item ity( opts[menu.ret] );
            bool ret;
            bool clear;
            std::string prefix;
            // Check item tile type
            std::tie( ret, clear, prefix ) = select_change_type( ity );
            if( ret ) {
                if( clear ) {
                    // Restore default tile
                    remove_tile_info_by_id( ity.typeId() );
                } else {
                    user_tile_info ut;
                    std::string id = prefix + ity.typeId();
                    std::tie( ret, ut ) = set_tile_menu( id, img_files );
                    if (ret) {
                        update_tile_info( ut );
                    }
                }
            }
        }
    } while( menu.ret >= 0 );

    if( !new_info.empty() ) {
        if(query_yn( _( "Save changes? (Requires restart)" ) )) {
            info = new_info;
            save();
        }
    }
}
