dirname = "tribes/buildings/warehouses/empire/headquarters/"

tribes:new_warehouse_type {
   msgctxt = "empire_building",
   name = "HQ_atterdag",
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

   outputs = {
      "granite",
      "marble",
      "iron_ore",
      "gold_ore",
      "log"
   },

   aihints = {},

   heal_per_second = 220,
   conquers = 9,
}
