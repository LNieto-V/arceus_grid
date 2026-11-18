TEX = pdflatex
FLAGS = -interaction=nonstopmode -halt-on-error
TARGET = presentacion_arceus_grid

.PHONY: all clean

all: $(TARGET).pdf clean_aux

$(TARGET).pdf: $(TARGET).tex
	$(TEX) $(FLAGS) $(TARGET).tex
	$(TEX) $(FLAGS) $(TARGET).tex

clean_aux:
	rm -f *.aux *.log *.nav *.out *.snm *.toc *.vrb *.fls *.fdb_latexmk *.synctex.gz

clean: clean_aux
	rm -f $(TARGET).pdf

