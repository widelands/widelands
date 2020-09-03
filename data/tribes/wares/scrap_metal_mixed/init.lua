push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_ware_type {
   name = "scrap_metal_mixed",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Mixed Scrap Metal"),
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 4, 10 },
      },
   }
}

pop_textdomain()
