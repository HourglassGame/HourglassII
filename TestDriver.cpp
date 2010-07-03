
#ifndef INC_OBJECTLIST
#define INC_OBJECTLIST
#include "ObjectList.h"
#endif // INC_OBJECTLIST

#include "PhysicsEngine.h"

#include <iostream>

#include <boost/smart_ptr.hpp>
#include <vector>
using namespace std;



int main()
{
	while (true)
	{

		std::cout << "Testing Object List" << std::endl;
		
		boost::shared_ptr<ObjectList> testList(new ObjectList());
		boost::shared_ptr<ObjectList> testList2(new ObjectList());
		boost::shared_ptr<ObjectList> testList3(new ObjectList());
		boost::shared_ptr<ObjectList> testList4(new ObjectList());
		boost::shared_ptr<ObjectList> testList5(new ObjectList());

		testList->addBox(100, 110,2,-2,1);
		testList->addBox(300, 440,1,-3,-1);

		testList2->addBox(200, 110,2,-2,1);
		testList2->addBox(400, 440,1,-3,-1);

		testList4->addBox(500, 440,1,-3,-1);

		vector<boost::shared_ptr<Box>> boxOutput = testList->getBoxList();
		
		std::cout << boxOutput.size() << std::endl;
		std::cout << boxOutput[1]->getX() << std::endl;
		std::cout << boxOutput[0]->getX() << std::endl;
		
		if (testList->equals(testList2))
		{
			std::cout << "testList equal to testList2" << std::endl;
		}

		//testList3 = testList->operator + (testList2); // causes crash
		
		testList3->add(testList4,0);
		testList3->add(testList,0);
		testList3->add(testList2,0);

		std::cout << std::endl << testList3->getBoxList().size() << std::endl;

		for (unsigned int i = 0; i < testList3->getBoxList().size(); ++i)
		{
			std::cout << testList3->getBoxList()[i]->getX() << std::endl;
		}

		testList5->add(testList3,0);
		testList5->sortElements();
		std::cout << std::endl;
		for (unsigned int i = 0; i < testList5->getBoxList().size(); ++i)
		{
			std::cout << testList5->getBoxList()[i]->getX() << std::endl;
		}

		int input;
		std::cin >> input;
		break;
	}
	return 0;
}