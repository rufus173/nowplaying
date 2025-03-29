#include <QApplication>
#include <stdio.h>
#include <unistd.h>
#include <QDBusConnection>
#include "media_players.h"
int main(int argc, char **argv){
	QApplication app = QApplication(argc,argv);

	//check we are connected to the session bus
	if (!QDBusConnection::sessionBus().isConnected()){
		fprintf(stderr,"Could not connect to sesion bus.\n");
		exit(EXIT_FAILURE);
	}

	MediaPlayers media_players = MediaPlayers();
	qDebug() << media_players.get_current_track_position() << "/" << media_players.get_current_track_length();

	app.exec();
}
