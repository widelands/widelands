dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "fur_garment_old",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Old Fur Garment"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         hotspot = { 8, 10 },
      }
   },
}
