print_test_src = []
if with_printing
    print_test_src += 'DumbPrintView.cxx'
    print_test_src += 'PrintPterTest.cxx'
endif

test_src = [
    print_test_src,
    'ConfigTest.cxx',
    'DocumentOutlineTest.cxx',
    'DumbDocument.cxx',
    'DumbDocumentObserver.cxx',
    'DumbFindView.cxx',
    'DumbMainView.cxx',
    'DumbPageView.cxx',
    'DumbPreferencesView.cxx',
    'FindPterTest.cxx',
    'main.cxx',
    'MainPterTest.cxx',
    'PagePterTest.cxx',
    'PDFDocumentTest.cxx',
    'PreferencesPterTest.cxx',
    'Utils.cxx',
]

test_deps = deps
test_deps += dependency('cppunit')

test_exe = executable('epdfview_test', test_src,
    dependencies: test_deps,
    link_with : epdfview_lib,
    include_directories: [src_inc, top_inc],
)

test('epdfview_test', test_exe)
