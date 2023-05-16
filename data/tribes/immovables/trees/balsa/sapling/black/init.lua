push_textdomain("tribes")

dirname = path.dirname(__file__) .. "../"

include(dirname .. "../terrain_affinity.lua")
include(dirname .. "../spritesheets.lua")

wl.Descriptions():new_immovable_type{
   name = "balsa_black_amazons_sapling",
   -- TRANSLATORS:  This is an Amazon rare tree.
   descname = _("Balsa Tree (Sapling)"),
   size = "small",
   terrain_affinity = terrain_affinity_black,
   programs = {
      main = {
         "animate=idle duration:55s",
         "remove=chance:13.67%",
         "grow=balsa_black_amazons_pole",
      },
   },
   icon = dirname .. "menu_sapling.png",
   animation_directory = dirname,
   spritesheets = spritesheet_sapling
}

pop_textdomain()
