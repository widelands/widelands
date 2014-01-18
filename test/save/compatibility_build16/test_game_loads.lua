use("aux", "coroutine")

run(
function ()
   use("aux", "lunit")

   wl.Game().desired_speed = 10000
   sleep(100 * 1000)

   lunit:run()
   wl.ui.MapView():close()
end
)
