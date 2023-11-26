push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_ware_type {
   name = "barbarians_bread",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("barbarians_ware", "Pitta Bread"),
   icon = dirname .. "menu.png",

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 9, 9 },
      },
   }
}

pop_textdomain()
