-- =================================
-- Island Hopping Fun Map Scripting 
-- =================================

use("aux", "coroutine")
use("aux", "infrastructure")
use("aux", "objective_utils")


-- ===================
-- Constants & Config
-- ===================
-- TODO: parse the scripts and create a new catalog for it
-- set_textdomain("scenario_atl01.wmf")

game = wl.Game()
map = game.map


-- Reveal the whole map for all players
function reveal_everything_for_everybody()
   local fields = {}
   local i = 1
   for x=0, map.width-1 do
      for y=0, map.height-1 do
         fields[i] = map:get_field(x,y)
         i = i + 1
      end
   end

   for idx, p in ipairs(game.players) do
      p:reveal_fields(fields)
   end
end

-- Place headquarters for all players
function place_headquarters()
   for idx, p in ipairs(game.players) do
      local sf = map.player_slots[p.number].starting_field

      prefilled_buildings(p, {"headquarters", sf.x, sf.y, 
         wares = {
            trunk = 10,
            planks = 10
         },
         workers = {
            builder = 3,
         },
         soldiers = {
            [{0,0,0,0}] = 10,
         }
      })
   end
end

-- Game initializations
function initialize()
   reveal_everything_for_everybody()
   place_headquarters()

end




run(initialize)
