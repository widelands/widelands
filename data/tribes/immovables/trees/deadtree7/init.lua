dirname = path.dirname(__file__)


tribes:new_immovable_type{
   msgctxt = "immovable",
   name = "deadtree7",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Dead Tree",
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle duration:20s",
         "remove=success:16",
      },
   },
   icon = dirname .. "menu.png",
   animation_directory = dirname,
   animations = { idle = { hotspot = {2, 72}}}
}
