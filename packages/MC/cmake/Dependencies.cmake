##---------------------------------------------------------------------------##
## MC/cmake/Dependencies.cmake
## Thomas M. Evans
## Saturday July 14 16:54:41 2012
##---------------------------------------------------------------------------##

##---------------------------------------------------------------------------##
## Define sub-packages
##---------------------------------------------------------------------------##

TRIBITS_PACKAGE_DEFINE_DEPENDENCIES(
  SUBPACKAGES_DIRS_CLASSIFICATIONS_OPTREQS
    mc_geometry   geometry     SS  REQUIRED
    mc_physics    physics      SS  REQUIRED
    mc_driver     mc_driver    SS  REQUIRED
  )

##---------------------------------------------------------------------------##
##                  end of MC/cmake/Dependencies.cmake
##---------------------------------------------------------------------------##
