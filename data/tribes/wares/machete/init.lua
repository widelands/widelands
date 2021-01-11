push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_ware_type {
   name = "machete",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Machete"),
   icon = dirname .. "menu.png",

   animation_directory = dirname,
   animations = { idle = { hotspot = {11, 5}}},
}

pop_textdomain()
