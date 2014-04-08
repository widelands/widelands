dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = { dirname .. "idle.png" },
      player_color_masks = {},
      hotspot = { 36, 86 }
   },
}

world:new_immovable_type{
   name = "stones8",
   descname = _ "Stones",
   size = "big",
   attributes = { "stone" },
   programs = {
      program = {
         "animate=idle",
         "transform=stones7"
      }
   },
   animations = animations
}
