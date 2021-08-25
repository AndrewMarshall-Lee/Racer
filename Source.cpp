#include <iostream>
#include <algorithm>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
using namespace std;


class OLC_AndrewRun : public olc::PixelGameEngine
{
public:
	OLC_AndrewRun()
	{
		sAppName = "AndrewRun";

	}
	olc::Sprite* sprCar = nullptr;
	olc::Decal* decCar = nullptr;

private:
	//Car goes from -1 to +1, 0 being centre
	float fCarPos = 0.0f;
	float fDistance = 0.0f;
	float fTrackCurvature = 0.0f;
	float fSpeed = 0.0f;
	float fPlayerCurvature = 0.0f;
	float fTrackDistance = 0.0f;
	float fLaptime = 0.0f;

	//player curvature means checking the total turning of player
	//This'll be used to check if it has gone off track.
	
	bool bLapComplete = false;
	
	
	//define track component: length, curvature
	vector<pair<float, float>> vecTrack;


public:
	bool OnUserCreate() override
	{
		
		sprCar = new olc::Sprite("./Sprites/Car.png");
		decCar = new olc::Decal(sprCar);


		float fCurve, fLength;
		string sCurve, sLength;
		//create track from file
		std::ifstream datTrack("./Tracks/1.dat");
		
		while (datTrack >> sCurve >> sLength) {
			//cout << sCurve << " " << sLength << endl;
			fCurve = stof(sCurve);
			fLength = stof(sLength);
			printf("%f %f \n", fCurve, fLength);
			vecTrack.push_back(make_pair(fCurve, fLength));
		}
		
		for (auto t: vecTrack)
			fTrackDistance += t.second;
		

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		if (GetKey(olc::Key::UP).bHeld)
			fSpeed += 1.0f * fElapsedTime;
		else
			fSpeed -= 1.0f * fElapsedTime;
		if (GetKey(olc::Key::RIGHT).bHeld && fSpeed > 0.0f)
			fPlayerCurvature += 0.5f  * fElapsedTime;
		if (GetKey(olc::Key::LEFT).bHeld && fSpeed > 0.0f)
			fPlayerCurvature -= 0.5f  * fElapsedTime;


		//limit speed
		if (fSpeed < 0.0f) fSpeed = 0.0f;
		if (fSpeed > 1.0f) fSpeed = 1.0f;

		



		//get point on track
		float fOffset = 0;
		int nTrackSection = 0;
		fDistance += (75.0f * fSpeed) * fElapsedTime;

		//Loop track
		fLaptime += fElapsedTime;
		if (fDistance >= fTrackDistance)
		{
			fDistance -= fTrackDistance;
			bLapComplete = true;
			fLaptime -= fLaptime;
		}
		else
		{
			bLapComplete = false;
		}
		//when total distance goes beyond sum of track sections (fOffset) we add the next track section
		while (nTrackSection < vecTrack.size() && fOffset <= fDistance)
		{
			fOffset += vecTrack[nTrackSection].second;
			nTrackSection++;
		}

		//Set curvature of current track section
		//Similar to perspective the middle point is moved wrt distance and curve
		float fTargetCurvature = vecTrack[nTrackSection - 1].first;
		//Smoothly move track
		float fTrackCurveDiff = (fTargetCurvature - fTrackCurvature) * fSpeed *  fElapsedTime;
		fTrackCurvature += fTrackCurveDiff;


		//Slow Car if going off track
		if (fabs(fPlayerCurvature - fTrackCurvature) >= 0.5f)
		{
			fSpeed -= 3.0f * fElapsedTime;
			if (GetKey(olc::Key::UP).bHeld && fSpeed < 0.5)
				fSpeed = 0.5f * fElapsedTime;

		}
		fCarPos = fPlayerCurvature - fTrackCurvature;
		Clear(olc::BLACK);
		//draw a track
		for (int y = 0; y < ScreenHeight() / 2; y++)
		{
			//classify components (road, grass, background etc)
			for (int x = 0; x < ScreenWidth(); x++)
			{
				//Adding perspective variable, higher the y the thinner the road appears
				float fPerspective = (float)y / (ScreenHeight() / 2.0f);

				//screen width normalised between 0 and 1
				//RoadWidth governs all other variables, so adjust for persoective will change all others
				float fMiddlePoint = 0.5f + fTrackCurvature * powf((1.0f- fPerspective),4);

				float fRoadWidth = 0.1f + fPerspective*0.8f;
				float fClipWidth = fRoadWidth * 0.15f;

				fRoadWidth *= 0.5f;

				int nLeftGrass = (fMiddlePoint - fRoadWidth - fClipWidth) * ScreenWidth();
				int nLeftClip = (fMiddlePoint - fRoadWidth) * ScreenWidth();
				
				int nRightGrass = (fMiddlePoint + fRoadWidth + fClipWidth) * ScreenWidth();
				int nRightClip = (fMiddlePoint + fRoadWidth) * ScreenWidth();

				//Draw to bottom half of screen (background will be at the top)
				int nRow = ScreenHeight() / 2 + y;

				//to Give illusion of motion, grass and clipboard will have rows of colour
				//sin function is used to give perspective 
				int nGrassColour = sinf(20.0f * powf(1.0f - fPerspective, 3) + fDistance * 0.1f) > 0.0f ? 255 : 100;
				int nClipColour = sinf(60.0f * powf(1.0f - fPerspective, 2) + fDistance * 0.1f) > 0.0f ? 255 : 10;

				int RoadColour = bLapComplete ? 255 : 211;

				if (x >= 0 && x < nLeftGrass)
					Draw(x, nRow, olc::Pixel(0, nGrassColour, 0));
				if (x >= nLeftGrass && x < nLeftClip)
					Draw(x, nRow, olc::Pixel(255, nClipColour, nClipColour));
				if (x >= nLeftClip && x < nRightClip)
					Draw(x, nRow, olc::Pixel(RoadColour, RoadColour, RoadColour));
				if (x >= nRightClip && x < nRightGrass)
					Draw(x, nRow, olc::Pixel(255, nClipColour, nClipColour));
				if (x >= nRightGrass && x < ScreenWidth())
					Draw(x, nRow, olc::Pixel(0, nGrassColour, 0));

			}
		}

		//Draw Sky
		for (int y = 0; y < ScreenHeight() / 2; y++)
			for (int x = 0; x < ScreenWidth(); x++)
				Draw(x, y, olc::Pixel(135, 206, 235, 255));

		//Draw Scenery
		for (int x = 0; x < ScreenWidth(); x++)
		{
			int nHillHeight = (int)(fabs(sinf(x * 0.01f + fTrackCurvature) * 16.0f));
			for (int y = ScreenHeight() / 2 - nHillHeight; y < ScreenHeight() / 2; y++)
				Draw(x, y, olc::Pixel(8, 206, 0));
		}
	 
		//DrawCar
		int nCarPos = ScreenWidth() / 2 + ((int)(ScreenWidth() * fCarPos) / 2.0f);
		olc::vf2d vecCarPos = {float(nCarPos), float(4*ScreenHeight()/5) };
		DrawDecal(vecCarPos, decCar, { 0.05f, 0.05f });
		return true;


	}
};
int main()
{
	OLC_AndrewRun game;
	game.Construct(160, 100, 8, 8);
	game.Start();
	return 0;
}


