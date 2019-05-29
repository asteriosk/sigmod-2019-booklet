booklet.pdf: booklet.tex confer/detailed.tex confer/overview.tex
	./latexrun booklet.tex

.PHONY: downloadschedule booklet.pdf

tools/convertconfer: tools/convertconfer.cpp tools/picojson.h
#	g++ -o$@ -g $^
	clang++ -std=c++11 -stdlib=libc++ --output tools/convertconfer -Weverything tools/convertconfer.cpp

confer/detailed.tex: tools/convertconfer confer/*.json
	tools/convertconfer detailed $@

confer/overview.tex: tools/convertconfer confer/*.json downloadschedule
	tools/convertconfer overview $@

downloadschedule:
	@-mkdir schedule
	wget https://homepages.cwi.nl/~manegold/SIGMOD-2019-Schedule-ataglance/SIGMOD-2019-Schedule-ataglance-128mm-x-85mm-p1.pdf -O schedule/p1.pdf
	wget https://homepages.cwi.nl/~manegold/SIGMOD-2019-Schedule-ataglance/SIGMOD-2019-Schedule-ataglance-128mm-x-85mm-p2.pdf -O schedule/p2.pdf
	wget https://homepages.cwi.nl/~manegold/SIGMOD-2019-Schedule-ataglance/SIGMOD-2019-Schedule-ataglance-128mm-x-85mm-p3.pdf -O schedule/p3.pdf
	wget https://homepages.cwi.nl/~manegold/SIGMOD-2019-Schedule-ataglance/SIGMOD-2019-Schedule-ataglance-128mm-x-85mm-p4.pdf -O schedule/p4.pdf
	wget https://homepages.cwi.nl/~manegold/SIGMOD-2019-Schedule-ataglance/SIGMOD-2019-Schedule-ataglance-128mm-x-85mm-p5.pdf -O schedule/p5.pdf
	wget https://homepages.cwi.nl/~manegold/SIGMOD-2019-Schedule-ataglance/SIGMOD-2019-Schedule-ataglance-128mm-x-85mm-p6.pdf -O schedule/p6.pdf
