push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_ware_type {
   name = "selected_firestones",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Selected Firestones"),
   icon = dirname .. "menu.png",

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 5, 6 },
      },
   }
}

pop_textdomain()
