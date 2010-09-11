-- =======================================================================
--                    Start conditions for Share Kingdom
-- =======================================================================

use("aux", "coroutine")
use("aux", "infrastructure")

set_textdomain("tribe_barbarians")

return {
   name = _ "Share Kingdom",
   func = function(p)

   -- General behaviour until now:
   -- if a player selects this option, the script checks if a player with smaller
   -- playernumber is in the same team, as the player and if yes, combines these
   -- two players. Else the player will be handled similiar to the
   -- "headquarters medium" initialization.

   -- what the script does not do:
   -- it neither checks for team mates with higher player numbers (that did not
   -- select "share kingdom" nor does it change the behaviour of the initiali-
   -- zation of the other mates.
   -- It does not care about savegame loading - this is one of the bigger things
   -- to implement.

   -- there are two possibilities:
   -- 1) The player is the team member with the smallest player number (or alone)
   -- 2) The player is in a team with another player who's number is smaller
   --    than p.number and the other player has the same tribe

   local sf = p.starting_field
   -- Do not call twice for teammate
   p:allow_workers("all")

   local teammate = nil
   for i=1,p.number do
      if (p.team > 0) then
         if pcall(wl.game.Player, i) then
            if (p.team == wl.game.Player(i).team) then
               teammate = wl.game.Player(i)
               break;
            end
         end
      end
   end
   if (teammate == nil) then
      teammate = p
   else
      -- Start a new coroutine that checks for defeated players
      run(function()
         while true do
            p:switchplayer(teammate.number)
            sleep(1000)
         end
      end)
   end

   if (teammate.tribe == "barbarians") then
      prefilled_buildings(teammate, { "headquarters_interim", sf.x, sf.y,
         wares = {
            axe = 6,
            bakingtray = 2,
            blackwood = 32,
            cloth = 5,
            coal = 12,
            fire_tongs = 2,
            fish = 6,
            fishing_rod = 2,
            gold = 4,
            grout = 12,
            hammer = 12,
            hunting_spear = 2,
            iron = 12,
            ironore = 5,
            kitchen_tools = 4,
            meal = 4,
            meat = 6,
            pick = 14,
            pittabread = 8,
            ration = 12,
            raw_stone = 40,
            scythe = 6,
            shovel = 4,
            snack = 3,
            thatchreed = 24,
            trunk = 80,
         },
         workers = {
            blacksmith = 2,
            brewer = 1,
            builder = 10,
            burner = 1,
            carrier = 40,
            ferner = 1,
            geologist = 4,
            ["lime-burner"] = 1,
            lumberjack = 3,
            miner = 4,
            ranger = 1,
            stonemason = 2,
            ox = 5,
         },
         soldiers = {
            [{0,0,0,0}] = 45,
         }
      })
   else
      if (teammate.tribe == "empire") then
         prefilled_buildings(teammate, { "headquarters", sf.x, sf.y,
            wares = {
               helm = 4,
               wood_lance = 5,
               axe = 6,
               bakingtray = 2,
               basket = 1,
               bread = 8,
               cloth = 5,
               coal = 12,
               fire_tongs = 2,
               fish = 6,
               fishing_rod = 2,
               flour = 4,
               gold = 4,
               grape = 4,
               hammer = 12,
               hunting_spear = 2,
               iron = 12,
               ironore = 5,
               kitchen_tools = 4,
               marble = 25,
               marblecolumn = 6,
               meal = 4,
               meat = 6,
               pick = 14,
               ration = 12,
               saw = 3,
               scythe = 5,
               shovel = 6,
               stone = 40,
               trunk = 30,
               water = 12,
               wheat = 4,
               wine = 8,
               wood = 45,
               wool = 2,
            },
            workers = {
               armoursmith = 1,
               brewer = 1,
               builder = 10,
               burner = 1,
               carrier = 40,
               geologist = 4,
               lumberjack = 3,
               miner = 4,
               stonemason = 2,
               toolsmith = 2,
               weaponsmith = 1,
               donkey = 5,
            },
            soldiers = {
               [{0,0,0,0}] = 45,
            }
         })
      else
         if (teammate.tribe == "atlanteans") then
            prefilled_buildings(teammate, { "headquarters", sf.x, sf.y,
               wares = {
                  diamond = 7,
                  ironore = 5,
                  quartz = 9,
                  stone = 50,
                  spideryarn = 9,
                  trunk = 20,
                  coal = 12,
                  gold = 4,
                  goldyarn = 6,
                  iron = 8,
                  planks = 45,
                  spidercloth = 5,
                  blackroot = 5,
                  blackrootflour = 12,
                  bread = 8,
                  corn = 5,
                  cornflour = 12,
                  fish = 3,
                  meat = 3,
                  smoked_fish = 6,
                  smoked_meat = 6,
                  water = 12,
                  bakingtray = 2,
                  bucket = 2,
                  fire_tongs = 2,
                  fishing_net = 4,
                  hammer = 11,
                  hunting_bow = 1,
                  milking_tongs = 2,
                  hook_pole = 2,
                  pick = 12,
                  saw = 9,
                  scythe = 4,
                  shovel = 9,
                  tabard = 5,
                  light_trident = 5,
               },
               workers = {
                  armoursmith = 1,
                  blackroot_farmer = 1,
                  builder = 10,
                  burner = 1,
                  carrier = 40,
                  fish_breeder = 1,
                  geologist = 4,
                  miner = 4,
                  sawyer = 1,
                  stonecutter = 2,
                  toolsmith = 2,
                  weaponsmith = 1,
                  woodcutter = 3,
                  horse = 5,
               },
               soldiers = {
                  [{0,0,0,0}] = 35,
               }
            })
         else
            -- no idea what tribe this is, so we better do nothing
         end
      end
  end
end,
}

