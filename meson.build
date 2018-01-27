project('epdfview', 'cpp',
    version: '0.2.0',
    meson_version : '>= 0.42.0',
    license : 'GPL2+'
)

datadir = join_paths(get_option('prefix'), get_option('datadir'))
localedir = join_paths(get_option('prefix'), get_option('localedir'))

top_inc = include_directories('.')
src_inc = include_directories('src')

# dependencies
glib_dep = dependency('glib-2.0', version : '>= 2.32.0')
gtk_dep = dependency('gtk+-2.0', version : '>= 2.6.0')
poppler_dep = dependency('poppler-glib', version : '>= 0.17.0')

deps = [glib_dep, gtk_dep, poppler_dep]

with_printing = get_option('enable-printing')
if (with_printing)
    deps += dependency('cups')
endif

with_nls = get_option('enable-nls')

cdata = configuration_data()
cdata.set_quoted('VERSION', meson.project_version())
cdata.set_quoted('DATADIR', datadir)
cdata.set_quoted('LOCALEDIR', localedir)
cdata.set_quoted('PACKAGE', meson.project_name())
cdata.set('HAVE_CUPS', with_printing)
cdata.set('ENABLE_NLS', with_nls)

configure_file(configuration : cdata, output : 'config.h')

subdir('src')
subdir('data')

if with_nls
    subdir('po')
endif
if get_option('build-docs')
    subdir('doc')
endif
if get_option('build-tests')
    subdir('tests')
endif
