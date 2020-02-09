dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "beer",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Beer"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {
      barbarians = 15,
      empire = 15,
      frisians = 15
   },
   preciousness = {
      barbarians = 2,
      empire = 5,
      frisians = 3
   },
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 5, 9 },
      },
   }
}
