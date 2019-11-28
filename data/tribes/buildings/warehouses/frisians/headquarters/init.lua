dirname = path.dirname (__file__)

tribes:new_warehouse_type {
   msgctxt = "frisians_building",
   name = "frisians_headquarters",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Headquarters"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "big",
   destructible = false,

   spritesheets = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = {92, 102},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      }
   },

   aihints = {},

   heal_per_second = 220,
   conquers = 9,
}
