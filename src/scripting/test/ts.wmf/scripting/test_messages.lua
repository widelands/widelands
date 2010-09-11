-- =======================================================================
--                          Testing Messages System                         
-- =======================================================================

messages_tests = lunit.TestCase("Messages")
function messages_tests:setup() 
   self.plr = wl.game.Player(1)
   for i,m in ipairs(self.plr.inbox) do
      m.status = "archived"
   end
end

function messages_tests:test_defaults() 
   local m = self.plr:send_message("Hallo", "World!")
   assert_equal("Hallo", m.title)
   assert_equal("World!", m.body)
   assert_equal("ScriptingEngine", m.sender)
   assert_equal(0, m.sent)
   assert_equal(0, m.sent)
   assert_equal(nil, m.duration)
   assert_equal(nil, m.field)
   assert_equal("new", m.status)
end
function messages_tests:test_status_read() 
   local m = self.plr:send_message("Hallo", "World!", {status="read"})
   assert_equal("read", m.status)
end
function messages_tests:test_status_archived() 
   local m = self.plr:send_message("Hallo", "World!", {status="archived"})
   assert_equal("archived", m.status)
end
function messages_tests:test_status_illegal() 
   assert_error("Illegal status!", function()
      self.plr:send_message("Hallo", "World!", {status="nono"})
   end)
end
function messages_tests:test_sender() 
   local m = self.plr:send_message("Hallo", "World!", {sender="i am you"})
   assert_equal("i am you", m.sender)
end
function messages_tests:test_field() 
   local f = wl.Map():get_field(23,28)
   local m = self.plr:send_message("Hallo", "World!", {field = f})
   assert_equal(f, m.field)
end
function messages_tests:test_duration() 
   local m = self.plr:send_message("Hallo", "World!", {duration = 2000})
   assert_equal(2000, m.duration)
end
function messages_tests:test_changing_status() 
   local m = self.plr:send_message("Hallo", "World!")
   m.status = "read"
   assert_equal("read", m.status)
   m.status = "archived"
   assert_equal("archived", m.status)
   m.status = "new"
   assert_equal("new", m.status)
   assert_error("Invalid message status!", function()
      m.status = "bluuuu"
   end)
end
function messages_tests:test_inbox()
   assert_equal(0, #self.plr.inbox)
   local m1 = self.plr:send_message("Hallo", "World!")
   assert_equal(1, #self.plr.inbox)
   local m2 = self.plr:send_message("Hallo", "World!")
   assert_equal(2, #self.plr.inbox)

   assert_equal(m1, self.plr.inbox[1])
   assert_equal(m2, self.plr.inbox[2])
end

