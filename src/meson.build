gtk_src = [
    'gtk/FindView.cxx',
    'gtk/MainView.cxx',
    'gtk/PageView.cxx',
    'gtk/PreferencesView.cxx',
    'gtk/StockIcons.cxx',
]

print_src = []
if with_printing
    print_src += 'JobPrint.cxx'
    print_src += 'PrintPter.cxx'
    print_src += 'gtk/PrintView.cxx'
endif

lib_src = [
    print_src,
    gtk_src,
    'main.cxx',
    'Config.cxx',
    'DocumentLinkGoto.cxx',
    'DocumentLinkUri.cxx',
    'DocumentOutline.cxx',
    'DocumentPage.cxx',
    'DocumentRectangle.cxx',
    'FindPter.cxx',
    'IDocument.cxx',
    'IDocumentLink.cxx',
    'IJob.cxx',
    'JobFind.cxx',
    'JobLoad.cxx',
    'JobRender.cxx',
    'JobSave.cxx',
    'MainPter.cxx',
    'PagePter.cxx',
    'PDFDocument.cxx',
    'PreferencesPter.cxx',
]

exe_src = [
    'main.cxx'
]

epdfview_lib = static_library('libepdfview.a', lib_src,
    dependencies: deps,
    include_directories: top_inc
)

executable('epdfview', exe_src,
    dependencies : deps,
    link_with : epdfview_lib,
    include_directories : top_inc,
    install: true
)
