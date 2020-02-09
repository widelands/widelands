dirname = path.dirname(__file__)

animations = {}
add_animation(animations, "idle", dirname, "idle", {8, 10})

tribes:new_ware_type {
   msgctxt = "ware",
   name = "fur_garment",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Fur Garment"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {
      frisians = 30,
   },
   preciousness = {
      frisians = 3,
   },

   animations = animations,
}
