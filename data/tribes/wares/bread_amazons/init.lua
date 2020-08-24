dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "amazons_ware",
   name = "amazons_bread",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("amazons_ware", "Bread"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = {7, 6},
      },
   }
}
