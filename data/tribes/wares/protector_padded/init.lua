push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_ware_type {
   name = "protector_padded",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Padded Protector"),
   icon = dirname .. "menu.png",

   animation_directory = dirname,
   animations = { idle = { hotspot = {6, 8}}}
}

pop_textdomain()
