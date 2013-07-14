-- =======================================================================
--                           LOADING/SAVING TESTS
-- =======================================================================
-- This tests saving and loading of various Lua objects in the global
-- environment.
--
-- To run this test, go to the top level directory, set the path to your
-- widelands binary and run:
--
-- $PATH_TO_WIDELANDS_BINARY --datadir="." --nozip --scenario=test/lua/persistence.wmf && \
--   $PATH_TO_WIDELANDS_BINARY --datadir="." --loadgame=~/.widelands/save/lua_persistence.wgf

-- NOCOM(#cghislai): this should use your new function to make sure that it is actually called. Also, you should
-- save from inside a coroutine and make sure that this is really possible AND that the coroutine itself is persistet
-- properly.

-- ====================
-- Test Data to persist
-- ====================
use("aux", "set")
use("map", "lunit")
game = wl.Game()

function check_coroutine()
my_name = "SirVer"

game:save("lua_coroutine_persistence")

end


-- ==========
-- Main Code
-- ==========
-- This starts the test routine, saves the game and exits.
-- Loading the saved game will check that all objects are
-- correctly unpersisted



game:launch_coroutine(coroutine.create(check_coroutine))

wl.ui.MapView():close()

