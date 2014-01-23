BASICS = prepare #git wget unzip tar python perl
OBJS = doxygen 
PWD = $(shell pwd)

## decompress tools, may vary from platforms
## $1=>targetDIR $2=>suffix $3=>decompressedName
define decompress
	{ \
		case $(2) in \
			"tar.gz") tar zxf $(1).$(2) ;; \
			"tgz") tar zxf $(1).$(2) ;; \
			"zip") unzip $(1).$(2) ;; \
		esac \
		&& mv $(3) $(1) \
	; } 
endef

## $1=>targetDIR $2=>webURL $3=>suffix $4=>decompressedName
define ensure_exists
	[ -d $(1) ] || { \
		{ wget -O $(1).$(3) $(2) || cp ../third/$(1).$(3) ; } \
		&& $(call decompress,$(1),$(3),$(4) )  \
		&& rm $(1).$(3) ; \
	}
endef

test :
	@echo $(PWD)

help :
	@echo "specify a cmd from : all $(OBJS)"

all : $(OBJS)

prepare :
	mkdir -p build

doxygen : $(BASICS)
	$(call ensure_exists,"doxygen","https://github.com/doxygen/doxygen/archive/master.zip","zip","doxygen-master")
	cd doxygen && ./configure && make && make install

## help doxygen to parse python docstring
## extra config : 
## INPUT_FILTER           = "python /usr/local/bin/doxypy.py"
## FILTER_SOURCE_FILES    = YES
doxypy : $(BASICS)
	$(call ensure_exists,"doxypy-0.4.2","http://code.foosel.org/files/doxypy-0.4.2.tar.gz","tar.gz","doxypy-0.4.2")
	cd doxypy-0.4.2 && python setup.py install

rouge : $(BASICS) 
	echo install XML::DOM | perl -MCPAN
	$(call ensure_exists,"ROUGE-1.5.5","http://research.microsoft.com/~cyl/download/ROUGE-1.5.5.tgz","tgz","RELEASE-1.5.5")
	cd ROUGE-1.5.5/data/WordNet-2.0-Exceptions && ./buildExeptionDB.pl ./ db ../WordNet-3.0.exc.db >/dev/null && mv ../WordNet-3.0.exc.db ../WordNet-2.0.exc.db
	echo "export ROUGE_EVAL_HOME="$(PWD)"/ROUGE-1.5.5/data" >> ~/.bashrc
	echo "export PATH=$$PATH:$(PWD)/ROUGE-1.5.5/" >> ~/.bashrc

clean :
	
