dirname = path.dirname(__file__)

tribes:new_warehouse_type {
   msgctxt = "barbarians_building",
   name = "barbarians_headquarters",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Headquarters"),
   directory = dirname,
   icon = dirname .. "menu.png",
   size = "big",
   destructible = false,

   -- The Headquarters of the barbarians is from the apperance a stable military
   -- Building, fortified base and some towers made of blackwood. Some flags
   -- in the player colors may be present
   animations = {
      idle = {
         template = "idle_??",
         directory = dirname,
         hotspot = { 90, 96 },
         fps = 10
      },
   },

   aihints = {},

   heal_per_second = 220,
   conquers = 9,
}
