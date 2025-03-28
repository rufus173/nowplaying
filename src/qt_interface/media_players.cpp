#include "media_players.h"
#include <QMap>
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
Player::Player(QString address){
	std::lock_guard<std::mutex> guard(this->attributes_mutex);
	this->address = address;
	//====== get initial properties ======
	QDBusInterface properties_interface = QDBusInterface(address,"/org/mpris/MediaPlayer2","org.freedesktop.DBus.Properties",QDBusConnection::sessionBus());
	QDBusReply<QVariantMap> reply = properties_interface.call("GetAll","org.mpris.MediaPlayer2.Player");
	//check it worked
	if (!reply.isValid()){
		throw std::runtime_error(reply.error().message().toStdString());
	}
	//store in object's player_properties attribute
	this->player_properties.insert(reply.value());

	//====== connect properties changing to relevant function ======
	QDBusConnection::sessionBus().connect(address,"/org/mpris/MediaPlayer2","org.freedesktop.DBus.Properties","PropertiesChanged",this,SLOT(dbus_properties_changed(QString, QVariantMap, QStringList)));
}

Player::~Player(){
}

QString Player::name(){
	std::lock_guard<std::mutex> guard(this->attributes_mutex);
	return this->address;
}

void Player::dbus_properties_changed(QString name, QVariantMap changed_properties, QStringList invalaid_properties){
	std::lock_guard<std::mutex> guard(this->attributes_mutex);
	//====== merge new properties with old ones to update them ======
	this->player_properties.insert(changed_properties);
	
	qDebug() << "======" << this->address << "======";
	qDebug() << this->player_properties;
}
