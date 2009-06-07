# - Find LAPACK libraries
# 2009 Lukas Jirkovsky
# based on FindPANO13 by TKSharpless
# reads cache variable
#  SOURCE_BASE_DIR -- directory that contains hugin source root
# defines cache vars
#  LAPACK_LIBRARIES, release link library list.
#  LAPACK_FOUND, If != "YES", do not try to use PANO13.

FIND_LIBRARY(LAPACK_LAPACK_LIBRARY
  NAMES lapack
  PATHS /usr/lib
        /usr/local/lib
        ${SOURCE_BASE_DIR}/
        ${SOURCE_BASE_DIR}/lapack
  )

FIND_LIBRARY(LAPACK_BLAS_LIBRARY
  NAMES blas
  PATHS /usr/lib
        /usr/local/lib
        ${SOURCE_BASE_DIR}/
        ${SOURCE_BASE_DIR}/lapack
  )

FIND_LIBRARY(LAPACK_G2C_LIBRARY
  NAMES g2c
  PATHS /usr/lib
        /usr/local/lib
        ${SOURCE_BASE_DIR}/
        ${SOURCE_BASE_DIR}/lapack
  )

IF(LAPACK_LAPACK_LIBRARY)
  IF(LAPACK_BLAS_LIBRARY)
    SET(LAPACK_LIBRARIES ${LAPACK_BLAS_LIBRARY})
  ENDIF(LAPACK_BLAS_LIBRARY)
  IF(LAPACK_G2C_LIBRARY)
    SET(LAPACK_LIBRARIES ${LAPACK_LIBRARIES} ${LAPACK_G2C_LIBRARY})
  ENDIF(LAPACK_G2C_LIBRARY)

  SET( LAPACK_FOUND "YES" )
  SET(LAPACK_LIBRARIES ${LAPACK_LAPACK_LIBRARY} ${LAPACK_LIBRARIES})
ENDIF(LAPACK_LAPACK_LIBRARY)
