
#include "custom_activity.h"
#include "type_id.h"
#include "itype.h"

#include <string>
#include <vector>
#include <map>

namespace
{
std::map<custom_activity_id, custom_activity> custom_activity_map;
std::map<mtype_id, custom_activity> custom_activity_map_pet_monster;
std::map<itype_id, custom_activity> custom_activity_map_use_item;
} // namespace

/** @relates string_id */
template<>
const custom_activity &string_id<custom_activity>::obj() const
{
    const auto iter = custom_activity_map.find( *this );
    if( iter == custom_activity_map.end() ) {
        debugmsg( "invalid custom activity id %s", c_str() );
        static const custom_activity dummy{};
        return dummy;
    }
    return iter->second;
}

custom_activity* custom_activity_manager::find_pet_monster_activity( mtype_id monster_id ){
    const auto iter = custom_activity_map_pet_monster.find( monster_id );
    if( iter == custom_activity_map_pet_monster.end() ) {
        return nullptr;
    }
    return &(iter->second);
}

custom_activity* custom_activity_manager::find_use_item_activity( itype_id item_id ){
    const auto iter = custom_activity_map_use_item.find( item_id );
    if( iter == custom_activity_map_use_item.end() ) {
        return nullptr;
    }
    return &(iter->second);
}

void load_custom_activity( const JsonObject &jo, const std::string & ){

    custom_activity new_custom_activity;
    new_custom_activity.id = custom_activity_id( jo.get_string( "id" ) );
    new_custom_activity.name = jo.get_string( "name" );
    new_custom_activity.name_in_select_menu = jo.get_string( "name_in_select_menu", new_custom_activity.name );
    new_custom_activity.start_message = jo.get_string( "start_message", "" );
    new_custom_activity.query_yn_message = jo.get_string( "query_yn_message", "" );
    new_custom_activity.not_ready_message = jo.get_string( "not_ready_message", "not ready." );
    new_custom_activity.doing_message = jo.get_string( "doing_message", "" );
    new_custom_activity.finish_message = jo.get_string( "finish_message", "" );
    new_custom_activity.pet_monster_id_str =  jo.get_string( "pet_monster_id" , "");
    new_custom_activity.pet_monster_id =  mtype_id(new_custom_activity.pet_monster_id_str);
    new_custom_activity.use_item_id_str =  jo.get_string( "use_item_id" , "");
    new_custom_activity.use_item_id =  itype_id(new_custom_activity.use_item_id);
    new_custom_activity.morale_type_id =  jo.get_string( "morale_type_id" , "");
    new_custom_activity.morale_amount =  jo.get_int( "morale_amount" , 10);
    new_custom_activity.morale_duration_turns =  jo.get_int( "morale_duration_turns" , 600);
    new_custom_activity.effect_type_id =  jo.get_string( "effect_type_id" , "");
    new_custom_activity.effect_intence =  jo.get_int( "effect_intence" , 1);
    new_custom_activity.effect_duration_turns =  jo.get_int( "effect_duration_turns" , 600);
    new_custom_activity.result_item_id_str =  jo.get_string( "result_item_id" , "");
    new_custom_activity.result_item_charges =  jo.get_int( "result_item_charges" , 1);
    new_custom_activity.is_result_item_drop_to_ground =  jo.get_bool( "is_result_item_drop_to_ground" , false);
    new_custom_activity.to_finish_turns =  jo.get_int( "to_finish_turns" , 600);
    new_custom_activity.pet_cooldown_turns =  jo.get_int( "pet_cooldown_turns" , 600);
    new_custom_activity.is_free_monster_in_act =  jo.get_bool( "is_free_monster_in_act" , false);

    custom_activity_map[new_custom_activity.id] = new_custom_activity;
    custom_activity_map_pet_monster[new_custom_activity.pet_monster_id] = new_custom_activity;
    custom_activity_map_use_item[new_custom_activity.use_item_id] = new_custom_activity;
}
