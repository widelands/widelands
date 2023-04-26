push_textdomain("tribes")

dirname = path.dirname(__file__) .. "../"

include(dirname .. "../terrain_affinity.lua")
include(dirname .. "../spritesheets.lua")


wl.Descriptions():new_immovable_type{
   name = "rubber_desert_amazons_sapling",
   -- TRANSLATORS:  This is an amazon rare tree.
   descname = _("Rubber Tree (Sapling)"),
   size = "small",
   terrain_affinity = terrain_affinity_desert,
   programs = {
      main = {
         "animate=idle duration:55s",
         "remove=chance:13.67%",
         "grow=rubber_desert_amazons_pole",
      },
   },
   icon = dirname .. "menu_sapling.png",
   animation_directory = dirname,
   spritesheets = spritesheet_sapling
}

pop_textdomain()
