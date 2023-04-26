push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_ware_type {
   name = "reed",
   -- TRANSLATORS: This is a ware name used in lists of wares. Used for thatching roofs and as a fiber for weaving cloth, fishing nets etc.
   descname = pgettext("ware", "Reed"),
   icon = dirname .. "menu.png",

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 3, 12 },
      },
   }
}

pop_textdomain()
