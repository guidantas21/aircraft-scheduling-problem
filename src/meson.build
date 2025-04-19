sources = files(
  'main.cpp',
  'instance.cpp',
  'solution.cpp',
  'construction.cpp',
  'runway.cpp',
  'flight.cpp',
  'neighborhood.cpp',
  'ASP.cpp',
  'VND.cpp',
  'GRASP.cpp',
  'GILS.cpp',
  'perturbation.cpp'
)

incdir = include_directories('../include')

executable(
  meson.project_name(), 
  sources,
  include_directories: [incdir, incdir_deps],
  dependencies: dependencies,
  install: true
)

