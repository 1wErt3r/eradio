AC_INIT([eradio], [0.0.13], [])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CC
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile data/Makefile])

PKG_CHECK_MODULES([EFL], [
    ecore-evas
    eina
    evas
    elementary
    edje
])

PKG_CHECK_MODULES([LIBXML], [libxml-2.0])
EFL_LIBS_NO_CON=$(echo $EFL_LIBS | sed 's/-lecore_con//g')
AC_SUBST(EFL_LIBS_NO_CON)

AC_OUTPUT
