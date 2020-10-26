push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_ware_type {
   name = "reed",
   -- TRANSLATORS: This is a ware name used in lists of wares. Used for thatching roofs and as a fiber for weaving cloth, fishing nets etc.
   descname = pgettext("ware", "Reed"),
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 3, 12 },
      },
   }
}

pop_textdomain()
