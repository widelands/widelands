push_textdomain("tribes")

dirname = path.dirname(__file__) .. "../"

include(dirname .. "../terrain_affinity.lua")
include(dirname .. "../spritesheets.lua")

descriptions:new_immovable_type{
   name = "ironwood_winter_amazons_mature",
   -- TRANSLATORS:  This is an amazon rare tree.
   descname = _ "Ironwood Tree (Mature)",
   size = "small",
   terrain_affinity = terrain_affinity_winter,
   programs = {
      main = {
         "animate=idle duration:1m12s",
         "remove=chance:7.42%",
         "grow=ironwood_winter_amazons_old",
      },
   },
   icon = dirname .. "menu_mature.png",
   animation_directory = dirname,
   spritesheets = spritesheet_mature
}

pop_textdomain()
