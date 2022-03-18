push_textdomain("tribes")

dirname = path.dirname(__file__) .. "../"

include(dirname .. "../terrain_affinity.lua")
include(dirname .. "../spritesheets.lua")

wl.Descriptions():new_immovable_type{
   name = "ironwood_black_amazons_mature",
   -- TRANSLATORS:  This is an amazon rare tree.
   descname = _("Ironwood Tree (Mature)"),
   size = "small",
   terrain_affinity = terrain_affinity_black,
   programs = {
      main = {
         "animate=idle duration:1m12s",
         "remove=chance:7.42%",
         "grow=ironwood_black_amazons_old",
      },
   },
   icon = dirname .. "menu_mature.png",
   animation_directory = dirname,
   spritesheets = spritesheet_mature
}

pop_textdomain()
