
world = wl.World()

include "world/resources.lua"
include "world/terrains.lua"

include "world/immovables/manmade/snowman/init.lua"
include "world/immovables/trees/aspen_summer_old/init.lua"

-- Adds 6 animations for each walking direction into 'table'. The pictures are
-- searched for in 'dirname'. All files should look like this
-- 'basename'_(e|ne|se|sw|w|nw)_\d+.png. 'hotspot' is the hotspot for blitting.
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

include "world/critters/bunny/init.lua"
include "world/critters/brownbear/init.lua"
