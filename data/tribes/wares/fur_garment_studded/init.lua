push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_ware_type {
   name = "fur_garment_studded",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Studded Fur Garment"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         hotspot = { 8, 10 },
      }
   },
}

pop_textdomain()
