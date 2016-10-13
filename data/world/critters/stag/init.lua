dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "stag_idle_??.png"),
      sound_effect = {
         -- Sound files with numbers starting for 10 are generating silence. Remove when we move the sound triggering to programs
         directory = "sound/animals",
         name = "stag",
      },
      hotspot = { 12, 26 },
      fps = 20,
   },
}
add_walking_animations(animations, "walk", dirname, "stag_walk", {25, 30}, 20)

world:new_critter_type{
   name = "stag",
   descname = _ "Stag",
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   animations = animations,
}
