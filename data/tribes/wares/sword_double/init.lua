dirname = path.dirname(__file__)

animations = {}
add_animation(animations, "idle", dirname, "idle", {12, 9})

tribes:new_ware_type {
   msgctxt = "ware",
   name = "sword_double",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Double-edged Sword"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {
      frisians = 2
   },
   preciousness = {
      frisians = 3
   },

   animations = animations,
}
