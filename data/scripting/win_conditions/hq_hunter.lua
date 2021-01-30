-- =======================================================================
--                         HQ Hunter Win condition
-- =======================================================================

include "scripting/coroutine.lua" -- for sleep
include "scripting/table.lua"
include "scripting/win_conditions/win_condition_functions.lua"

push_textdomain("win_conditions")

include "scripting/win_conditions/win_condition_texts.lua"

local wc_name = "HQ Hunter"
-- This needs to be exactly like wc_name, but localized, because wc_name
-- will be used as the key to fetch the translation in C++
local wc_descname = _("HQ Hunter")
local wc_version = 2
local wc_desc = _ "The tribe or team that can destroy all other headquarters wins the game!. This wincondition does not work with the following starting conditions: Poor Hamlet, Fortified Village, Discovery and New World."
local r = {
    name = wc_name,
    description = wc_desc,
    peaceful_mode_allowed = false,
    func = function()
        local plrs = wl.Game().players

        -- set the objective with the game type for all players
        broadcast_objective("win_condition", wc_descname, wc_desc)

        -- Iterate all players, if one has lost his Headquarters, destroy all remaing Warehouses and ports, 
        -- remove him from the list, send him a defeated message and give him full vision
        repeat
            sleep(5000)
            -- checkk if a player still has a Headquarters
            for idx,p in ipairs(plrs) do
                if (
                    #p:get_buildings("barbarians_headquarters") + 
                    #p:get_buildings("atlanteans_headquarters") + 
                    #p:get_buildings("empire_headquarters") + 
                    #p:get_buildings("frisians_headquarters") +
                    #p:get_buildings("amazons_headquarters")
                ) == 0 then
                    for idx,b in ipairs(array_combine(
                        p:get_buildings("barbarians_warehouse"),
                        p:get_buildings("atlanteans_warehouse"),
                        p:get_buildings("empire_warehouse"),
                        p:get_buildings("frisians_warehouse"),
                        p:get_buildings("amazons_warehouse"),
                        p:get_buildings("barbarians_port"),
                        p:get_buildings("atlanteans_port"),
                        p:get_buildings("empire_port"),
                        p:get_buildings("frisians_port"),
                        p:get_buildings("amazons_port")
                    )) do
                        b:destroy()
                    end
                end
            end
            check_player_defeated(plrs, lost_game.title, lost_game.body, wc_descname, wc_version)
        until count_factions(plrs) <= 1

        -- Send congratulations to all remaining players
        broadcast_win(plrs,
        won_game.title,
        won_game.body,{},
        wc_descname, wc_version
        )

    end,
}
pop_textdomain()
return r
