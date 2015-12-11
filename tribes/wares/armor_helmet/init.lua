dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "empire_ware",
   name = "armor_helmet",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("empire_ware", "Helmet"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {
      empire = 30
   },
   preciousness = {
      empire = 2
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 5, 10 },
      },
   }
}
