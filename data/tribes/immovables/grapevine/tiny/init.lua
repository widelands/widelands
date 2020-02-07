dirname = path.dirname(__file__)

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "grapevine_tiny",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Grapevine (tiny)"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",
   attributes = { "field", "seed_grapes" },
   programs = {
      program = {
         "animate=idle 22000",
         "transform=grapevine_small",
      }
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 15, 18 },
      },
   }
}
