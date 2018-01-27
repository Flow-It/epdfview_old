doxy_exe = find_program('doxygen')

docdir = join_paths(meson.current_build_dir(), meson.project_name())

doc_data = configuration_data()
doc_data.set('VERSION', meson.project_version())
doc_data.set('top_srcdir', 'src')
doc_data.set('HTML_DIR', docdir)

doxyfile = configure_file(input : 'Doxyfile.in',
    output : 'Doxyfile',
    configuration : doc_data)

custom_target('docs',
    input : doxyfile,
    output : meson.project_name(),
    command : [ doxy_exe, doxyfile ],
    build_by_default : true)

if (get_option('install-docs'))
    insdir = join_paths(get_option('prefix'), get_option('datadir'), 'doc')
    install_subdir(docdir, install_dir : insdir)
endif
