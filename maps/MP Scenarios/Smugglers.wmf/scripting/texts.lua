-- =======================================================================
--                          Texts for this scenario
-- =======================================================================
welcome_msg = rt(
   h1(_"Smugglers") ..
   h2(_"Rules") ..
   p(_(
[[Smugglers is a fun map for 4 players. You and your partner start diagonally from each other on a point symmetric island. ]] ..
[[There are plenty of smuggling tunnels on this island, each consisting of a receiving and a sending end. ]])
.. _(
[[To establish a smuggling route, you need to build a warehouse on a sending/receiving spot ]] ..
[[while your team mate has to build one on the corresponding receiving/sending spot. ]] ..
[[A ware is then transported every 10 seconds. ]]) ..
-- TRANSLATORS: %s = '<number> points'
_([[For harder to defend smuggling routes, you get 2 or 3 points per ware smuggled. The first team to collect %s wins.]]
)) ..
   rt("image=map:send_spot.png", p(_"A sending spot")) ..
   rt("image=map:recv_spot.png", p(_"A receiving spot")) ..
rt(
   p(_(
[[Remember that the map has rotational symmetry. ]] ..
[[For example, when you have found a spot to the top-left of your headquarters, ]] ..
[[the corresponding spot will be to the bottom-right of the headquarters of your team mate.]])
))  .. p(_(
[[You can see the number of wares traded at any time in the general statistics menu. Good luck!]]
)))

-- TRANSLATORS: the first 2 parameters are player names, the last parameter is '<number> points'
smuggling_route_established_other_team = rt(p(_(
[[A new smuggling route from %1$s to %2$s has been established! ]] ..
[[Every ware they smuggle there is worth %3$s.]]
)))
-- TRANSLATORS: %s = '<number> points'
smuggling_route_established_sender = rt(p(_(
[[Your team has established a new smuggling route. You have the sending warehouse. ]] ..
[[Every ware smuggled here is worth %s.]]
)))
-- TRANSLATORS: %s = '<number> points'
smuggling_route_established_receiver = rt(p(_(
[[Your team has established a new smuggling route. You have the receiving warehouse. ]] ..
[[Every ware smuggled here is worth %s.]]
)))

-- TRANSLATORS: the first parameter is '<number> points', the last 2 parameters are player names
smuggling_route_broken = rt(p(_
[[The smuggling route worth %1$s from %2$s to %3$s has been broken!]]
))

game_over = rt(
   h1("Game over!") ..
-- TRANSLATORS: the first 4 parameters are player names, the last parameter is '<number> points'
   p(_
[[Game over! %1$s and %2$s have won the game! %3$s and %4$s only managed to collect %5$s.]]
)
)



