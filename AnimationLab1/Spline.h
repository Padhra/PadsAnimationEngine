#pragma once 

#include <glm\glm.hpp>
#include <vector>
#include "Helper.h"

class Spline
{
	private:

	public:

		std::vector<std::pair<double, glm::vec3>> keyframes;
		double timer; //in seconds

		//short mode; // 0 = Linear, 1 = cubic

		//double totalDuration;

		void AddKeyframe(double t, glm::vec3 keyframe) 
		{
			keyframes.push_back(std::make_pair(t, keyframe));
		}

		glm::vec3 GetPosition()
		{
			bool reset = true;
			int prev_key = 1;
			int next_key = 1;
		
			//Find the two keyframes
			for (int keyidx = 1; keyidx < keyframes.size() - 2; keyidx++) 
			{
				prev_key = keyidx;
				next_key = keyidx + 1;

				if (keyframes[next_key].first >= timer) // if the next keyframe is greater than the timer, then we have our two keyframes
				{
					reset = false;
					break;
				}
			}

			if(reset)
			{
				prev_key = 1;
				next_key = 2;
				timer = 0;
			}

		
			float timeBetweenKeys = keyframes[next_key].first - keyframes[prev_key].first;
			float t = (timer - keyframes[prev_key].first) / timeBetweenKeys; 

			return cubicLerp(keyframes[prev_key-1].second, keyframes[prev_key].second, keyframes[next_key].second, keyframes[next_key+1].second, t);
		}

		void Update(double deltaTime)
		{
			timer += deltaTime/1000;
		}
};
