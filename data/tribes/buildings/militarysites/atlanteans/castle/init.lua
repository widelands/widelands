-- RST
-- .. _lua_tribes_buildings_militarysites:
--
-- Military Sites
-- --------------
--
-- Militarysites are a type of building that can house soldiers and that expand the players' terrain.
-- They can also be attacked.
--
-- Militarysites are defined in
-- ``data/tribes/buildings/militarysites/<tribe_name>/<building_name>/init.lua``.
-- The building will also need its :ref:`help texts <lua_tribes_tribes_helptexts>`,
-- which are defined in ``data/tribes/initialization/<tribe_name>/units.lua``
--
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
--                occupied = pgettext("atlanteans_building", "Your soldiers have occupied your fortress."),
--                aggressor = pgettext("atlanteans_building", "Your fortress discovered an aggressor."),
--                attack = pgettext("atlanteans_building", "Your fortress is under attack."),
--                defeated_enemy = pgettext("atlanteans_building", "The enemy defeated your soldiers at the fortress."),
--                defeated_you = pgettext("atlanteans_building", "Your soldiers defeated the enemy at the fortress.")
--            }
--
-- For making the UI texts translateable, we also need to push/pop the correct textdomain.
--
-- Example:
--
-- .. code-block:: lua
--
--    push_textdomain("tribes")
--
--    dirname = path.dirname(__file__)
--
--    wl.Descriptions():new_militarysite_type {
--       name = "atlanteans_castle",
--       descname = pgettext("atlanteans_building", "Castle"),
--       animation_directory = dirname,
--       icon = dirname .. "menu.png",
--       size = "big",
--
--       buildcost = {
--          planks = 4,
--          granite = 7,
--          log = 5,
--          spidercloth = 2,
--          diamond = 1,
--          quartz = 1
--       },
--       return_on_dismantle = {
--          planks = 2,
--          granite = 5,
--          log = 2
--       },
--
--       animations = {
--          idle = {
--             hotspot = { 91, 91 },
--          }
--       },
--
--       aihints = {
--          expansion = true,
--          fighting = true,
--          mountain_conqueror = true,
--          prohibited_till = 1400
--       },
--
--       max_soldiers = 12,
--       heal_per_second = 200,
--       conquers = 12,
--       prefer_heroes = true,
--
--       messages = {
--          occupied = pgettext("atlanteans_building", "Your soldiers have occupied your castle."),
--          aggressor = pgettext("atlanteans_building", "Your castle discovered an aggressor."),
--          attack = pgettext("atlanteans_building", "Your castle is under attack."),
--          defeated_enemy = pgettext("atlanteans_building", "The enemy defeated your soldiers at the castle."),
--          defeated_you = pgettext("atlanteans_building", "Your soldiers defeated the enemy at the castle.")
--       },
--    }
--
--    pop_textdomain()

push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_militarysite_type {
   name = "atlanteans_castle",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Castle"),
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

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 91, 91 },
      }
   },

   aihints = {
      expansion = true,
      fighting = true,
      mountain_conqueror = true,
      prohibited_till = 1400
   },

   max_soldiers = 12,
   heal_per_second = 200,
   conquers = 12,
   prefer_heroes = true,

   messages = {
      -- TRANSLATORS: Message sent by an Atlantean military site
      occupied = pgettext("atlanteans_building", "Your soldiers have occupied your castle."),
      -- TRANSLATORS: Message sent by an Atlantean military site
      aggressor = pgettext("atlanteans_building", "Your castle discovered an aggressor."),
      -- TRANSLATORS: Message sent by an Atlantean military site
      attack = pgettext("atlanteans_building", "Your castle is under attack."),
      -- TRANSLATORS: Message sent by an Atlantean military site
      defeated_enemy = pgettext("atlanteans_building", "The enemy defeated your soldiers at the castle."),
      -- TRANSLATORS: Message sent by an Atlantean military site
      defeated_you = pgettext("atlanteans_building", "Your soldiers defeated the enemy at the castle.")
   },
}

pop_textdomain()
