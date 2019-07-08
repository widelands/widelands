dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      sound_effect = {
         -- Sound files with numbers starting for 10 are generating silence. Remove when we move the sound triggering to programs
         path = "sound/animals/coyote",
      },
      hotspot = { 10, 13 },
      fps = 10,
   },
}

add_directional_animation(animations, "walk", dirname, "walk", {11, 14}, 20)

world:new_critter_type{
   name = "fox",
   descname = _ "Fox",
   editor_category = "critters_carnivores",
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   animations = animations,
}
