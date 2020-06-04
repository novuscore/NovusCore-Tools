
SET(RGETC_SRCS
	rg_etc1.cpp
	rg_etc1.h)

ADD_LIBRARY(rg_etc1 STATIC ${RGETC_SRCS})
set_target_properties(rg_etc1 PROPERTIES FOLDER "${ROOT_FOLDER}/deps")
