-- =======================================================================
--                          Texts for this scenario
-- =======================================================================
welcome_msg = rt(
   h1(_"Smugglers") ..
   h2(_"Rules") ..
   p(_
[[Smugglers is a Fun map for 4 players. You and your partner start diagonally
to each other on a X-shaped island. Each player has a point in the
center of the map where he should place a warehouse. If both players in a
team succeed to build their warehouse in the right spot a smuggling route
is established: all wares from the top player that are delivered to his
warehouse are warped to the bottom player's warehouse. The aim of the game is to
smuggle %i items from top to bottom.]]) .. p(_
[[The other team has the same goal and will try to interrupt your smuggling
routes by burning down your warehouses. You will pay them back in their own
coin of course!]]) .. p(_
[[You can see the number of wares traded at any time in the general
statistics menu. Good luck!]]
))

smuggling_route_established = rt(p(_
[[%s and %s have established a smuggling route!]]
))
smuggling_route_broken = rt(p(_
[[The smuggling route of %s and %s has been broken!]]
))

game_over = rt(
   h1("Game over!") ..
   p(_
[[Game over! %s and %s have won the game! %s and %s only managed to
smuggle %i wares.]]
)
)



