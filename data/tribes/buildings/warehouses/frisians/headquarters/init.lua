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

   animations = {
      idle = {
         pictures = path.list_files (dirname .. "idle_??.png"),
         hotspot = {119, 136},
         fps = 10,
      },
   },

   aihints = {},

   heal_per_second = 220,
   conquers = 9,
}
