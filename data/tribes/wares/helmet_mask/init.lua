push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_ware_type {
   name = "helmet_mask",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Mask"),
   icon = dirname .. "menu.png",

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 7, 10 },
      },
   }
}

pop_textdomain()
