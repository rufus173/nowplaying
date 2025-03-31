#include "media_players.h"
#include <QMap>
#include <QDBusConnection>
#include <QStringList>
#include <QDebug>
#include <QDBusReply>
#include <QDBusConnectionInterface>
#include <QString>
#include <QRegularExpression>



//================ MediaPlayers class ===============



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
		//whenever a player is unpaused/started move it to the front of the list
		QObject::connect(current_player,&Player::started_playing,this,&MediaPlayers::move_player_to_front);
	}
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

void MediaPlayers::move_player_to_front(QString name){
	std::lock_guard<std::mutex> lock(this->players_mutex);
	//====== search for it in the list ======
	std::list<Player *>::iterator iter = this->players.begin();
	qDebug() << this->players;
	for (;iter != this->players.end();){
		if ((*iter)->name() == name){
			qDebug() << "\nmoving player to front:" << name << "\n";
			Player *player_to_move = *iter;
			//remove it
			this->players.erase(iter);
			//re-add it at the begining
			this->players.push_front(player_to_move);
			break;
		}
	}
	//qDebug() << this->players;
}

MediaPlayers::~MediaPlayers(){
	for (Player *player : this->players){
		delete player;
	}
}

int MediaPlayers::get_current_track_position(){
	//time is in ms so contert to s
	return this->players.front()->get_current_position()/1000000;
}

int MediaPlayers::get_current_track_length(){
	QVariantMap properties = *(this->players.front()->properties());
	QVariantMap metadata = qdbus_cast<QVariantMap>(properties["Metadata"].value<QDBusArgument>());
	//mpris does not require track length be present, so return a default value if one isnt present
	if (metadata.keys().contains("mpris:length")){
		return metadata["mpris:length"].toLongLong() / 1000000; //conv to s from ms
	}else{
		return -1;
	}
}

QString MediaPlayers::get_current_track_name(){
	QVariantMap properties = *(this->players.front()->properties());
	QVariantMap metadata = qdbus_cast<QVariantMap>(properties["Metadata"].value<QDBusArgument>());
	if (metadata.keys().contains("xesam:title")){
		return metadata["xesam:title"].toString(); //conv to s from ms
	}else{
		return QString("No Title");
	}
}
QString MediaPlayers::get_current_track_artist(){
	QVariantMap properties = *(this->players.front()->properties());
	QVariantMap metadata = qdbus_cast<QVariantMap>(properties["Metadata"].value<QDBusArgument>());
	if (metadata.keys().contains("xesam:artist")){
		return metadata["xesam:artist"].toString(); //conv to s from ms
	}else{
		return QString("No Artist");
	}
}



//================ Player class ================






Player::Player(QString address){
	std::lock_guard<std::recursive_mutex> guard(this->attributes_mutex);
	this->address = address;

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
	std::lock_guard<std::recursive_mutex> guard(this->attributes_mutex);
	return this->address;
}

void Player::dbus_properties_changed(QString name, const QVariantMap changed_properties, QStringList invalaid_properties){
	//up here threading my safety 
	//ima be so real i have no idea if this is nessecary or not but im doing it just in case
	std::lock_guard<std::recursive_mutex> guard(this->attributes_mutex);

	qDebug() << "======" << this->address << "======";
	qDebug() << "--- from ---\n" << this->player_properties;

	//====== if the player has given an updated time property update it ======
	if (changed_properties.keys().contains("Position")){
		qDebug() << "\nupdated position to" << changed_properties["Position"] << "\n";
		this->current_track_position = changed_properties["Position"].toLongLong()/1000000;
	}

	//====== if it has gone from paused to playing, emit started_playing() ======
	//this allows the MediaPlayers class to move it to the front of the list
	if (this->player_properties["PlaybackStatus"].toString() != changed_properties["PlaybackStatus"].toString()){
		if (changed_properties["PlaybackStatus"].toString() == "Playing"){
			//qDebug() << "player just started playing";
			emit started_playing(this->address);
		}
	}

	//====== merge new properties with old ones to update them ======
	this->player_properties.insert(changed_properties);
	qDebug() << "--- to ---\n" << this->player_properties;
	
	//qDebug() << this->player_properties;
	//qDebug() << "// metadata";
 	//qDebug() << qdbus_cast<QVariantMap>(this->player_properties["Metadata"].value<QDBusArgument>());

}
int64_t Player::get_current_position(){ //no need for mutex as the only attribute we use is the address, which never changes.
	//====== if it is stopped, set current track position attribute to 0 ======
	if (this->player_properties["PlaybackStatus"].toString() == "Stopped") this->current_track_position = 0;

	//====== if its playing, use dbus to get the value ======
	if (this->player_properties["PlaybackStatus"].toString() == "Playing"){
		//get the Position property
		QDBusInterface properties_interface = QDBusInterface(address,"/org/mpris/MediaPlayer2","org.freedesktop.DBus.Properties",QDBusConnection::sessionBus());
		QDBusReply<QDBusVariant> reply = properties_interface.call("Get","org.mpris.MediaPlayer2.Player","Position");
		//check it worked
		if (!reply.isValid()){
			throw std::runtime_error(reply.error().message().toStdString());
		}
		//qDebug() << "received" << reply.value().variant();
		this->current_track_position = reply.value().variant().toLongLong();
		return reply.value().variant().toLongLong();
	//====== if its paused/stopped, retreive from stored value ======
	}else{
		qDebug() << "track paused";
		qDebug() << this->player_properties["PlaybackStatus"].toString();
		return this->current_track_position;
	}
}

QVariantMap *Player::properties(){
	return &this->player_properties;
}
