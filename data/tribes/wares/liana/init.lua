dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "liana",
   -- TRANSLATORS: This is a ware name used in lists of wares. Used to make ropes.
   descname = pgettext("ware", "Liana"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 3, 12 },
      },
   }
}
