dirname = path.dirname(__file__)

animations = {}
add_animation(animations, "idle", dirname, "idle", {6, 6})

tribes:new_ware_type {
   msgctxt = "ware",
   name = "cassavaroot",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Cassavaroot"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {},
   preciousness = {
      amazons = 10
   },

   animations = animations,
}
