include "../data/scripting/messages.lua"
include "../data/scripting/richtext.lua"

run(function()
   sleep(5000)

   -- Test for <not_a_tag> in table and multilinetextarea.
   send_to_inbox(p1, "Title <not_a_tag>", rt(p("Some text <not_a_tag> more&nbsp;text")), {popup = true})
   sleep(500)
   send_to_inbox(p1, "Title <not_a_tag>", "Some text <not_a_tag> more&nbsp;text", {popup = true})
   sleep(500)

   -- Test for <not_a_tag> in listselect and multilinetextarea.
   p1:add_objective("obj1", "Title 1 <not_a_tag>", rt(p("Some text <not_a_tag> more&nbsp;text")))
   wl.ui.MapView().buttons.objectives:click()
   sleep(500)
   p1:add_objective("obj2", "Title 2 <not_a_tag>", "Some text <not_a_tag> more&nbsp;text")

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
