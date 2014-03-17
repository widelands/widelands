
function add_walking_animations(table, dirname, basename, hotspot)
   for idx, dir in ipairs{ "ne", "e", "se", "sw", "w", "nw" } do
      table["walk_" .. dir] = {
         pictures = path.glob(dirname, basename .. "_" .. dir .. "_*.png"),
         player_color_masks = {},
         hotspot = hotspot,
         fps = 5,
      }
   end
end

dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = { dirname .. "/menu.png" },
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
