run(function()
   sleep(5000)

   map:get_field(10, 10).immovable:destroy()
   sleep(120000)
   local imm = map:get_field(10, 10).immovable
   if imm ~= nil then
      assert_not_equal("destroyed_building", imm.descr.name)
   end

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
