push_textdomain("tribes")

dirname = path.dirname(__file__) .. "../"

include(dirname .. "../terrain_affinity.lua")
include(dirname .. "../spritesheets.lua")

descriptions:new_immovable_type{
   name = "rubber_black_amazons_pole",
   -- TRANSLATORS:  This is an amazon rare tree.
   descname = _ "Rubber Tree (Pole)",
   size = "small",
   terrain_affinity = terrain_affinity_black,
   programs = {
      main = {
         "animate=idle duration:1m05s",
         "remove=chance:9.37%",
         "grow=rubber_black_amazons_mature",
      },
   },
   icon = dirname .. "menu_pole.png",
   animation_directory = dirname,
   spritesheets = spritesheet_pole
}

pop_textdomain()
