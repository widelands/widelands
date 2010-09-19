-- =======================================================================
--                           LOADING/SAVING TESTS                           
-- =======================================================================
-- This tests saving and loading of various Lua objects in the global 
-- environment.
--
-- To run this test use:
-- ./widelands --nozip --scenario=src/scripting/test/persistence.wmf && 
--   ./widelands --loadgame=~/.widelands/save/lua_persistence.wgf

-- ====================
-- Test Data to persist 
-- ====================
my_name = "SirVer"
pi = 3.1415
eight = 8
is_true = true
is_false = false

p = wl.Game().players[1]
a = { "Hallo", "Welt" }
c = { func = function(a) return "I say " .. a .. "!" end }
field = wl.Map():get_field(32,34)
tree = wl.Map():place_immovable("tree3", field)
removed_tree = wl.Map():place_immovable("tree4", wl.Map():get_field(34,34))
removed_tree:remove()
corout = coroutine.create(function()
   local a = 100
   coroutine.yield("What cool is that?")
   coroutine.yield(a)
end)
objective = p:add_objective("lumber", "House", "Boat!")
objective.done = true

p:send_message("dummy msg1", "dummy msg 1")
msg = p:send_message("hello nice", "World", {sender="blah", field = field })
game = wl.Game()
map = wl.Map()
player_slot = map.player_slots[1]

-- ========================
-- Test after unpersisting 
-- ========================
function check_persistence()
coroutine.yield(wl.Game().time + 2000)

use("map", "lunit")
lunit.import "assertions"

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
assert_equal("Boat!", objective.body)
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
assert_equal("barbarians", player_slot.tribe)
assert_equal("Player 1", player_slot.name)
assert_equal(player_slot.name, map.player_slots[1].name)
assert_equal(player_slot.tribe, map.player_slots[1].tribe)

print("################### ALL TEST PASS!")
wl.debug.exit()
end


-- ==========
-- Main Code 
-- ==========
-- This starts the test routine, saves the game and exits.
-- Loading the saved game will check that all objects are 
-- correctly unpersisted
game = wl.Game()
game:launch_coroutine(coroutine.create(check_persistence))
game:save("lua_persistence")
wl.debug.exit()

