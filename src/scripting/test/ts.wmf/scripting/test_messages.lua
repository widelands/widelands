-- =======================================================================
--                          Testing Messages System                         
-- =======================================================================

-- TODO: persistence tests of messages

messages_test_creation = lunit.TestCase("Messages creation")
function messages_test_creation:test_simple()
   m = wl.game.Player(1):send_message("Hallo", "World!")
   assert_equal("Hallo", m.title)
   assert_equal("World!", m.body)
   assert_equal("ScriptingEngine", m.sender)
end


