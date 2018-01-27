    TODO (So any pull request is apreciated): 
	* Add continuous scrolling & maybe an option for toggle hide scrollbars. 
	* Add shift+scroll for horizontal scrolling
	* Change icons for some more originals to not use GNOME icons anymore
	* Finish the port to GTK+ 3.0 to keep a lightweight alternative to evince alive
    (Any pull request to add functionality is also apreciated).


 ePDFView
 ========


![Travis status](https://travis-ci.org/jristz/epdfview.svg)

 ePDFView is a simple and lightweight PDF viewer.
 For more general information about the original ePDFView please visit the project's website at 
 http://www.emma-soft.com/projects/epdfview/ .
 For info about this fork check the source code or ask a question on the bugtracker.

 This software is licensed under the GNU General Public License (GPL).
 The icons used by this software are part of the Gnome Icon Theme, 
 which is copyright The GNOME Project and released under the GNU General Public License (GPL).

    Enhancements by Pedro A. Aranda Gutiérrez
    =========================================

	epdfsync
	========

	epdfsync is a companion script that will be called by epdfview upon a
	Ctrl-Button1Down. This script should call synctex, if you want to have a full 
	edit cycle for LaTEX.
	Tested on:
	    * Lubuntu 15.10 (LXDE)
	    * Ubuntu 14.04.3 LTS (Unity)
	    * FreeBSD 10.2 (Xfce)
            * ArchLinux on 2017/04/23 (Xfce)
	SIGHUP
	======

	Reload on SIGHUP to integrate with 'latexmk -pvc'.


 Requirements
 ============
 
 GTK+ version 3.22.0 or higher ( http://www.gtk.org/ )
 Poppler version 0.17.0 with glib bindings ( http://poppler.freedesktop.org/ )
 CppUnit to run the test suite ( http://cppunit.sourceforge.net/ )
 Doxygen to build the documentation ( http://www.stack.nl/~dimitri/doxygen/ )
