dirname = "tribes/buildings/warehouses/empire/warehouse/"

push_textdomain("scenario_emp04.wmf")

tribes:new_warehouse_type {
   name = "empire_temple_of_vesta",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Temple of Vesta"),
   helptext_script = path.dirname(__file__) .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   return_on_dismantle = {
      planks = 1,
      granite = 2,
      marble = 2,
      marble_column = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 58, 55  },
      },
   },

   aihints = {},

   heal_per_second = 220,
   conquers = 7,
}
pop_textdomain()
