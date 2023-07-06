push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_ware_type {
   name = "rubber",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Rubber"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         hotspot = { 6, 6 },
      }
   },
}

pop_textdomain()
