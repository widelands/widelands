-- RST
-- Headquarters, Warehouses and Ports
-- ----------------------------------
--
-- Warehouses are a type of building where wares and workers are being stored
-- that aren't currently needed at another building.
-- Warehouses can also be one of two special types: a headquarters or a port.
-- A *headquarters* is also the tribe's main building.
-- A *port* can only be built on a designated port space and allows transferring
-- wares by sea and starting expeditions.
--
-- Warehouses are defined in
-- ``data/tribes/buildings/warehouses/<tribe_name>/<building_name>/init.lua``.
-- The building will also need its help texts, which are defined in
-- ``data/tribes/buildings/warehouses/<tribe_name>/<building_name>/helptexts.lua``

dirname = path.dirname(__file__)

-- RST
-- .. function:: new_warehouse_type{table}
--
--    This function adds the definition of a warehouse building to the engine.
--
--    :arg table: This table contains all the data that the game engine will add to this building.
--                In addition to the :ref:`lua_tribes_buildings_common`, it contains the following entries:
--
--    **heal_per_second**
--        *Mandatory*. The number of health points that a garrisoned soldier will heal each second.
--
--    **conquers**
--        *Optional*. The conquer radius for this building.
--
tribes:new_warehouse_type {
   msgctxt = "atlanteans_building",
   name = "atlanteans_headquarters",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Headquarters"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "big",
   destructible = false,

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 81, 110 },
      },
   },

   aihints = {},

   heal_per_second = 220,
   conquers = 9,
}
