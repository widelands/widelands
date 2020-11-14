-- =======================================================================
--                          Testing Messages System
-- =======================================================================

messages_tests = lunit.TestCase("Messages")
function messages_tests:setup()
   for i,m in ipairs(player1.inbox) do
      m.status = "archived"
   end
end

function messages_tests:test_defaults()
   local m = player1:send_to_inbox("Hallo", "World!")
   assert_equal("Hallo", m.title)
   assert_equal("World!", m.body)
   assert_equal(0, m.sent)
   assert_equal(0, m.sent)
   assert_equal(nil, m.field)
   assert_equal("new", m.status)
end
function messages_tests:test_status_read()
   local m = player1:send_to_inbox("Hallo", "World!", {status="read"})
   assert_equal("read", m.status)
end
function messages_tests:test_status_archived()
   local m = player1:send_to_inbox("Hallo", "World!", {status="archived"})
   assert_equal("archived", m.status)
end
function messages_tests:test_status_illegal()
   assert_error("Illegal status!", function()
      player1:send_to_inbox("Hallo", "World!", {status="nono"})
   end)
end
function messages_tests:test_heading()
   local m = player1:send_to_inbox("Hallo", "World!", {heading="long heading"})
   assert_equal("long heading", m.heading)
end
function messages_tests:test_icon_name()
   local ware_description = game:get_ware_description("log")
   local m = player1:send_to_inbox("Hallo", "World!", {icon=ware_description.icon_name})
   assert_equal(ware_description.icon_name, m.icon_name)
end
function messages_tests:test_field()
   local f = map:get_field(23,28)
   local m = player1:send_to_inbox("Hallo", "World!", {field = f})
   assert_equal(f, m.field)
end
function messages_tests:test_changing_status()
   local m = player1:send_to_inbox("Hallo", "World!")
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
   assert_equal(0, #player1.inbox)
   local m1 = player1:send_to_inbox("Hallo", "World!")
   assert_equal(1, #player1.inbox)
   local m2 = player1:send_to_inbox("Hallo", "World!")
   assert_equal(2, #player1.inbox)

   assert_equal(m1, player1.inbox[1])
   assert_equal(m2, player1.inbox[2])
end
