dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "fruit",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Fruit"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         hotspot = { 4, 7 },
      }
   },
}
