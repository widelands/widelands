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
p = wl.game.Player(1)
a = { "Hallo", "Welt" }
c = { func = function(a) return "I say " .. a .. "!" end }
field = wl.map.Field(32,34)
tree = wl.map.create_immovable("tree3", field)
removed_tree = wl.map.create_immovable("tree4", wl.map.Field(34,34))
removed_tree:remove()
corout = coroutine.create(function() coroutine.yield("What cool is that?") end)
objective = p:add_objective("lumber", "House", "Boat!")
objective.done = true

p:send_message("dummy msg1", "dummy msg 1")
msg = p:send_message("hello nice", "World", {sender="blah", loc = field })

-- ========================
-- Test after unpersisting 
-- ========================
function check_persistence()
coroutine.yield(wl.game.get_time() + 2000)

use("map", "lunit")
lunit.import "assertions"

print("###################### CHECKING FOR CORRECT PERSISTENCE")
assert_equal(1, p.number)
assert_equal(my_name, "SirVer")

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

assert_table(objective)
assert_equal("lumber", objective.name)
assert_equal("House", objective.title)
assert_equal("Boat!", objective.body)
assert_equal(true, objective.done)

assert_table(msg)
assert_equal("hello nice", msg.title)
assert_equal("World", msg.body)
assert_equal("blah", msg.sender)
assert_equal(field, msg.location)


print("################### ALL TEST PASS!")
wl.debug.exit()
end


-- ==========
-- Main Code 
-- ==========
-- This starts the test routine, saves the game and exits.
-- Loading the saved game will check that all objects are 
-- correctly unpersisted
wl.game.run_coroutine(coroutine.create(check_persistence))
wl.debug.save("lua_persistence")
wl.debug.exit()

