push_textdomain("tribes")

local dirname = path.dirname(__file__) .. "../"

include(dirname .. "../terrain_affinity.lua")
include(dirname .. "../spritesheets.lua")

wl.Descriptions():new_immovable_type{
   name = "ironwood_amazons_old",
   -- TRANSLATORS:  This is an Amazon rare tree.
   descname = _("Ironwood Tree (Old)"),
   -- TRANSLATORS:  This is an Amazon rare tree.
   species = _("Ironwood Tree"),
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:25m35s",
         "transform=deadtree7 chance:12.89%",
         "seed=ironwood_amazons_sapling proximity:19.53%",
      },
      fall = {
         "animate=fall duration:1s",
         "remove=",
      },
   },
   icon = dirname .. "menu_old.png",
   animation_directory = dirname,
   spritesheets = spritesheet_old
}

pop_textdomain()
