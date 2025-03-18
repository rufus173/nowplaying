.PHONY : nowplaying
nowplaying :
	cd src/qt_interface ; qmake ; make ; cp nowplaying ../..
