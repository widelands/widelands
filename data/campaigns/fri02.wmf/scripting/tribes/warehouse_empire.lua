dirname = "tribes/buildings/warehouses/empire/warehouse/"

tribes:new_warehouse_type {
   msgctxt = "frisians_building",
   name = "frisians_warehouse_empire",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Empire Warehouse"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 58, 55 },
      },
   },
   aihints = {},
   heal_per_second = 170,
}
