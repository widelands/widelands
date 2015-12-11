dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "armor",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Armor"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {
      empire = 1
   },
   preciousness = {
      empire = 0
   },
   animations = {
      idle = {
         template = "idle",
         directory = dirname,
         hotspot = { 3, 11 },
      },
   }
}
