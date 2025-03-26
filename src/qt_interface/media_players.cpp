#include "media_players.h"
#include <QDBusConnection>
#include <QStringList>
#include <QDebug>
#include <QDBusReply>
#include <QDBusConnectionInterface>
#include <QString>
#include <QRegularExpression>
//====== constructor ======
MediaPlayers::MediaPlayers(){
	//====== lock the mutex before we do anything ======
	std::lock_guard<std::mutex> lock(this->players_mutex);

	//====== get all clients on session bus ======
	QDBusConnection session_bus = QDBusConnection::sessionBus();
	QDBusConnectionInterface *session_bus_interface = session_bus.interface();
	QDBusReply<QStringList> response = session_bus_interface->registeredServiceNames();
	if (!response.isValid()){
		throw std::runtime_error(response.error().message().toStdString());
	}
	QStringList dbus_clients = response.value();
	
	//====== filter for org.mpris ======
	QStringList mpris_clients = dbus_clients.filter(QRegularExpression("^org.mpris.MediaPlayer2"));

	//====== create callback for when new media players pop up ======
	session_bus.connect("org.freedesktop.DBus","/org/freedesktop/DBus","org.freedesktop.DBus","NameOwnerChanged",this,SLOT(dbus_clients_change(QString,QString,QString)));

	//====== add to list of players ======
	for (const QString &name : mpris_clients){
		qDebug() << "found player" << name;
		Player *current_player = new Player(name);
		this->players.push_front(current_player);
	}
}
Player::Player(QString address){
	this->address = address;
	this->properties_interface = new QDBusInterface(address,"/org/mpris/MediaPlayer2","org.freedesktop.DBus.Properties",QDBusConnection::sessionBus());
}
Track *MediaPlayers::get_current_track(){
	Track *current_track = new Track;
	return current_track;
}

void MediaPlayers::dbus_clients_change(QString name, QString old_owner, QString new_owner){
	//====== filter for mpris media players ======
	if (name.contains(QRegularExpression("^org.mpris.MediaPlayer2"))){
		//lets be thread safe for the sake of everyone
		std::lock_guard<std::mutex> lock(this->players_mutex);
		
		if (old_owner == "" && new_owner != ""){
			//====== new player added =======
			qDebug() << name << "added";
			//add it to the list
			Player *current_player = new Player(name);
			this->players.push_back(current_player);

		} else if (new_owner == "" && old_owner != ""){
			//====== player removed ======
			//find and remove it
			for (std::list<Player *>::iterator players_iterator = this->players.begin();players_iterator != this->players.end();){
				if ((*players_iterator)->name() == name){
					qDebug() << name << "removed";
					delete *players_iterator;
					players_iterator = this->players.erase(players_iterator);
				}else{
					players_iterator++;
				}
			}
		}
	}
}

MediaPlayers::~MediaPlayers(){
	for (Player *player : this->players){
		delete player;
	}
}
Player::~Player(){
	delete this->properties_interface;
}
QString Player::name(){
	return this->address;
}
