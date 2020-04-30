#pragma once
#ifndef CUSTOM_ACTIVITY_H
#define CUSTOM_ACTIVITY_H

#include <string>
#include <vector>

#include "type_id.h"
#include "itype.h"


struct custom_activity {
    custom_activity_id id;
    std::string name;
    std::string name_in_select_menu;
    std::string query_yn_message;
    std::string start_message;
    std::string not_ready_message;
    std::string doing_message;
    std::string finish_message;
    std::string pet_monster_id_str;
    std::string use_item_id_str;
    mtype_id pet_monster_id;
    itype_id use_item_id;
    std::string morale_type_id;
    int morale_amount;
    int morale_duration_turns; // currently turns only
    std::string effect_type_id;
    int effect_intence;
    int effect_duration_turns;
    std::string result_item_id_str;
    int result_item_charges;
    bool is_result_item_drop_to_ground;
    int to_finish_turns;
    bool is_free_monster_in_act;
    int pet_cooldown_turns;
};

class custom_activity_manager {

    public:
        static custom_activity* find_pet_monster_activity( mtype_id monster_id );
        static custom_activity* find_use_item_activity( itype_id item_id );
};

void load_custom_activity( const JsonObject &jo, const std::string &src );


#endif /* SRC_CUSTOM_ACTIVITY_H_ */
