-- RST
-- .. _lua_tribes_buildings_warehouses:
--
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
-- The warehouse will also need its :ref:`help texts <lua_tribes_tribes_helptexts>`,
-- which are defined in ``data/tribes/initialization/<tribe_name>/units.lua``
--
-- .. function:: new_warehouse_type{table}
--
--    This function adds the definition of a warehouse building to the engine.
--
--    :arg table: This table contains all the data that the game engine will add to this building.
--                In addition to the :ref:`lua_tribes_buildings_common`, it contains the following entries:
--
--    **size**
--        *Mandatory*. In addition to the common size values ``"small"``,
--        ``"medium"``, or ``"big"``, warehouses can also have size ``"port"``
--        for defining a port building.
--
--    **heal_per_second**
--        *Mandatory*. The number of health points that a garrisoned soldier will heal each second.
--
--    **conquers**
--        *Optional. Default:* ``0``. The conquer radius for this building.
--
--    **destructible**
--        *Optional. Default:* ``true``. Set this to ``false`` for headquarters.
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
--    wl.Descriptions():new_warehouse_type {
--       name = "atlanteans_headquarters",
--       descname = pgettext("atlanteans_building", "Headquarters"),
--       animation_directory = dirname,
--       icon = dirname .. "menu.png",
--       size = "big",
--       destructible = false,
--
--       animations = {
--          idle = {
--             hotspot = { 81, 110 },
--          },
--       },
--
--       aihints = {},
--
--       heal_per_second = 220,
--       conquers = 9,
--    }
--
--    pop_textdomain()

push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_warehouse_type {
   name = "atlanteans_headquarters",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Headquarters"),
   icon = dirname .. "menu.png",
   size = "big",
   destructible = false,

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 81, 110 },
      },
   },

   aihints = {},

   heal_per_second = 220,
   conquers = 9,
}

pop_textdomain()
