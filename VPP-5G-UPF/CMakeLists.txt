cmake_minimum_required(VERSION 3.10)
project(gtpu_upf_plugin)

find_package(VPP REQUIRED)

add_vpp_plugin(gtpu_upf
  SOURCES
  gtpu_upf_node.c

  INSTALL_HEADERS
  gtpu_upf.h
)
