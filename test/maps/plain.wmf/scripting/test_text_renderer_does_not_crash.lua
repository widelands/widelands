include "../data/scripting/messages.lua"
include "../data/scripting/formatting.lua"

run(function()
   sleep(5000)

	-- Test for <not_a_tag> in table and multilinetextarea.
   send_message(p1, "Title <not_a_tag>", rt(p("Some text <not_a_tag> more&nbsp;text")), {popup = true})

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
