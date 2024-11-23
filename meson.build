project('cottage', 'c', 'cpp', default_options: ['default_library=static'])

cc = meson.get_compiler('c')
cxx = meson.get_compiler('cpp')

ssl_dep = dependency('openssl')

json = subproject('nlohmann_json')
json_dep = json.get_variable('nlohmann_json_dep')

include = include_directories('include')

fmt = subproject('fmt')
fmt_dep = fmt.get_variable('fmt_dep')

src = ['main.cc', 'httplib.cc', 'webfinger.cc']
executable('cottage', src, dependencies: [json_dep, ssl_dep, fmt_dep], include_directories: include)
