dirname = "campaigns/emp04.wmf/" .. path.dirname (__file__)

push_textdomain("scenario_emp04.wmf")

wl.Descriptions():new_warehouse_type {
   name = "empire_temple_of_vesta",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Temple of Vesta"),
   icon = dirname .. "menu.png",
   size = "medium",

   return_on_dismantle = {
      planks = 1,
      granite = 2,
      marble = 2,
      marble_column = 1
   },

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 58, 58 }
      }
   },

   aihints = {},

   heal_per_second = 220,
   conquers = 7,
}
pop_textdomain()
