booklet.pdf: booklet.tex confer/detailed.tex confer/overview.tex
	./latexrun booklet.tex

.PHONY: booklet.pdf

tools/convertconfer: tools/convertconfer.cpp tools/picojson.h
	g++ -o$@ -g $^

confer/detailed.tex: tools/convertconfer confer/*.json
	tools/convertconfer detailed $@

confer/overview.tex: tools/convertconfer confer/*.json
	tools/convertconfer overview $@
