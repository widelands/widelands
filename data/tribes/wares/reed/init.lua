dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "reed",
   -- TRANSLATORS: This is a ware name used in lists of wares. Used for thatching roofs and as a fiber for weaving cloth, fishing nets etc.
   descname = pgettext("ware", "Reed"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 3, 12 },
      },
   }
}
