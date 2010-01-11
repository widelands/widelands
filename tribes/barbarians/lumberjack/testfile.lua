0=lua testfile.lua 
# [chop]
# 0=findobject attrib:tree radius:10
# 1=walk object
# 2=playFX ../../../sound/woodcutting/woodcutting 255
# 3=animation hacking 10000
# 4=playFX ../../../sound/spoken/timber 192
# 5=object fall
# 6=animation idle 2000
# 7=createitem trunk
# 8=return

function do_work(worker)
   tree = wl.map.find_object('tree', 10)
   worker.walk_to(tree) -- This should yield and only return when the worker has arrived
   wl.sound("../../../sound/woodcutting/woodcutting", 255) -- This should yield till he's done
   worker.set_animation("hacking", 10000) -- This should 


   coroutine.yield()
   print "Working more!"
   coroutine.yield()
   print "Done with working... hui"
end

-- This crashes the second time around
-- return coroutine.create(do_work)

-- This works forever
rv =  coroutine.create(do_work)
print("LUA: rv=", rv);
return rv



