-- RST
-- .. _lua_tribes_tribes_start_conditions:
--
-- Start Conditions
-- ================
--
-- The start conditions a tribe can use are listed in the tribes :ref:`init.lua <lua_tribes_tribes_init>` file.
--
-- They are defined in their definition files named as follows and located in
-- ``data/tribes/initialization/<tribe_name>/starting_conditions/<start_condition_name>.lua``.
--
-- .. function:: init
--
--    This function initializes the start condition.
--
--    :arg table: This table contains all the data needed to initilize the start condition.
--                It contains the following entries:
--
--    **descname**: The localized name of the start condition.
--
--    **tooltip**: A short description of the start condition. Should contain essential hints if necessary.
--
--    **map_tags**: *Optional* A table with map tags that are needed for this startcondition.
--                  e.g. ``{ "seafaring" }`` if the start conditions needs a seafaring map
--
--    **incompatible_wc**: *Optional* A table with strings containing the path of win conditions taht are incompatible with this
--                         startcondition.
--                         e.g. ``{ "scripting/win_conditions/hq_hunter.lua" }`` if the start conditions is incompatible
--                         with the win condition "HQ Hunter"
--
--    **func**: A standardized function allowing to share this start conditions between players (Shared Kingdom)
--
-- After these declarations the lua commands to define the initial mapobjects for the player follow.
--
-- For making the UI texts translateable, we also need to push/pop the correct textdomain.
--
-- Example:
--
-- .. code-block:: lua
--
-- include "scripting/starting_conditions.lua"

-- push_textdomain("tribes")

-- init = {
--    -- TRANSLATORS: This is the name of a starting condition
--    descname = _ "Discovery",
--    -- TRANSLATORS: This is the tooltip for the "Discovery" starting condition
--    tooltip = _"Start the game with three ships on the ocean and only a handful of supplies",
--    map_tags = {"seafaring"},
--    incompatible_wc = {"scripting/win_conditions/hq_hunter.lua"},
--
--    func = function(player, shared_in_start)
--
--    local map = wl.Game().map
--    local sf = map.player_slots[player.number].starting_field
--    if shared_in_start then
--       sf = shared_in_start
--    else
--       player:allow_workers("all")
--    end
--
--    launch_expeditions(player, {
--       {
--          log = 4,
--          granite = 1,
--          planks = 2,
--          spidercloth = 3,
--          atlanteans_woodcutter = 3,
--          atlanteans_forester = 2,
--          atlanteans_soldier = 1,
--       },
--       {
--          log = 3,
--          granite = 4,
--          planks = 2,
--          spidercloth = 3,
--          atlanteans_stonecutter = 2,
--          atlanteans_soldier = 1,
--       },
--       {
--          iron = 2,
--          atlanteans_soldier = 1,
--          atlanteans_geologist = 1,
--          atlanteans_miner = 3,
--          atlanteans_smelter = 2,
--          atlanteans_toolsmith = 1,
--          atlanteans_baker = 2,
--          atlanteans_smoker = 2,
--          atlanteans_fisher = 1,
--       },
--    })
-- end
-- }
--
-- pop_textdomain()
-- return init


-- =======================================================================
--                 Start conditions for Discovery
-- =======================================================================

include "scripting/starting_conditions.lua"

push_textdomain("tribes")

init = {
   -- TRANSLATORS: This is the name of a starting condition
   descname = _ "Discovery",
   -- TRANSLATORS: This is the tooltip for the "Discovery" starting condition
   tooltip = _"Start the game with three ships on the ocean and only a handful of supplies",
   map_tags = {"seafaring"},
   incompatible_wc = {"scripting/win_conditions/hq_hunter.lua"},

   func = function(player, shared_in_start)

   local map = wl.Game().map
   local sf = map.player_slots[player.number].starting_field
   if shared_in_start then
      sf = shared_in_start
   else
      player:allow_workers("all")
   end

   launch_expeditions(player, {
      {
         log = 4,
         granite = 1,
         planks = 2,
         spidercloth = 3,
         atlanteans_woodcutter = 3,
         atlanteans_forester = 2,
         atlanteans_soldier = 1,
      },
      {
         log = 3,
         granite = 4,
         planks = 2,
         spidercloth = 3,
         atlanteans_stonecutter = 2,
         atlanteans_soldier = 1,
      },
      {
         iron = 2,
         atlanteans_soldier = 1,
         atlanteans_geologist = 1,
         atlanteans_miner = 3,
         atlanteans_smelter = 2,
         atlanteans_toolsmith = 1,
         atlanteans_baker = 2,
         atlanteans_smoker = 2,
         atlanteans_fisher = 1,
      },
   })
end
}

pop_textdomain()
return init
