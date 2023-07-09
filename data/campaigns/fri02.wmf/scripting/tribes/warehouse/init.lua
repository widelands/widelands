local dirname = "campaigns/fri02.wmf/" .. path.dirname (__file__)

push_textdomain("scenario_fri02.wmf")

wl.Descriptions():new_warehouse_type {
   name = "frisians_warehouse_empire",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Empire Warehouse"),
   icon = dirname .. "menu.png",
   size = "medium",

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 58, 58 },
      },
   },

   aihints = {},
   heal_per_second = 170,
}
pop_textdomain()
