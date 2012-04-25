-- =======================================================================
--                          Texts for this scenario
-- =======================================================================
welcome_msg = rt(
   h1(_"Smugglers") ..
   h2(_"Rules") ..
   p(_
[[Smugglers is a Fun map for 4 players. You and your partner start diagonally
to each other on a point symmetric island. There are plenty of smuggling tunnels
on this island each consisting of a receiving and a sending end. To establish a smuggling
route you need to build a warehouse on a sending/receiving spot while your team
mate has to build one on the corresponding receiving/sending spot. A ware is then transported
each 10 seconds. For harder to defend smuggling routes you get points for smuggling:
2 or 3 per ware. The first team to have %i points wins.]])
) ..
   rt("image=map:send_spot.png", p(_"A sending spot")) ..
   rt("image=map:recv_spot.png", p(_"A receiving spot")) ..
rt(
   p(_
[[Remember that the map has rotational symmetry: When you have found a spot to the top-left
of your headquarters, the corresponding spot will be to the bottom-right of the headquarters
of your team mate.]]
)  .. p(_
[[You can see the number of wares traded at any time in the general
statistics menu. Good luck!]]
))

smuggling_route_established = rt(p(_
[[A new smuggling route from %s to %s has been established!. Every ware smuggled here
is worth %i points.]]
))
smuggling_route_broken = rt(p(_
[[The smuggling route worth %i from %s to %s has been broken!]]
))

game_over = rt(
   h1("Game over!") ..
   p(_
[[Game over! %s and %s have won the game! %s and %s only managed to
acquire %i points.]]
)
)



