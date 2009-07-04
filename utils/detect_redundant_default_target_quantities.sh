#!/bin/bash
#  Detects ware types that define a default_target_quantity without even though
#  the ware is not demand checked. This will let the player set a target
#  quantity for the ware type even thoug the setting has no effect, which is
#  confusing for the user.

for tribe in barbarians empire atlanteans; do
	echo -n $tribe:
	for ware_type in $(grep "default_target_quantity=" tribes/atlanteans/*/conf|sed "s@tribes/.*/\(.*\)/conf:.*@\1@"); do
		if [[ -z $(egrep "economy needs $ware_type" tribes/atlanteans/*/conf) ]]; then
			echo -n " $ware_type"
		fi
	done
	echo
done
