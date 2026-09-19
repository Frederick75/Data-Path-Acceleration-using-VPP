cmake_minimum_required(VERSION 3.10)
project(oran_ecpri_plugin)

find_package(VPP REQUIRED)

add_vpp_plugin(oran_ecpri
  SOURCES
  ecpri_oran_node.c

  INSTALL_HEADERS
  ecpri_oran.h
)
