-- RST
-- Military Sites
-- --------------
--
-- Militarysites are a type of building that can house soldiers and that expand the players' terrain.
-- They can also be attacked.
--
-- Militarysites are defined in
-- ``data/tribes/buildings/militarysites/<tribe_name>/<building_name>/init.lua``.
-- The building will also need its help texts, which are defined in
-- ``data/tribes/buildings/militarysites/<tribe_name>/<building_name>/helptexts.lua``

dirname = path.dirname(__file__)

-- RST
-- .. function:: new_militarysite_type{table}
--
--    This function adds the definition of a military site building to the engine.
--
--    :arg table: This table contains all the data that the game engine will add to this building.
--                In addition to the :ref:`lua_tribes_buildings_common`, it contains the following entries:
--
--    **max_soldiers**
--        *Mandatory*. The maximum number of solders that can be garrisoned at this building.
--
--    **heal_per_second**
--        *Mandatory*. The number of health points that a garrisoned soldier will heal each second.
--
--    **conquers**
--        *Mandatory*. The conquer radius for this building.
--
--    **prefer_heroes**
--        *Mandatory*. Whether this building prefers heroes or rookies per default.
--        Set to ``true`` or ``false``.
--
--    **messages**
--        *Mandatory*. Messages that this building will send to the user. There are 5 messages,
--        defined like this::
--
--            messages = {
--                occupied = _"Your soldiers have occupied your fortress.",
--                aggressor = _"Your fortress discovered an aggressor.",
--                attack = _"Your fortress is under attack.",
--                defeated_enemy = _"The enemy defeated your soldiers at the fortress.",
--                defeated_you = _"Your soldiers defeated the enemy at the fortress."
--            }
tribes:new_militarysite_type {
   msgctxt = "atlanteans_building",
   name = "atlanteans_castle",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Castle"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      planks = 4,
      granite = 7,
      log = 5,
      spidercloth = 2,
      diamond = 1,
      quartz = 1
   },
   return_on_dismantle = {
      planks = 2,
      granite = 5,
      log = 2
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 91, 91 },
      }
   },

   aihints = {
      expansion = true,
      fighting = true,
      mountain_conqueror = true,
      prohibited_till = 1500
   },

   max_soldiers = 12,
   heal_per_second = 200,
   conquers = 12,
   prefer_heroes = true,

   messages = {
      occupied = _"Your soldiers have occupied your castle.",
      aggressor = _"Your castle discovered an aggressor.",
      attack = _"Your castle is under attack.",
      defeated_enemy = _"The enemy defeated your soldiers at the castle.",
      defeated_you = _"Your soldiers defeated the enemy at the castle."
   },
}
