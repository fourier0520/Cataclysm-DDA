
#include "item_enchant.h"
#include "type_id.h"
#include "itype.h"
#include "rng.h"
#include "options.h"
#include "creature.h"
#include "player.h"
#include "item.h"
#include "messages.h"
#include "map.h"
#include "game.h"

#include <string>
#include <vector>
#include <map>

namespace
{
std::map<item_enchant_id, item_enchant> item_enchant_map;
std::vector<item_enchant> item_enchant_list;
std::map<item_enchant_type, std::vector<item_enchant>> item_enchant_list_map_by_type;
} // namespace

/** @relates string_id */
template<>
const item_enchant &string_id<item_enchant>::obj() const
{
    const auto iter = item_enchant_map.find( *this );
    if( iter == item_enchant_map.end() ) {
        debugmsg( "invalid enchant id id %s", c_str() );
        static const item_enchant dummy{};
        return dummy;
    }
    return iter->second;
}

item_enchant_type enchant_manager::item_enchant_type_string_to_enum( const std::string type_str ){
    if ( type_str == "enchant_effect_to_target" ) {
        return enchant_effect_to_target;
    } else if ( type_str == "enchant_effect_to_self" ) {
        return enchant_effect_to_self;
    } else if ( type_str == "enchant_fire_gun" ) {
        return enchant_fire_gun;
    } else if ( type_str == "enchant_anti_specie" ) {
        return enchant_anti_specie;
    } else if ( type_str == "enchant_heal_self" ) {
        return enchant_heal_self;
    } else if ( type_str == "enchant_emit_field" ) {
        return enchant_emit_field;
    } else if ( type_str == "enchant_teleport_target" ) {
        return enchant_teleport_target;
    } else {
        debugmsg( "unknown enchant type: %s", type_str );
        return enchant_null;
    }
}

void load_item_enchant( const JsonObject &jo, const std::string & ){

    item_enchant new_item_enchant;
    new_item_enchant.id = item_enchant_id( jo.get_string( "id" ) );
    new_item_enchant.name = jo.get_string( "name" );
    new_item_enchant.description = jo.get_string( "description" );

    new_item_enchant.enchant_type = enchant_manager::item_enchant_type_string_to_enum( jo.get_string( "enchant_type" ) );

    new_item_enchant.weight_in_natural_spawn = jo.get_int( "weight_in_natural_spawn" , 0);

    new_item_enchant.effect_chance = jo.get_float( "effect_chance" , 1.0);
    new_item_enchant.effect_chance_min = jo.get_float( "effect_chance_min" , new_item_enchant.effect_chance);
    new_item_enchant.effect_chance_max = jo.get_float( "effect_chance_max" , new_item_enchant.effect_chance);

    new_item_enchant.apply_message =  jo.get_string( "message_at_trigger" , "");

    if( new_item_enchant.enchant_type == enchant_effect_to_target){
        new_item_enchant.effect_type_id_to_apply_target = efftype_id( jo.get_string( "effect_type_id_to_apply_target" ) );
        new_item_enchant.effect_int =  jo.get_int( "effect_int" , 1);
        new_item_enchant.effect_duration_turn =  jo.get_int( "effect_duration_turn" , 10);
    } else if( new_item_enchant.enchant_type == enchant_effect_to_self){
        new_item_enchant.effect_type_id_to_apply_self = efftype_id( jo.get_string( "effect_type_id_to_apply_self" ) );
        new_item_enchant.effect_int =  jo.get_int( "effect_int" , 1);
        new_item_enchant.effect_duration_turn =  jo.get_int( "effect_duration_turn" , 10);

    } else if( new_item_enchant.enchant_type == enchant_fire_gun){
        new_item_enchant.gun_type_id_to_fire = itype_id( jo.get_string( "gun_type_id_to_fire" ) );
        new_item_enchant.gun_target_num =  jo.get_int( "gun_target_num" , 1);

    } else if( new_item_enchant.enchant_type == enchant_anti_specie){
        new_item_enchant.specie_id_to_anti = species_id( jo.get_string( "specie_id_to_anti" ) );
        new_item_enchant.gun_target_num =  jo.get_int( "gun_target_num" , 1);

    } else if( new_item_enchant.enchant_type == enchant_emit_field){
        new_item_enchant.emit_id_to_emit = emit_id( jo.get_string( "emit_id_to_emit" ) );
    }

    item_enchant_map[new_item_enchant.id] = new_item_enchant;
    item_enchant_list.push_back( new_item_enchant );
    item_enchant_list_map_by_type[ new_item_enchant.enchant_type ].push_back( new_item_enchant );
}

static item_enchant get_random_enchant_with_weight( const std::vector<item_enchant> &enchant_list) {

    int weight_sum = 0;

    for( item_enchant enchant : enchant_list ){
        weight_sum += enchant.weight_in_natural_spawn;
    }

    int weight_iterating_sum = rng( 0, weight_sum );
    for( item_enchant enchant : enchant_list ){
        weight_iterating_sum -= enchant.weight_in_natural_spawn;
        if( weight_iterating_sum <= 0 ) {
            return enchant;
        }
    }
    debugmsg( "choosen enchant weight is out bound! check weight_in_natural_spawn at item_enchant json" );
    return item_enchant();
}

/**
 * return random whole enchant data at natural generation
 */
item_enchant enchant_manager::generate_natual_enchant() {

    item_enchant new_item_enchant;

    int weight_effect_self     = get_option<int>("ENCHANT_WEIGHT_EFFECT_SELF");
    int weight_effect_target   = get_option<int>("ENCHANT_WEIGHT_EFFECT_TARGET");
    int weight_anti_specie     = get_option<int>("ENCHANT_WEIGHT_EFFECT_ANTI_SPECIE");
    int weight_fire_gun        = get_option<int>("ENCHANT_WEIGHT_EFFECT_FIRE_GUN");
    int weight_emit_field      = get_option<int>("ENCHANT_WEIGHT_EFFECT_EMIT_FIELD");
    int weight_heal_self       = get_option<int>("ENCHANT_WEIGHT_EFFECT_HEAL_SELF");
    // int weight_teleport_target = get_option<int>("ENCHANT_WEIGHT_EFFECT_TELEPORT_TARGET");

    int weight_sum =
        weight_effect_self      +
        weight_effect_target    +
        weight_anti_specie      +
        weight_fire_gun         +
        weight_emit_field       +
        weight_heal_self        +
    //    weight_teleport_target  +
        0;

    int weight_iterating_sum = rng( 0, weight_sum );

    if( ( weight_iterating_sum -= weight_effect_self) <= 0 ) {
        new_item_enchant = get_random_enchant_with_weight( item_enchant_list_map_by_type[ enchant_effect_to_self ] );

        if( new_item_enchant.effect_chance_min != new_item_enchant.effect_chance_max ) {
            new_item_enchant.effect_chance = rng_float( new_item_enchant.effect_chance_min, new_item_enchant.effect_chance_max );
        }

    } else if( ( weight_iterating_sum -= weight_effect_target ) <= 0 ) {
        new_item_enchant = get_random_enchant_with_weight( item_enchant_list_map_by_type[ enchant_effect_to_target ] );

        if( new_item_enchant.effect_chance_min != new_item_enchant.effect_chance_max ) {
            new_item_enchant.effect_chance = rng_float( new_item_enchant.effect_chance_min, new_item_enchant.effect_chance_max );
        }

    } else if( ( weight_iterating_sum -= weight_anti_specie ) <= 0 ) {
        new_item_enchant = get_random_enchant_with_weight( item_enchant_list_map_by_type[ enchant_anti_specie ] );

        if( new_item_enchant.effect_chance_min != new_item_enchant.effect_chance_max ) {
            new_item_enchant.effect_chance = rng_float( new_item_enchant.effect_chance_min, new_item_enchant.effect_chance_max );
        }

    } else if( ( weight_iterating_sum -= weight_fire_gun ) <= 0 ) {
        new_item_enchant = get_random_enchant_with_weight( item_enchant_list_map_by_type[ enchant_fire_gun ] );

        if( new_item_enchant.effect_chance_min != new_item_enchant.effect_chance_max ) {
            new_item_enchant.effect_chance = rng_float( new_item_enchant.effect_chance_min, new_item_enchant.effect_chance_max );
        }

    } else if( ( weight_iterating_sum -= weight_emit_field ) <= 0 ) {
        new_item_enchant = get_random_enchant_with_weight( item_enchant_list_map_by_type[ enchant_emit_field ] );

        if( new_item_enchant.effect_chance_min != new_item_enchant.effect_chance_max ) {
            new_item_enchant.effect_chance = rng_float( new_item_enchant.effect_chance_min, new_item_enchant.effect_chance_max );
        }

    } else if( ( weight_iterating_sum -= weight_heal_self ) <= 0 ) {
        new_item_enchant = get_random_enchant_with_weight( item_enchant_list_map_by_type[ enchant_heal_self ] );

        if( new_item_enchant.effect_chance_min != new_item_enchant.effect_chance_max ) {
            new_item_enchant.effect_chance = rng_float( new_item_enchant.effect_chance_min, new_item_enchant.effect_chance_max );
        }


//    } else if( ( weight_iterating_sum -= weight_teleport_target ) <= 0 ) {
//        new_item_enchant.enchant_type = enchant_teleport_target;
//
//        if( new_item_enchant.effect_chance_min != new_item_enchant.effect_chance_max ) {
//            new_item_enchant.effect_chance = rng_float( new_item_enchant.effect_chance_min, new_item_enchant.effect_chance_max );
//        }

    } else {
        debugmsg( "choosen enchant weight is out bound! check external option named ENCHANT_WEIGHT_EFFECT_xxx" );
    }

    return new_item_enchant;

}

void enchant_manager::invoke_damage_modifier_enchantment(damage_instance dmg,
        item_enchant &enchant, Creature &target, item &weapon, player&) {
    if( enchant.enchant_type == enchant_anti_specie ) {
        if ( rng_float( 0.0, 1.0 ) < enchant.effect_chance ) {
            if( target.in_species( enchant.specie_id_to_anti ) ) {
                dmg.mult_damage( enchant.anti_specie_multiplier );

                if( enchant.apply_message == ""){
                    if ( 2.0 < enchant.anti_specie_multiplier ) {
                        add_msg(_("%s is super effective to %s!"), weapon.tname(), target.get_name());
                    } else if ( 1.0 < enchant.anti_specie_multiplier ) {
                        add_msg(_("%s is very effective to %s!"), weapon.tname(), target.get_name());
                    } else {
                        add_msg(_("%s is not effective to %s..."), weapon.tname(), target.get_name());
                    }
                } else {
                    add_msg( enchant.apply_message,  weapon.tname() );
                }
            }
        }
    }
}

void enchant_manager::invoke_enchantment_effect(item_enchant& enchant, Creature& target,
        item &weapon, player& user) {

    if ( rng_float( 0.0, 1.0 ) < enchant.effect_chance ) {

        if( enchant.enchant_type == enchant_effect_to_target ) {
            target.add_effect(
                    efftype_id( enchant.effect_type_id_to_apply_target ),
                    time_duration::from_turns(enchant.effect_duration_turn));

            add_msg( enchant.apply_message,  weapon.tname(), target.get_name(), user.get_name());

        } else if ( enchant.enchant_type == enchant_effect_to_self ) {
            user.add_effect(
                    efftype_id( enchant.effect_type_id_to_apply_self ),
                    time_duration::from_turns(enchant.effect_duration_turn) );
            add_msg( enchant.apply_message,  weapon.tname(), target.get_name(), user.get_name());

        } else if ( enchant.enchant_type == enchant_anti_specie ) {
            // anti specie is in damage modifire enchantment.
        } else if ( enchant.enchant_type == enchant_fire_gun ) {
            add_msg("teleport is not inplemented yet, sorry!");
            add_msg( enchant.apply_message,  weapon.tname(), target.get_name(), user.get_name());
        } else if ( enchant.enchant_type == enchant_emit_field ) {
            g->m.emit_field( target.pos(), emit_id( enchant.emit_id_to_emit ) , 1.0f );
            add_msg( enchant.apply_message,  weapon.tname(), target.get_name(), user.get_name());

        } else if ( enchant.enchant_type == enchant_heal_self ) {
            add_msg("teleport is not inplemented yet, sorry!");
            add_msg( enchant.apply_message,  weapon.tname(), target.get_name(), user.get_name());
        } else if ( enchant.enchant_type == enchant_teleport_target ) {
            add_msg("teleport is not inplemented yet, sorry!");
            add_msg( enchant.apply_message,  weapon.tname(), target.get_name(), user.get_name());
        }
    }

}

