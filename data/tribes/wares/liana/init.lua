push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_ware_type {
   name = "liana",
   -- TRANSLATORS: This is a ware name used in lists of wares. Used to make ropes.
   descname = pgettext("ware", "Liana"),
   icon = dirname .. "menu.png",

   animation_directory = dirname,
   animations = { idle = { hotspot = {2, 10}}}
}

pop_textdomain()
