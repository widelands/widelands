FILES = point-above-line.png triangle-measures.png triangle-projection-measures.png viewangle.png

all: $(FILES)

%.png: %.gnuplot
	gnuplot $< > $@.
	pngrewrite $@. $@
	optipng -q -zc1-9 -zm1-9 -zs0-3 -f0-5 $@
	rm $@.

clean:
	@rm -f echo $(FILES)
