bool func(bool const a, bool const b) {
	return
		func
			(a,
			 func(b, a)) == func(b, func(b, a));
}

bool fund(bool const a, bool const b) {
	return
		func
			(func
			 	(a,
			 	 func(b, a)), func(b, func(b, a)));
}
