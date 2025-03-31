#include <QApplication>
#include <QTimer>
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
MainWindow::MainWindow(QWidget *parent) : QWidget(parent,Qt::FramelessWindowHint){
	//mediaplayers object
	this->players = new MediaPlayers();

	//====== grid layout and widgets ======
	this->grid = new QGridLayout(this);
	this->song_progress_bar = new QProgressBar();
	this->song_progress_bar->setTextVisible(false);
	this->song_info_label = new QLabel("nothing playing");
	this->song_remaining_time_label = new QLabel("-/-");
	//                                                    row column rowspan columnspan
	this->grid->addWidget(this->song_progress_bar,        1,  0,     1,      2);
	this->grid->addWidget(this->song_info_label,          0,  0,     1,      3);
	this->grid->addWidget(this->song_remaining_time_label,1,  2,     1,      1);

	//====== update loop timer ======
	this->update_loop_timer = new QTimer(this);
	connect(update_loop_timer,&QTimer::timeout,this,&MainWindow::update_ui);
	//                             0.5s
	this->update_loop_timer->start(500);

	//====== display everything ======
	this->show();
}
void MainWindow::update_ui(){
	//====== update the time remaining label ======
	int track_time_passed = this->players->get_current_track_position();
	int track_length = this->players->get_current_track_length();
	QString time_remaining_text;
	time_remaining_text = QString("%1:%2 / %3:%4")
		//current time
		//minutes
		.arg((int)(track_time_passed/60))
		//seconds (padded with leading 0s)
		.arg(track_time_passed % 60,2,10,QLatin1Char('0'))

		//time remaining
		//minutes
		.arg((int)(track_length/60))
		//seconds (padded with leading 0s)
		.arg(track_length % 60,2,10,QLatin1Char('0'));

	this->song_remaining_time_label->setText(time_remaining_text);

	//====== update the progressbar ======
	this->song_progress_bar->setValue(track_time_passed);
	this->song_progress_bar->setMaximum(track_length);

	//====== update the artist and song label ======
	QString song_info_text;
	song_info_text = this->players->get_current_track_name() + " | " + this->players->get_current_track_artist();
	this->song_info_label->setText(song_info_text);
}
MainWindow::~MainWindow(){
	delete this->players;
}
