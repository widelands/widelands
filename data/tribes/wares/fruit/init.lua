dirname = path.dirname(__file__)

animations = {}
add_animation(animations, "idle", dirname, "idle", {4, 7})

tribes:new_ware_type {
   msgctxt = "ware",
   name = "fruit",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Fruit"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {},
   preciousness = {
      frisians = 1
   },

   animations = animations,
}
