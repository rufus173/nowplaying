#include <QApplication>
#include <QLabel>
#include <QGridLayout>
#include <QDBusConnection>
#include <QWindow>

#include <unistd.h>
#include <stdio.h>

#include "media_players.h"
#include "main.h"
int main(int argc, char **argv){
	QApplication app = QApplication(argc,argv);

	//check we are connected to the session bus
	if (!QDBusConnection::sessionBus().isConnected()){
		fprintf(stderr,"Could not connect to sesion bus.\n");
		exit(EXIT_FAILURE);
	}

	//====== create the main window ======
	MainWindow main_window = MainWindow();

	app.exec();
}
MainWindow::MainWindow(QWidget *parent){
	//mediaplayers object
	this->players = new MediaPlayers();

	//====== grid layout and widgets ======
	this->grid = new QGridLayout(this);
	this->song_progress_bar = new QProgressBar();
	this->song_info_label = new QLabel();
	this->grid->addWidget(this->song_progress_bar,0,0);
	this->grid->addWidget(this->song_info_label,0,0);

	//====== display everything ======
	this->show();
}
MainWindow::~MainWindow(){
	delete this->players;
}
