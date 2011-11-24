srcdir = /home/xma/work/svn/optimization/extension/odus
builddir = /home/xma/work/svn/optimization/extension/odus
top_srcdir = /home/xma/work/svn/optimization/extension/odus
top_builddir = /home/xma/work/svn/optimization/extension/odus
EGREP = grep -E
SED = /bin/sed
CONFIGURE_COMMAND = './configure' '--with-php-config=/home/xma/php/bin/php-config'
CONFIGURE_OPTIONS = '--with-php-config=/home/xma/php/bin/php-config'
SHLIB_SUFFIX_NAME = so
SHLIB_DL_SUFFIX_NAME = so
ZEND_EXT_TYPE = zend_extension
RE2C = exit 0;
AWK = gawk
shared_objects_odus = odus.lo odwrapper.lo od_hash.lo hash_si.lo hash_function.lo od_igbinary.lo od_debug.lo
PHP_PECL_EXTENSION = odus
PHP_MODULES = $(phplibdir)/odus.la
PHP_ZEND_EX =
all_targets = $(PHP_MODULES) $(PHP_ZEND_EX)
install_targets = install-modules install-headers
prefix = /home/xma/php
exec_prefix = $(prefix)
libdir = ${exec_prefix}/lib
prefix = /home/xma/php
phplibdir = /home/xma/work/svn/optimization/extension/odus/modules
phpincludedir = /home/xma/php/include/php
CC = cc
#CFLAGS = -g -O2
CFLAGS = -g
CFLAGS_CLEAN = $(CFLAGS)
CPP = cc -E
CPPFLAGS = -DHAVE_CONFIG_H
CXX =
CXXFLAGS =
CXXFLAGS_CLEAN = $(CXXFLAGS)
EXTENSION_DIR = /home/xma/php/modules
PHP_EXECUTABLE = /home/xma/php/bin/php
EXTRA_LDFLAGS =
EXTRA_LIBS =
INCLUDES = -I/home/xma/php/include/php -I/home/xma/php/include/php/main -I/home/xma/php/include/php/TSRM -I/home/xma/php/include/php/Zend -I/home/xma/php/include/php/ext -I/home/xma/php/include/php/ext/date/lib
LFLAGS =
LDFLAGS =
SHARED_LIBTOOL =
LIBTOOL = $(SHELL) $(top_builddir)/libtool
SHELL = /bin/sh
INSTALL_HEADERS =
mkinstalldirs = $(top_srcdir)/build/shtool mkdir -p
INSTALL = $(top_srcdir)/build/shtool install -c
INSTALL_DATA = $(INSTALL) -m 644

DEFS = -DPHP_ATOM_INC -I$(top_builddir)/include -I$(top_builddir)/main -I$(top_srcdir)
COMMON_FLAGS = $(DEFS) $(INCLUDES) $(EXTRA_INCLUDES) $(CPPFLAGS) $(PHP_FRAMEWORKPATH)

all: $(all_targets) 
	@echo
	@echo "Build complete."
	@echo "Don't forget to run 'make test'."
	@echo
	
build-modules: $(PHP_MODULES) $(PHP_ZEND_EX)

libphp$(PHP_MAJOR_VERSION).la: $(PHP_GLOBAL_OBJS) $(PHP_SAPI_OBJS)
	$(LIBTOOL) --mode=link $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -rpath $(phptempdir) $(EXTRA_LDFLAGS) $(LDFLAGS) $(PHP_RPATHS) $(PHP_GLOBAL_OBJS) $(PHP_SAPI_OBJS) $(EXTRA_LIBS) $(ZEND_EXTRA_LIBS) -o $@
	-@$(LIBTOOL) --silent --mode=install cp $@ $(phptempdir)/$@ >/dev/null 2>&1

libs/libphp$(PHP_MAJOR_VERSION).bundle: $(PHP_GLOBAL_OBJS) $(PHP_SAPI_OBJS)
	$(CC) $(MH_BUNDLE_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) $(PHP_GLOBAL_OBJS:.lo=.o) $(PHP_SAPI_OBJS:.lo=.o) $(PHP_FRAMEWORKS) $(EXTRA_LIBS) $(ZEND_EXTRA_LIBS) -o $@ && cp $@ libs/libphp$(PHP_MAJOR_VERSION).so

install: $(all_targets) $(install_targets)

install-sapi: $(OVERALL_TARGET)
	@echo "Installing PHP SAPI module:       $(PHP_SAPI)"
	-@$(mkinstalldirs) $(INSTALL_ROOT)$(bindir)
	-@if test ! -r $(phptempdir)/libphp$(PHP_MAJOR_VERSION).$(SHLIB_DL_SUFFIX_NAME); then \
		for i in 0.0.0 0.0 0; do \
			if test -r $(phptempdir)/libphp$(PHP_MAJOR_VERSION).$(SHLIB_DL_SUFFIX_NAME).$$i; then \
				$(LN_S) $(phptempdir)/libphp$(PHP_MAJOR_VERSION).$(SHLIB_DL_SUFFIX_NAME).$$i $(phptempdir)/libphp$(PHP_MAJOR_VERSION).$(SHLIB_DL_SUFFIX_NAME); \
				break; \
			fi; \
		done; \
	fi
	@$(INSTALL_IT)

install-modules: build-modules
	@test -d modules && \
	$(mkinstalldirs) $(INSTALL_ROOT)$(EXTENSION_DIR)
	@echo "Installing shared extensions:     $(INSTALL_ROOT)$(EXTENSION_DIR)/"
	@rm -f modules/*.la >/dev/null 2>&1
	@$(INSTALL) modules/* $(INSTALL_ROOT)$(EXTENSION_DIR)

install-headers:
	-@if test "$(INSTALL_HEADERS)"; then \
		for i in `echo $(INSTALL_HEADERS)`; do \
			i=`$(top_srcdir)/build/shtool path -d $$i`; \
			paths="$$paths $(INSTALL_ROOT)$(phpincludedir)/$$i"; \
		done; \
		$(mkinstalldirs) $$paths && \
		echo "Installing header files:          $(INSTALL_ROOT)$(phpincludedir)/" && \
		for i in `echo $(INSTALL_HEADERS)`; do \
			if test "$(PHP_PECL_EXTENSION)"; then \
				src=`echo $$i | $(SED) -e "s#ext/$(PHP_PECL_EXTENSION)/##g"`; \
			else \
				src=$$i; \
			fi; \
			if test -f "$(top_srcdir)/$$src"; then \
				$(INSTALL_DATA) $(top_srcdir)/$$src $(INSTALL_ROOT)$(phpincludedir)/$$i; \
			elif test -f "$(top_builddir)/$$src"; then \
				$(INSTALL_DATA) $(top_builddir)/$$src $(INSTALL_ROOT)$(phpincludedir)/$$i; \
			else \
				(cd $(top_srcdir)/$$src && $(INSTALL_DATA) *.h $(INSTALL_ROOT)$(phpincludedir)/$$i; \
				cd $(top_builddir)/$$src && $(INSTALL_DATA) *.h $(INSTALL_ROOT)$(phpincludedir)/$$i) 2>/dev/null || true; \
			fi \
		done; \
	fi

PHP_TEST_SETTINGS = -d 'open_basedir=' -d 'output_buffering=0' -d 'memory_limit=-1'
PHP_TEST_SHARED_EXTENSIONS =  ` \
	if test "x$(PHP_MODULES)" != "x"; then \
		for i in $(PHP_MODULES)""; do \
			. $$i; $(top_srcdir)/build/shtool echo -n -- " -d extension=$$dlname"; \
		done; \
	fi; \
	if test "x$(PHP_ZEND_EX)" != "x"; then \
		for i in $(PHP_ZEND_EX)""; do \
			. $$i; $(top_srcdir)/build/shtool echo -n -- " -d $(ZEND_EXT_TYPE)=$(top_builddir)/modules/$$dlname"; \
		done; \
	fi`

test: all
	-@if test ! -z "$(PHP_EXECUTABLE)" && test -x "$(PHP_EXECUTABLE)" && cp -r $(EXTENSION_DIR)/* $(top_builddir)/modules/; then \
		TEST_PHP_EXECUTABLE=$(PHP_EXECUTABLE) \
		TEST_PHP_SRCDIR=$(top_srcdir) \
		CC="$(CC)" \
			$(PHP_EXECUTABLE) $(PHP_TEST_SETTINGS) $(top_srcdir)/run-tests.php -d extension_dir=modules/ $(PHP_TEST_SHARED_EXTENSIONS) tests/; \
	elif test ! -z "$(SAPI_CLI_PATH)" && test -x "$(SAPI_CLI_PATH)"; then \
		INI_FILE=`$(top_builddir)/$(SAPI_CLI_PATH) -d 'display_errors=stderr' -r 'echo php_ini_loaded_file();' 2> /dev/null`; \
		if test "$$INI_FILE"; then \
			$(EGREP) -v '^(zend_)?extension(_debug)?(_ts)?[\t\ ]*=' "$$INI_FILE" > $(top_builddir)/tmp-php.ini; \
		else \
			echo > $(top_builddir)/tmp-php.ini; \
		fi; \
		INI_SCANNED_PATH=`$(top_builddir)/$(SAPI_CLI_PATH) -d 'display_errors=stderr' -r '$$a = explode(",\n", trim(php_ini_scanned_files())); echo $$a[0];' 2> /dev/null`; \
		if test "$$INI_SCANNED_PATH"; then \
			INI_SCANNED_PATH=`$(top_srcdir)/build/shtool path -d $$INI_SCANNED_PATH`; \
			$(EGREP) -h -v '^(zend_)?extension(_debug)?(_ts)?[\t\ ]*=' "$$INI_SCANNED_PATH"/*.ini >> $(top_builddir)/tmp-php.ini; \
		fi; \
		TEST_PHP_EXECUTABLE=$(top_builddir)/$(SAPI_CLI_PATH) \
		TEST_PHP_SRCDIR=$(top_srcdir) \
		CC="$(CC)" \
			$(top_builddir)/$(SAPI_CLI_PATH) -n -c $(top_builddir)/tmp-php.ini $(PHP_TEST_SETTINGS) $(top_srcdir)/run-tests.php -n -c $(top_builddir)/tmp-php.ini -d extension_dir=$(top_builddir)/modules/ $(PHP_TEST_SHARED_EXTENSIONS) $(TESTS); \
	else \
		echo "ERROR: Cannot run tests without CLI sapi."; \
	fi

clean:
	find . -name \*.gcno -o -name \*.gcda | xargs rm -f
	find . -name \*.lo -o -name \*.o | xargs rm -f
	find . -name \*.la -o -name \*.a | xargs rm -f 
	find . -name \*.so | xargs rm -f
	find . -name .libs -a -type d|xargs rm -rf
	rm -f libphp$(PHP_MAJOR_VERSION).la $(SAPI_CLI_PATH) $(OVERALL_TARGET) modules/* libs/*

distclean: clean
	rm -f config.cache config.log config.status Makefile.objects Makefile.fragments libtool main/php_config.h stamp-h php5.spec sapi/apache/libphp$(PHP_MAJOR_VERSION).module buildmk.stamp
	$(EGREP) define'.*include/php' $(top_srcdir)/configure | $(SED) 's/.*>//'|xargs rm -f
	find . -name Makefile | xargs rm -f

.PHONY: all clean install distclean test
.NOEXPORT:
odus.lo: /home/xma/work/svn/optimization/extension/odus/odus.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/xma/work/svn/optimization/extension/odus $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/xma/work/svn/optimization/extension/odus/odus.c -o odus.lo 
odwrapper.lo: /home/xma/work/svn/optimization/extension/odus/odwrapper.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/xma/work/svn/optimization/extension/odus $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/xma/work/svn/optimization/extension/odus/odwrapper.c -o odwrapper.lo 
od_hash.lo: /home/xma/work/svn/optimization/extension/odus/od_hash.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/xma/work/svn/optimization/extension/odus $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/xma/work/svn/optimization/extension/odus/od_hash.c -o od_hash.lo 
hash_si.lo: /home/xma/work/svn/optimization/extension/odus/hash_si.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/xma/work/svn/optimization/extension/odus $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/xma/work/svn/optimization/extension/odus/hash_si.c -o hash_si.lo 
hash_function.lo: /home/xma/work/svn/optimization/extension/odus/hash_function.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/xma/work/svn/optimization/extension/odus $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/xma/work/svn/optimization/extension/odus/hash_function.c -o hash_function.lo 
od_igbinary.lo: /home/xma/work/svn/optimization/extension/odus/od_igbinary.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/xma/work/svn/optimization/extension/odus $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/xma/work/svn/optimization/extension/odus/od_igbinary.c -o od_igbinary.lo 
od_debug.lo: /home/xma/work/svn/optimization/extension/odus/od_debug.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/xma/work/svn/optimization/extension/odus $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/xma/work/svn/optimization/extension/odus/od_debug.c -o od_debug.lo 
$(phplibdir)/odus.la: ./odus.la
	$(LIBTOOL) --mode=install cp ./odus.la $(phplibdir)

./odus.la: $(shared_objects_odus) $(ODUS_SHARED_DEPENDENCIES)
	$(LIBTOOL) --mode=link $(CC) $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS) $(LDFLAGS) -o $@ -export-dynamic -avoid-version -prefer-pic -module -rpath $(phplibdir) $(EXTRA_LDFLAGS) $(shared_objects_odus) $(ODUS_SHARED_LIBADD)

