#pragma once
#ifndef ITEM_ENCHANT_H
#define ITEM_ENCHANT_H

#include <string>
#include <vector>

#include "type_id.h"
#include "itype.h"

enum item_enchant_type{
    enchant_null = 0,
    enchant_effect_to_target,
    enchant_effect_to_self,
    enchant_anti_specie,
    enchant_fire_gun,
    enchant_emit_field,
    enchant_heal_self,
    enchant_teleport_target,

};
enum heal_type{
    heal_null = 0,
    heal_hp,
    heal_stamina,
    heal_fatigue,
    heal_mana,
};

class item_enchant {

    public:
        void serialize( JsonOut &json ) const;
        void deserialize( const JsonObject &jo );
    public:
        item_enchant_id id;
        std::string name;
        std::string description;

        item_enchant_type enchant_type = enchant_null;

        bool obsolete;
        int spawn_weight_in_natural;
        int spawn_weight_in_crafting;
        int allowed_attack_sum_min;
        int allowed_attack_sum_max;
        std::vector<std::string> exclude_group;

        float effect_chance;
        float effect_chance_min;
        float effect_chance_max;

        efftype_id effect_type_id_to_apply_target;
        efftype_id effect_type_id_to_apply_self;
        int effect_int;
        int effect_duration_turn;

        itype_id gun_type_id_to_fire;
        int gun_target_num;
        int gun_max_range;
        gun_mode_id gun_fire_mode;
        itype_id gun_ammo;
        std::string message_on_fire_gun;

        species_id specie_id_to_anti;
        float anti_specie_multiplier;
        int anti_specie_constant_damage;

        emit_id emit_id_to_emit;

        int heal_amount;
        heal_type heal_type;

        std::string message_on_trigger;

};

class enchant_manager {

    public:
        static item_enchant_type item_enchant_type_string_to_enum( const std::string );
        static heal_type enchant_heal_type_string_to_enum( const std::string );

        static void load_enchant_balance_options();

        static bool check_enchant_allows_item( item_enchant, item& );

        static item_enchant get_random_enchant_with_weight( item&, bool );

        // static item_enchant generate_natual_enchant(item&, bool);

        static void invoke_damage_modifier_enchantment( damage_instance &dmg, item_enchant&, Creature&, item&, player& );

        static void invoke_enchantment_effect( item_enchant&, Creature&, item&, player& );


        static void add_random_enchant_to_item( item& , bool );
        static void add_random_enchant_to_item( std::vector<item>& , bool );
        static void add_random_enchant_to_item( std::list<item>& , bool );

        // enchant balance things.
        // load by load_enchant_balance_options()
        // from external options
        // external optionfiles are
        // data/mods/variant_addition/enchant/*.json
};

void load_item_enchant( const JsonObject &jo, const std::string &src );



#endif /* SRC_ITEM_ENCHANT_H_ */
