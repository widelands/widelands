#  A newly built whitespace_checker may find new errors, so remove the cache.
whitespace_checker: whitespace_checker.adb
	gnatmake -gnatafn -gnatwa -gnaty whitespace_checker
	rm -fr ../../build/stylecheck

clean:
	@rm -f whitespace_checker *.ali *.o
