dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 8, 15 },
      fps = 10,
      sound_effect = {
         -- Sound files with numbers starting from 10 are generating silence.
         path = "sound/animals/wolf",
      },
   },
}

add_directional_animation(animations, "walk", dirname, "walk", {19, 19}, 20)

world:new_critter_type{
   name = "wolf",
   descname = _ "Wolf",
   editor_category = "critters_carnivores",
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   animations = animations,
}
