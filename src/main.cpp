#include <QApplication>
#include <QTimer>
#include <QSizePolicy>
#include <QLabel>
#include <QGridLayout>
#include <QGraphicsEffect>
#include <QGraphicsOpacityEffect>
#include <QDBusConnection>
#include <QWindow>
#include <QFile>
#include <QCursor>

#include <unistd.h>
#include <stdio.h>
#include <wordexp.h>

#include "media_players.h"
#include "main.h"
int main(int argc, char **argv){
	QApplication app = QApplication(argc,argv);

	//====== check we are connected to the session bus ======
	if (!QDBusConnection::sessionBus().isConnected()){
		fprintf(stderr,"Could not connect to sesion bus.\n");
		exit(EXIT_FAILURE);
	}

	//====== load a custom style sheet if the user has one ======
	wordexp_t expanded_expression;
	if (int result = wordexp("~/.config/nowplaying/style.css",&expanded_expression,0) == 0){
		for (size_t i = 0; i < expanded_expression.we_wordc; i++){
			qDebug() << "checking" << QString(expanded_expression.we_wordv[i]);
			QFile stylesheet_file = QFile(expanded_expression.we_wordv[i]);
			if (stylesheet_file.exists()){
				stylesheet_file.open(QFile::ReadOnly);
				QString stylesheet = QLatin1String(stylesheet_file.readAll());
				app.setStyleSheet(stylesheet);
				stylesheet_file.close();
				qDebug() << "using stylesheet" << QString(expanded_expression.we_wordv[i]);
				break;
			}
		}
		wordfree(&expanded_expression);
	}else{
		qDebug() << "wordexp error:" << result;
	}

	//====== create the main window ======
	MainWindow main_window = MainWindow();
	main_window.setWindowFlags(main_window.windowFlags() | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
	main_window.setAttribute(Qt::WA_TranslucentBackground);
	//====== display everything ======
	main_window.show();

	app.exec();
}
MainWindow::MainWindow(QWidget *parent) : QWidget(parent,Qt::FramelessWindowHint){
	//misc
	this->players = new MediaPlayers();
	this->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);

	//====== main window ======
	//so we can detect when the mouse hovers over
	this->setAttribute(Qt::WA_MouseTracking,true);

	//====== grid layout and widgets ======
	this->grid = new QGridLayout(this);
	this->song_progress_bar = new QProgressBar();
	this->song_progress_bar->setTextVisible(false);
	this->song_info_label = new QLabel("nothing playing");
	this->song_remaining_time_label = new QLabel("-/-");
	this->song_remaining_time_label->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);
	//                                                    row column rowspan columnspan
	this->grid->addWidget(this->song_progress_bar,        1,  0,     1,      2);
	this->grid->addWidget(this->song_info_label,          0,  0,     1,      3);
	this->grid->addWidget(this->song_remaining_time_label,1,  2,     1,      1);

	//====== update loop timer ======
	this->update_loop_timer = new QTimer(this);
	connect(update_loop_timer,&QTimer::timeout,this,&MainWindow::update_ui);
	//                             0.5s
	this->update_loop_timer->start(500);

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

	//====== update the window size ======
	this->adjustSize();
}
MainWindow::~MainWindow(){
	delete this->players;
}
void MainWindow::mouseMoveEvent(QMouseEvent *event){
	//this is triggered when the mouse hovers over the widget
	this->hide();
	qDebug() << QCursor::pos();
	qDebug() << this->geometry();
	//schedule an attempt to reopen the window
	QTimer::singleShot(1,this,&MainWindow::attemptReappear);
}
void MainWindow::attemptReappear(){
	qDebug()<<"reappearing";
}

