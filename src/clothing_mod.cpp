#include "clothing_mod.h"

#include <string>
#include <set>

#include "generic_factory.h"
#include "item.h"

namespace
{

generic_factory<clothing_mod> all_clothing_mods( "clothing mods" );

} // namespace

/** @relates string_id */
template<>
bool string_id<clothing_mod>::is_valid() const
{
    return all_clothing_mods.is_valid( *this );
}

/** @relates string_id */
template<>
const clothing_mod &string_id<clothing_mod>::obj() const
{
    return all_clothing_mods.obj( *this );
}

void clothing_mod::load( JsonObject &jo, const std::string & )
{
    mandatory( jo, was_loaded, "flag", flag );
    mandatory( jo, was_loaded, "item", item_string );
    mandatory( jo, was_loaded, "implement_prompt", implement_prompt );
    mandatory( jo, was_loaded, "destroy_prompt", destroy_prompt );

    JsonArray jarr = jo.get_array( "mod_value" );
    while( jarr.has_more() ) {
        JsonObject mv_jo = jarr.next_object();
        mod_value mv;
        mandatory( mv_jo, was_loaded, "type", mv.type );
        mandatory( mv_jo, was_loaded, "value", mv.value );
        JsonArray jarr_prop = mv_jo.get_array( "proportion" );
        while( jarr_prop.has_more() ) {
            std::string str = jarr_prop.next_string();
            if( str == "thickness" ) {
                mv.thickness_propotion = true;
            } else if( str == "coverage" ) {
                mv.coverage_propotion = true;
            }
        }
        mod_values.push_back( mv );
    }
}

float clothing_mod::get_mod_val( const std::string type, const item &it )
{
    const int thickness = it.get_thickness();
    const int coverage = it.get_coverage();
    float result = 0.0f;
    for( mod_value &mv : mod_values ) {
        if( mv.type == type ) {
            float tmp = mv.value;
            if( mv.thickness_propotion ) {
                tmp *= thickness;
            }
            if( mv.coverage_propotion ) {
                tmp *= coverage / 100.0f;
            }
            result += tmp;
        }
    }
    return result;
}

size_t clothing_mod::count()
{
    return all_clothing_mods.size();
}

void clothing_mods::load( JsonObject &jo, const std::string &src )
{
    all_clothing_mods.load( jo, src );
}

void clothing_mods::reset()
{
    all_clothing_mods.reset();
}

const std::vector<clothing_mod> &clothing_mods::get_all()
{
    return all_clothing_mods.get_all();
}
