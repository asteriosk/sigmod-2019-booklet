booklet.pdf: booklet.tex confer/detailed.tex confer/overview.tex
	./latexrun booklet.tex

.PHONY: booklet.pdf

tools/convertconfer: tools/convertconfer.cpp tools/picojson.h
	clang++ -std=c++11 -stdlib=libc++ --output tools/convertconfer -Weverything tools/convertconfer.cpp

confer/detailed.tex: tools/convertconfer confer/*.json
	tools/convertconfer detailed $@

confer/overview.tex: tools/convertconfer confer/*.json
	tools/convertconfer overview $@

download:
	cd confer ; ./updateconfer

clean:
	rm -rf build
	find . -type f \( -name '*.aux' -or -name  '*.auxlock' -or -name '*.figlist' -or -name '*eps-converted-to.pdf' -or -name '*.bbl' -or -name '*.fdb_latexmk' -or -name '*.synctex.gz' -or -name '*.blg' -or -name '*.log' -or -name '*.out' -or -name '*.toc' -or -name '*.lot' -or -name '*.lof' -or -name '*.loa' -or -name '*.xdv' -or -name '*.fls' \) -exec rm -f '{}' \;

