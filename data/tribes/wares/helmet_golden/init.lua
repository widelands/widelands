dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "frisians_ware",
   name = "helmet_golden",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("frisians_ware", "Golden Helmet"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = {8, 6},
      },
   }
}
