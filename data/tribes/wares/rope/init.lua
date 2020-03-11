dirname = path.dirname(__file__)

animations = {}
add_animation(animations, "idle", dirname, "idle", {10, 8})

tribes:new_ware_type {
   msgctxt = "ware",
   name = "rope",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Rope"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {},
   preciousness = {
      amazons = 2
   },

   animations = animations,
}
