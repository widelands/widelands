dirname = path.dirname(__file__)

tribes:new_dismantlesite_type {
   msgctxt = "building",
   name = "dismantlesite",
   -- TRANSLATORS: This is a name used in lists of buildings for buildings being taken apart
   descname = pgettext("building", "Dismantle Site"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 5, 5 },
      },
   },

   aihints = {},
}
