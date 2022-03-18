push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_ware_type {
   name = "chisel",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Chisel"),
   icon = dirname .. "menu.png",

   animation_directory = dirname,
   animations = { idle = { hotspot = {6, 5}}},
}

pop_textdomain()
