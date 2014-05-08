noinst_SCRIPTS=		runtest
EXTRA_DIST=		NiHTest.pm runtest.in
TEST_LOG_COMPILER=	${builddir}/runtest
CLEANFILES=		runtest

check-am: runtest

runtest: runtest.in
	sed -e 's!@[s]rcdir@!${srcdir}!g' -e 's!@[a]bs_srcdir@!${abs_srcdir}!g' ${srcdir}/runtest.in > runtest
	chmod +x runtest

cleanup:
	rm -rf ${builddir}/sandbox-*.d[0-9]*/
