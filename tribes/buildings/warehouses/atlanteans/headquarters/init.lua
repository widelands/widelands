dirname = path.dirname(__file__)

tribes:new_warehouse_type {
   msgctxt = "atlanteans_building",
   name = "atlanteans_headquarters",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Headquarters"),
   directory = dirname,
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
