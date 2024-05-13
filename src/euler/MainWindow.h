#pragma once

#include <QMainWindow>
#include <interactive_markers/interactive_marker_server.h>
#include <QTimer>
#include <QMutex>
#include <Eigen/Geometry>


class RotationControl;

class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

signals:

public slots:
		void updateOrientation();
private:
	void setupUi();

private:
	boost::shared_ptr<interactive_markers::InteractiveMarkerServer> server;
	ros::AsyncSpinner spinner;

	RotationControl *frame1;
	RotationControl *frame2;
	RotationControl *frame1p2;
	RotationControl *frame1c2;
	RotationControl *frame1Alt;
};

class Interpolation : public QTimer
{
	Q_OBJECT
public:
	Interpolation(const Eigen::Quaterniond &q1 = Eigen::Quaterniond::Identity(),
	              const Eigen::Quaterniond &q2 = Eigen::Quaterniond::Identity(),
	              QObject* parent = Q_NULLPTR);

public slots:
	void setStart(const Eigen::Quaterniond &q1);
	void setEnd(const Eigen::Quaterniond &q2);

signals:
	void valueChanged(const Eigen::Quaterniond &q);

private slots:
	void step();

private:
	QMutex _mutex;
	Eigen::Quaterniond _q1, _q2;
	double _t;
};