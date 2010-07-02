#include <iostream>
#include "ObjectList.h"

#include <vector>
using namespace std;


int main()
{
	while (true)
	{

		std::cout << "Testing Object List" << std::endl;
		
		ObjectList* testList = new ObjectList();

		testList->addBox(100, 110,2,-2,1);
		testList->addBox(400, 440,1,-3,-1);
		vector<Box*> boxOutput = testList->getBoxList();
		
		std::cout << boxOutput.size() << std::endl;
		std::cout << boxOutput[1]->getX() << std::endl;
		std::cout << boxOutput[0]->getX() << std::endl;

		delete testList;
		//boxOutput = testList->getBoxList(); //this causes crash as it should
		std::cout << boxOutput[0]->getX() << std::endl; // why is this?
		
		int input;
		std::cin >> input;
		break;
	}
	return 0;
}