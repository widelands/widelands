push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_ware_type {
   name = "machete",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Machete"),
   icon = dirname .. "menu.png",

   animation_directory = dirname,
   animations = { idle = { hotspot = {11, 5}}},
}

pop_textdomain()
