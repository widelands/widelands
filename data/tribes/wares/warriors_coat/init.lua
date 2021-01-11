push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_ware_type {
   name = "warriors_coat",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Warrior’s Coat"),
   icon = dirname .. "menu.png",

   animation_directory = dirname,
   animations = { idle = { hotspot = {6, 8}}}
}

pop_textdomain()
