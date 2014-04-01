-- =======================================================================
--                           LOADING/SAVING TESTS
-- =======================================================================
-- This tests saving and loading of various Lua objects in the global
-- environment.

include "scripting/lunit.lua"

include "scripting/set.lua"

function save_coroutine()
   my_name = "SirVer"
   pi = 3.1415
   eight = 8
   is_true = true
   is_false = false

   game = wl.Game()
   p = game.players[1]
   map = game.map
   a = { "Hallo", "Welt" }
   c = { func = function(a) return "I say " .. a .. "!" end }
   field = map:get_field(32,34)
   tree = map:place_immovable("tree3", field)
   removed_tree = map:place_immovable("tree4", map:get_field(34,34))
   removed_tree:remove()

   corout = coroutine.create(function()
      local a = 100
      coroutine.yield("What cool is that?")
      coroutine.yield(a)
   end)

   objective = p:add_objective("lumber", "House", "Ship!")
   objective.done = true

   p:send_message("dummy msg1", "dummy msg 1")
   msg = p:send_message("hello nice", "World", {sender="blah", field = field })
   player_slot = map.player_slots[1]

   myset = Set:new{
      map:get_field(10,10), map:get_field(10,10), map:get_field(10,11)
   }

   mapview = wl.ui.MapView()
   mapview.viewpoint_x = 10
   mapview.viewpoint_y = 40
   mapview.statistics = false
   mapview.census = true

   game:save("lua_persistence")
   print("Save requested\n");

   -- Stick around, so that we are needed to get loaded too.
   coroutine.yield(wl.Game().time + 8000)
end

function check_coroutine()
   -- Sleep till the save routine has done its job.
   coroutine.yield(wl.Game().time + 2000)

   print("###################### CHECKING FOR CORRECT PERSISTENCE")
   assert_equal("SirVer", my_name)
   assert_equal(3.1415, pi)
   assert_equal(8, eight)
   assert_equal(true, is_true)
   assert_equal(false, is_false)

   assert_equal(1, p.number)

   assert_table(a)
   assert_equal(a[1], "Hallo")
   assert_equal(a[2], "Welt")

   assert_table(c)
   assert_function(c.func)
   assert_equal("I say zonk!", c.func("zonk"))

   assert_equal("tree3", tree.name)

   assert_equal(32, field.x)
   assert_equal(34, field.y)
   assert_equal(tree, field.immovable)

   assert_thread(corout)
   _,rv = coroutine.resume(corout)
   assert_equal("What cool is that?", rv)
   _,rv = coroutine.resume(corout)
   assert_equal(100, rv)

   assert_table(objective)
   assert_equal("lumber", objective.name)
   assert_equal("House", objective.title)
   assert_equal("Ship!", objective.body)
   assert_equal(true, objective.done)

   assert_table(msg)
   assert_equal("hello nice", msg.title)
   assert_equal("World", msg.body)
   assert_equal("blah", msg.sender)
   assert_equal(field, msg.field)

   assert_table(map)
   assert_equal(64, map.width)
   assert_equal(64, map.height)

   assert_table(player_slot)
   assert_equal("barbarians", player_slot.tribe_name)
   assert_equal("Player 1", player_slot.name)
   assert_equal(player_slot.name, map.player_slots[1].name)
   assert_equal(player_slot.tribe_name, map.player_slots[1].tribe_name)

   assert_equal(2, myset.size)
   assert_true(myset:contains(map:get_field(10,10)))
   assert_true(myset:contains(map:get_field(10,11)))

   mapview = wl.ui.MapView()
   assert_equal(mapview.viewpoint_x, 10)
   assert_equal(mapview.viewpoint_y, 40)
   assert_equal(false, mapview.statistics)
   assert_equal(true, mapview.census)

   print("# All Tests passed.")

   wl.ui.MapView():close()
end


-- ==========
-- Main Code
-- ==========
-- This starts the test routine, saves the game and exits.
-- Loading the saved game will check that all objects are
-- correctly unpersisted
wl.Game():launch_coroutine(coroutine.create(check_coroutine))
wl.Game():launch_coroutine(coroutine.create(save_coroutine))
