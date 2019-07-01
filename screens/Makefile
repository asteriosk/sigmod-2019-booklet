pdfs:	sources/*.tex
	mkdir -p pdfs
	find sources/*.tex -exec pdflatex -output-directory pdfs {} \;
	rm pdfs/*.aux 
	rm pdfs/*.log
	mkdir -p pdfs/Mon pdfs/Tue pdfs/Wed pdfs/Thu pdfs/Fri
	mv pdfs/*Monday* pdfs/Mon/
	mv pdfs/*Tuesday* pdfs/Tue/
	mv pdfs/*Wednesday* pdfs/Wed/
	mv pdfs/*Thursday* pdfs/Thu/
	mv pdfs/*Friday* pdfs/Fri/


convertconfer: convertconfer.cpp picojson.h
	clang++ -std=c++11 -stdlib=libc++ --output convertconfer -Weverything convertconfer.cpp
	mkdir -p sources pdfs
	./convertconfer detailed $@

.PHONY: convertconfer pdfs


clean:
	rm -rf build pdfs
	find . -type f \( -name '*.aux' -or -name  '*.auxlock' -or -name '*.figlist' -or -name '*eps-converted-to.pdf' -or -name '*.bbl' -or -name '*.fdb_latexmk' -or -name '*.synctex.gz' -or -name '*.blg' -or -name '*.log' -or -name '*.out' -or -name '*.toc' -or -name '*.lot' -or -name '*.lof' -or -name '*.loa' -or -name '*.xdv' -or -name '*.fls' \) -exec rm -f '{}' \;
