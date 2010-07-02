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
		ObjectList* testList2 = new ObjectList();
		ObjectList* testList4 = new ObjectList();

		testList->addBox(100, 110,2,-2,1);
		testList->addBox(300, 440,1,-3,-1);

		testList2->addBox(200, 110,2,-2,1);
		testList2->addBox(400, 440,1,-3,-1);

		testList4->addBox(500, 440,1,-3,-1);

		vector<Box*> boxOutput = testList->getBoxList();
		
		std::cout << boxOutput.size() << std::endl;
		std::cout << boxOutput[1]->getX() << std::endl;
		std::cout << boxOutput[0]->getX() << std::endl;

		if (testList->operator == (testList2))
		{
			std::cout << "testList equal to testList2" << std::endl;
		}

		ObjectList* testList3 = testList->operator + (testList2);

		testList3->operator += (testList4);

		std::cout << testList3->getBoxList().size() << std::endl;
		for (int i = 0; i < testList3->getBoxList().size(); ++i)
		{
			std::cout << testList3->getBoxList()[i]->getX() << std::endl;
		}

		delete testList;
		delete testList2;
		delete testList3;
		delete testList4;
		//boxOutput = testList->getBoxList(); //this causes crash as it should
		std::cout << boxOutput[0]->getX() << std::endl; // why is this?
		
		int input;
		std::cin >> input;
		break;
	}
	return 0;
}