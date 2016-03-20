include "scripting/coroutine.lua"
include "scripting/infrastructure.lua"
include "scripting/lunit.lua"
include "scripting/ui.lua"

game = wl.Game()
map = game.map
p1 = game.players[1]
p2 = game.players[2]

function archive_messages()
   for idx, m in ipairs(p1.inbox) do
      m.status = "archived"
   end
end

function wait_for_message(title)
   archive_messages()
   sleep(5000)
   local old_speed = game.desired_speed
   game.desired_speed = 100 * 1000
   sleep(5000)
   while true do
      while #p1.inbox == 0 do
         sleep(313)
      end
      local message_title = p1.inbox[1].title
      archive_messages()
      if title == message_title then
         break
      end
   end
   game.desired_speed = old_speed
   sleep(5000)
end

function create_infrastructure(tower_x, tower_y)
   p1:allow_workers("all")
   p1.see_all = true

   prefilled_buildings(p2,
      { "barbarians_headquarters", 40, 13 },
      { "barbarians_tower", tower_x, tower_y,
        soldiers = { [{0,0,0,0}] = 1 },
      }
   )

   prefilled_buildings(p1,
      { "barbarians_headquarters", 21, 8,
         wares = {
            blackwood = 100,
            grout = 100,
            iron = 100,
            granite = 100,
            thatch_reed = 100,
            log = 100,
         },
         workers = {
            barbarians_builder = 1,
         },
      },
      { "barbarians_port", 13, 14,
         wares = {
            blackwood = 100,
            grout = 100,
            iron = 100,
            granite = 100,
            thatch_reed = 100,
            log = 100,
            gold = 100,
         },
         workers = {
            barbarians_builder = 1,
         },
      }
   )
   local ship = p1:place_ship(map:get_field(9, 13))

   local port = map:get_field(13, 14).immovable

   game.desired_speed = 100 * 1000
   port:start_expedition()
   wait_for_message("Expedition")
   ship.scouting_direction = "nw"

   return ship
end
