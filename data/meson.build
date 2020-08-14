pixmap_data = [
    'stock_find_next_24.png',
    'stock_find_previous_24.png',
    'stock_rotate-90.png',
    'stock_rotate-270.png',
    'stock_zoom-page-width.png',
]

ui_data = [
    'ui/epdfview-ui.xml',
]
if with_printing
    ui_data += 'ui/epdfview-ui-print.xml'
endif

desktop_data = [
    'epdfview.desktop',
]

man_data = [
    'epdfview.1',
]

foreach s: ['24x24', '32x32', '48x48']
  icon = join_paths('icons', s, 'epdfview.png')
  inspath = join_paths(datadir, 'icons/hicolor', s, 'apps')
  install_data(icon, install_dir : inspath)
endforeach

install_data(join_paths('icons', 'scalable', 'epdfview.svg'), install_dir : join_paths(datadir, 'icons/hicolor', 'scalable', 'apps'))
install_data(pixmap_data, install_dir : join_paths(datadir, meson.project_name(), 'pixmaps'))
install_data(ui_data, install_dir : join_paths(datadir, meson.project_name(), 'ui'))
install_data(desktop_data, install_dir : join_paths(datadir, 'applications'))
install_man(man_data)
