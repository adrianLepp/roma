// include headers declaring the used class interfaces
#include "MainWindow.h"
#include "RotationControl.h"

// these are system includes (from Qt, Eigen, ROS)
#include <QVBoxLayout>
#include <Eigen/Core>
#include <ros/ros.h>
#include <QMutexLocker>


MainWindow::MainWindow(QWidget *parent) :
   QMainWindow(parent), spinner(1)
{
	server.reset(new interactive_markers::InteractiveMarkerServer("euler","",false));
	setupUi();

	server->applyChanges();
	spinner.start();
}

MainWindow::~MainWindow()
{
	spinner.stop();
}

// keep axes in sync between the two RotationControl widgets f1 and f2
static void linkAxes(RotationControl *f1, RotationControl *f2) {
	// f1 -> f2
	QObject::connect(f1, SIGNAL(axesChanged(uint,uint,uint)),
	                 f2, SLOT(setEulerAxes(uint,uint,uint)));
	// f2 -> f1
	QObject::connect(f2, SIGNAL(axesChanged(uint,uint,uint)),
	                 f1, SLOT(setEulerAxes(uint,uint,uint)));
}

void MainWindow::setupUi() {
	QWidget *central = new QWidget(this);

	// create 4 RotationControl widgets
	double s = 0.5;
	QColor grey("grey"), red("red"), green("green");
	frame1 = new RotationControl("frame 1", Eigen::Vector3d(-s,s,0), grey, server, this);
	frame2 = new RotationControl("frame 2", Eigen::Vector3d( s,s,0), grey, server, this);
	frame1p2 = new RotationControl("frame 1+2", Eigen::Vector3d(-2*s,-s,0), red, server, this);
	frame1c2 = new RotationControl("frame 1*2", Eigen::Vector3d( 2*s,-s,0), green, server, this);
	frame1Alt = new RotationControl("frame 1 alternative", Eigen::Vector3d(0,3*s,0), QColor("yellow"), server, this);

	// add those widgets to the vertical layout of the MainWindow's central widget
	QVBoxLayout *layout = new QVBoxLayout(central);
	layout->addWidget(frame1);
	layout->addWidget(frame2);
	layout->addWidget(frame1p2);
	layout->addWidget(frame1c2);
	layout->addWidget(frame1Alt);

	this->setCentralWidget(central);

	// keep Euler axes in sync between all widgets
	linkAxes(frame1, frame2);
	linkAxes(frame1, frame1p2);
	linkAxes(frame1, frame1c2);

	connect(frame1, SIGNAL(valueChanged(Eigen::Quaterniond)), this, SLOT(updateOrientation()));
	connect(frame2, SIGNAL(valueChanged(Eigen::Quaterniond)), this, SLOT(updateOrientation()));

	RotationControl *frameSlerp = new RotationControl("interpolated", Eigen::Vector3d(0,-s,0), QColor("blue"), server, this);
	layout->addWidget(frameSlerp);
	Interpolation *timer = new Interpolation(frame1->value(), frame2->value(), this);
	connect(frame1, SIGNAL(valueChanged(Eigen::Quaterniond)), timer, SLOT(setStart(Eigen::Quaterniond)));
	connect(frame2, SIGNAL(valueChanged(Eigen::Quaterniond)), timer, SLOT(setEnd(Eigen::Quaterniond)));
	connect(timer, SIGNAL(valueChanged(Eigen::Quaterniond)), frameSlerp, SLOT(setValue(Eigen::Quaterniond)));
	linkAxes(frame1, frameSlerp);
	timer->start(50);

}

void MainWindow::updateOrientation()
{
	Eigen::Quaterniond q = frame2->value() * frame1->value();
	frame1c2->setValue(q);

	Eigen::Vector3d e = frame1->eulerAngles() + frame2->eulerAngles();
	frame1p2->setEulerAngles(e[0], e[1], e[2]);

	frame1Alt->setEulerAxes(2,1,2);
	frame1Alt->setValue(frame1->value());
}

Interpolation::Interpolation(const Eigen::Quaterniond &q1,
                             const Eigen::Quaterniond &q2,
                             QObject* parent)
   :_q1(q1), _q2(q2)
{
	_t = 0.0;
	connect(this, SIGNAL(timeout()), this, SLOT(step()));
}

void Interpolation::setStart(const Eigen::Quaterniond &q)
{
	QMutexLocker lock(&_mutex);
	_q1 = q;
	_t = 0;
}

void Interpolation::setEnd(const Eigen::Quaterniond &q)
{
	QMutexLocker lock(&_mutex);
	_q2 = q;
	_t = 0;
}

void Interpolation::step()
{
	QMutexLocker lock(&_mutex);
	_t += 0.05;
	if (_t > 1.) _t = -1.;
	emit valueChanged(_q1.slerp(std::fabs(_t), _q2));
}