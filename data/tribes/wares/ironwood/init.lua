dirname = path.dirname(__file__)

animations = {}
add_animation(animations, "idle", dirname, "idle", {8, 7})

tribes:new_ware_type {
   msgctxt = "ware",
   name = "ironwood",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Ironwood"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {
      amazons = 40
   },
   preciousness = {
      amazons = 50
   },

   animations = animations,
}
