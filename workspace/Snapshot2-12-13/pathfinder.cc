/*
 * ex0.cc
 *
 *  Created on: Dec 29, 2012
 *      Author: kanaad
 */
#include <stdio.h>
#include <iostream>
#include <libplayerc++/playerc++.h>
#include <sstream>
using namespace std;
using namespace PlayerCc;
bool detectCrash(Position2dProxy *p2dProxy, int robotnum) {
	if (p2dProxy->GetStall()) {

		cerr << "Robot " << robotnum << " has crashed at " << "("
				<< p2dProxy->GetXPos() << ", " << p2dProxy->GetYPos()
				<< ")! \n";
		return true;
	} else {
		return false;
	}
}
int onCrash(SimulationProxy *playerSim, int robotnum){
	stringstream ss;
	ss << "car" << robotnum;
	std::string cname = ss.str();
	cout << "Robot " << robotnum << ": " << cname << " has crashed and is now being removed"<< "\n";
	playerSim->SetIntProp("asdf", 10);
	playerSim->SetPose2d((char*)cname.c_str(), 20, 22, 0);
	cout << "Robot " << robotnum << ": " << cname << " has been removed and disabled"<< "\n";
	return 0;
}
bool lidarScan(RangerProxy *ranger) {
	for (double checkangle = 60; checkangle <= 120;
			checkangle = checkangle + ranger->GetAngularRes()) {
		double maxrange = ranger->GetMaxRange();
		double checkrange = ranger->GetRange(checkangle);
		//cout << "Robot " << robotnum << ": " << "(" << checkangle << ", " << checkrange << ")! \n";
		if (checkrange < maxrange) {
			return true;
		}
	}
	return false;
}
int main(int argc, char* argv[]) {
	//command line robot, turn, base speed, turn speed
	if (argc < 4){
		cerr << "Invalid number of arguments!" << "\n"
		return 0;
	}
	int robotnum = atoi(argv[1]);
	bool turn;
	if(strcmp(argv[2], "1")==0){
		 turn = true;
	}
	else{
	turn = false;
	}
	double basespeed = atof(argv[3]);
	double turnspeed = atof(argv[4]);

	bool crashed = false;
	double DECELERATION = .85;
	cout << "Robot " << robotnum << ": " << "Robot " << robotnum << "\n";
	cout << "Robot " << robotnum << ": " << "Turning = " << turn << "\n";
	cout << "Robot " << robotnum << ": " << "Base speed = " << basespeed << "\n";
	cout << "Robot " << robotnum << ": " << "Turn speed = " << turnspeed << "\n";

	try {
		PlayerClient client("localhost");
		Position2dProxy p2dProxy(&client, robotnum);
		RangerProxy ranger(&client, robotnum);
		SimulationProxy playerSim(&client, 0);
		client.Read();
		playerSim.SetIntProp("asdf", (int32_t)20);
		int32_t i = 30;
		playerSim.GetIntProp("asdf", &i);
		cout << "==================="<<i << "\n";
	//	cout << "Robot " << robotnum << ": " << "G"et Range Count = " << ranger.GetRangeCount() << "\n";
	//	cout << "Robot " << robotnum << ": " << "Get Max Range = " << ranger.GetMaxRange() << "\n";
	//	cout << "Robot " << robotnum << ": " << "Get Min Range  = " << ranger.GetMinRange() << "\n";
	//	cout << "Robot " << robotnum << ": " << "Get Max Angle = " << ranger.GetMaxAngle() << "\n";
	//	cout << "Robot " << robotnum << ": " << "Get Min Angle  = " << ranger.GetMinAngle() << "\n";
	//	cout << "Robot " << robotnum << ": " << "Get Angular Res = " << ranger.GetAngularRes() << "\n";

		client.Read();
		bool pathblocked = false;
		double LegOneMotion = basespeed;
		double LegTwoMotion = basespeed;
		double LegThreeMotion = basespeed;
		double changeangle = M_PI / 4;
		double robotx = p2dProxy.GetXPos();
		double roboty = p2dProxy.GetYPos();
		double robotyaw = p2dProxy.GetYaw();
		cout << "Robot " << robotnum << ": " << "robotx =" << robotx << "\n";
		cout << "Robot " << robotnum << ": " << "roboty =" << roboty << "\n";
		cout << "Robot " << robotnum << ": " << "robotyaw =" << robotyaw << "\n";
		cout << "Robot " << robotnum << ": " << "LEG 1" << "\n";
		while (fabs(roboty) > 3.0 || fabs(robotx) > 3.0) {
			//cout << "Robot " << robotnum << ": " << "LEG 1" << "\n";
			client.Read();
			robotx = p2dProxy.GetXPos();
			roboty = p2dProxy.GetYPos();
			robotyaw = p2dProxy.GetYaw();

			if (lidarScan(&ranger)) {
				LegOneMotion = LegOneMotion * DECELERATION;
				cout << "Robot " << robotnum << ": " << "There's something in the way!" << "\n";
				pathblocked = true;
			} else {
				LegOneMotion = basespeed;
				pathblocked = false;
			}
			p2dProxy.SetSpeed(LegOneMotion, 0, 0);

			if (detectCrash(&p2dProxy, robotnum)) {
				onCrash(&playerSim, robotnum);
				crashed = true;
				break;
			}
		}

		if (crashed) {
			return 0;
		}
		double idealangle = robotyaw + changeangle;
		if (idealangle > M_PI) {
			idealangle = idealangle - (2) * (M_PI);

		}
		if (turn) {
			cout << "Robot " << robotnum << ": " << "yaw movement" << "\n";
			cout << "Robot " << robotnum << ": " << "ideal angle =" << idealangle;
			while (fabs(idealangle - robotyaw) > .05) {
				//cout << "Robot " << robotnum << ": " << "yaw movement" << "\n";
				p2dProxy.SetSpeed(0, 0, turnspeed);
				client.Read();
				robotx = p2dProxy.GetXPos();
				roboty = p2dProxy.GetYPos();
				robotyaw = p2dProxy.GetYaw();

				if (detectCrash(&p2dProxy, robotnum)) {
					onCrash(&playerSim, robotnum);
					crashed = true;
					break;
				}
			}
			if (crashed) {
				return 0;
			}
		}
		cout << "Robot " << robotnum << ": " << "LEG 2" << "\n";
		while (fabs(roboty) < 3.0 && fabs(robotx) < 3.0) {
			//cout << "Robot " << robotnum << ": " << "LEG 2" << "\n";
			client.Read();
			robotx = p2dProxy.GetXPos();
			roboty = p2dProxy.GetYPos();
			robotyaw = p2dProxy.GetYaw();
			if (lidarScan(&ranger)) {
				LegTwoMotion = LegTwoMotion * DECELERATION;
				pathblocked = true;
			}
			else{
				LegTwoMotion = basespeed;
				pathblocked = false;
			}
			p2dProxy.SetSpeed(LegTwoMotion, 0, 0);

			if (detectCrash(&p2dProxy, robotnum)) {
				onCrash(&playerSim, robotnum);
				crashed = true;
				break;
			}
		}
		if (crashed) {
			return 0;
		}
		if (turn) {
			cout << "Robot " << robotnum << ": " << "yaw movement" << "\n";
			idealangle = robotyaw + changeangle + .05;
			while (fabs(idealangle - robotyaw) > .1) {
				//cout << "Robot " << robotnum << ": " << "idang" << idealangle << "\n";
				//cout << "Robot " << robotnum << ": " << "yaw movement" << "\n";
				p2dProxy.SetSpeed(0, 0, turnspeed);
				client.Read();
				robotx = p2dProxy.GetXPos();
				roboty = p2dProxy.GetYPos();
				robotyaw = p2dProxy.GetYaw();
				if (detectCrash(&p2dProxy, robotnum)) {
					onCrash(&playerSim, robotnum);
					crashed = true;
					break;
				}
			}
			if (crashed) {
				return 0;
			}
		}
		cout << "Robot " << robotnum << ": " << "LEG 3" << "\n";
		while (fabs(roboty) < 18.0 || fabs(robotx) < 18.0) {
			//cout << "Robot " << robotnum << ": " << "LEG 3" << "\n";
			client.Read();
			robotx = p2dProxy.GetXPos();
			roboty = p2dProxy.GetYPos();
			robotyaw = p2dProxy.GetYaw();
			if (lidarScan(&ranger)) {
				LegThreeMotion = LegThreeMotion * DECELERATION;
				pathblocked = true;
			} else {
				LegThreeMotion = basespeed;
				pathblocked = false;
			}
			p2dProxy.SetSpeed(LegThreeMotion, 0, 0);

			if (detectCrash(&p2dProxy, robotnum)) {
				onCrash(&playerSim, robotnum);
				crashed = true;
				break;
			}
		}
		if (crashed) {
			return 0;
		}

	} catch (PlayerCc::PlayerError &e) {
		const char* c = e.GetErrorStr().c_str();
		cout << "Robot " << robotnum << ": " << c;
	}
	cout << "Robot " << robotnum << ": " << "Finished!";
	return 0;
}

