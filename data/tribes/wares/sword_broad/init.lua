dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "sword_broad",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Broadsword"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         hotspot = { 12, 9 },
      }
   },
}
