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
         "animate=idle 20000",
         "remove=16",
      },
   },
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 23, 61 },
      },
   }
}
