dirname = path.dirname(__file__)

tribes:new_warehouse_type {
   msgctxt = "empire_building",
   name = "empire_headquarters",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Headquarters"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "big",
   destructible = false,

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 95, 109 },
      },
   },

   aihints = {},

   heal_per_second = 220,
   conquers = 9,
}
