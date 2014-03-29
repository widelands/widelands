dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.glob(dirname, "duck_\\d+.png"),
      -- NOCOM(#sirver): add support for this into the engine.
      sound_effects = {
         [0] = path.glob(dirname, "duck_\\d+.ogg")
      },
      player_color_masks = {},
      hotspot = { 5, 7 },
      fps = 1,
   },
}
add_walking_animations(animations, dirname, "duck_walk", {5, 10}, 1)

world:new_critter_type{
   name = "duck",
   descname = _ "Duck",
   swimming = true,
   attributes = {},
   programs = {
      remove = { "remove" },
   },
   animations = animations,
}
