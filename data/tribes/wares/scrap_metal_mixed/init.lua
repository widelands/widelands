push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_ware_type {
   name = "scrap_metal_mixed",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Mixed Scrap Metal"),
   icon = dirname .. "menu.png",

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 4, 10 },
      },
   }
}

pop_textdomain()
