push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_ware_type {
   name = "chocolate",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Chocolate"),
   icon = dirname .. "menu.png",

   animation_directory = dirname,
   animations = { idle = { hotspot = {3, 8}}}
}

pop_textdomain()
