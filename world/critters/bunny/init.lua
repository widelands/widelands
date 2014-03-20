dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = { dirname .. "/menu.png" },
      -- NOCOM(#sirver): reconsider this key - maybe just look for _pc.png instead?
      player_color_masks = {},
      hotspot = { 4, 9 },
      fps = 1,
   },
}
add_walking_animations(animations, dirname, "bunny_walk", {5, 9})

world:new_critter_type{
   name = "bunny",
   descname = _ "Bunny",
   swimming = false,
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   animations = animations,
}
