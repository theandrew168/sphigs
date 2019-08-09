all : sphigs.dvi sphigs.ps sphigs.txt sphigs.quikref

.SUFFIXES: .dvi .tex .ps

sphigs.dvi: sphigs.tex
	cat tex_macros sphigs.tex > /tmp/sphigs.tex
	tex /tmp/sphigs
	/bin/rm /tmp/sphigs.tex

sphigs.txt: sphigs.tex 
	cat sphigs.tex > /tmp/sphigs.tex
	/cs/src/srgp/doc/detex < /tmp/sphigs.tex | \
	      sed  -e 's/\`\`/\"/g'  -e s/\\\'\\\'/\\\"/g  > sphigs.txt
	/bin/rm /tmp/sphigs.tex

sphigs.quikref: sphigs.3
	nroff -man sphigs.3 > sphigs.quikref

.dvi.ps :
	dvieps $* -o $*.ps
