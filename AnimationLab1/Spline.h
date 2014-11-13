#pragma once 

#include <glm\glm.hpp>
#include <vector>
#include "Helper.h"
#include "Node.h"

enum InterpolationMode { Linear = 0, Cubic };

class Spline
{
	private:

	public:

		std::vector<Node*> nodes;

		double timer; //in seconds
		static float speedScalar;

		int currentNode;

		short mode; // 0 = Linear, 1 = cubic

		Spline()
		{
			mode = InterpolationMode::Cubic;

			currentNode = 0;
		}

		void SetMode(short mode) { this->mode = mode;}

		void SetSpeed(float speed) { speedScalar = speed;}

		glm::vec3 GetPosition()
		{
			if(timer >= 1.0)
			{
				currentNode++;
				timer = 0;
			}
			
			float t = timer / 1.0; 

			if(mode == InterpolationMode::Cubic)
			{
				return cubicLerp(nodes[(currentNode-1) % nodes.size()]->GetPosition(), 
					nodes[currentNode % nodes.size()]->GetPosition(), nodes[(currentNode+1) % nodes.size()]->GetPosition(), 
					nodes[(currentNode+2) % nodes.size()]->GetPosition(), t);
			}
			else
			{
				return lerp(nodes[currentNode % nodes.size()]->GetPosition(), nodes[(currentNode+1) % nodes.size()]->GetPosition(), t);
			}
		}

		void AddNode(Node* node) 
		{
			nodes.push_back(node);
		}

		void DeleteNode(Node* node) 
		{
			nodes.erase(std::remove(nodes.begin(), nodes.end(), node), nodes.end());
		}

		void Update(double deltaTime)
		{
			timer += deltaTime/1000 * speedScalar;
		}

		void Save(int selectedFile)
		{
			std::stringstream ss;
			ss << "Splines/spline" << selectedFile << ".txt";
			ofstream outfile (ss.str());
		
			if (outfile.is_open())
			{
				for(int i = 0; i < nodes.size(); i++)
				{
					outfile << nodes.at(i)->GetPosition().x << "\n";
					outfile << nodes.at(i)->GetPosition().y << "\n";
					outfile << nodes.at(i)->GetPosition().z;

					if(i != nodes.size()-1)
						outfile << "\n";
				}

				outfile.close();
			}
		}

		void Load(int selectedFile)
		{
			//nodes.erase(nodes.begin(), nodes.begin() + nodes.size());
			//nodes.clear();
			//nodes.erase( nodes.begin(), nodes.end() );

			ifstream infile;

			std::stringstream ss;
			ss << "Splines/spline" << selectedFile << ".txt";
			infile.open (ss.str(), ifstream::in);

			while (infile.good()) 
			{   
				glm::vec3 v = glm::vec3();
				for(int i = 0; i < 3; i++)
				{
					string s;
					getline(infile, s);
					v[i] = std::stoi(s);
				}

				AddNode(new Node(v));
			}
          
			infile.close();
		}
};
