
SET(POSHLIB_SRCS
	posh.c
	posh.h)

ADD_LIBRARY(posh STATIC ${POSHLIB_SRCS})
set_target_properties(posh PROPERTIES FOLDER "${ROOT_FOLDER}/deps")
