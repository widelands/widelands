dirname = path.dirname(__file__)

animations = {}
add_animation(animations, "idle", dirname, "idle", {6, 6})

tribes:new_ware_type {
   msgctxt = "ware",
   name = "rubber",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Rubber"),
   helptext_script = dirname .. "helptexts.lua",
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
      amazons = 30
   },
   preciousness = {
      amazons = 40
   },

   animations = {
      idle = {
         hotspot = { 6, 6 },
      }
   },
}
