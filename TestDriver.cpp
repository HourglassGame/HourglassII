
#ifndef INC_OBJECTLIST
#define INC_OBJECTLIST
#include "ObjectList.h"
#endif // INC_OBJECTLIST

#include "TimeEngine.h"

#ifndef INC_INPUTLIST
#define INC_INPUTLIST
#include "InputList.h"
#endif // INC_INPUTLIST

#ifndef INC_PHYSICSENGINE
#define INC_PHYSICSENGINE
#include "PhysicsEngine.h"
#endif // INC_PHYSICSENGINE

#include <iostream>

#include <boost/smart_ptr.hpp>
#include <vector>
using namespace std;


int main()
{

	std::cout << "Testing Object List" << std::endl;
	
	boost::shared_ptr<ObjectList> testList(new ObjectList());
	boost::shared_ptr<ObjectList> testList2(new ObjectList());
	boost::shared_ptr<ObjectList> testList3(new ObjectList());
	boost::shared_ptr<ObjectList> testList4(new ObjectList());
	boost::shared_ptr<ObjectList> testList5(new ObjectList());

	testList->addBox(100, 110,2,-2,1,1);
	testList->addBox(300, 440,1,-3,1,-1);

	testList2->addBox(200, 110,2,-2,1,1);
	testList2->addBox(400, 440,1,-3,1,-1);

	testList4->addBox(500, 440,1,-3,1,-1);

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

	std::cout << std::endl << "**Testing Time Engine**" << std::endl;
	
	vector<vector<bool>> wallmap;
	for (int i = 0; i < 3; ++i)
	{
		wallmap.push_back(vector<bool>());
	}

	for (int i = 0; i < 3; ++i)
	{
		wallmap[0].push_back(true);
		wallmap[2].push_back(true);
	}

	wallmap[1].push_back(true);
	wallmap[1].push_back(false);
	wallmap[1].push_back(true);

	boost::shared_ptr<TimeEngine> testEngine(new TimeEngine(10,wallmap,3500,100));

	boost::shared_ptr<ObjectList> testEngineObjectList(new ObjectList());
	testEngineObjectList->addBox(5000, 3400, 0 ,0, 1000, 1);
	testEngineObjectList->addGuy(5500, 5400, 0, 0, 500, 1000, 1, 0, 0, 0);

	testEngine->checkConstistencyAndPropagateLevel(testEngineObjectList,0);

	for (unsigned int i = 0; i < 10; ++i)
	{
		boost::shared_ptr<ObjectList> arrivalAtI = testEngine->getLastArrival(i);
		std::cout << std::endl;
		for (unsigned int j = 0; j < arrivalAtI->getGuyList().size(); ++j)
		{
			std::cout << "Guy Y at Frame " << i << " " << arrivalAtI->getGuyList()[j]->getY() << std::endl;
		}
		for (unsigned int j = 0; j < arrivalAtI->getBoxList().size(); ++j)
		{
			std::cout << "Box Y at Frame " << i << " " << arrivalAtI->getBoxList()[j]->getY() << std::endl;
		}
	}

	std::cout << std::endl << "**'Gameplay' Test**" << std::endl;

	vector<boost::shared_ptr<ObjectList>> engineReturn;
	engineReturn = testEngine->getNextPlayerFrame(boost::shared_ptr<InputList>(new InputList(true,false,false,false,false,false,false,0,0)));

	std::cout << std::endl;
	for (unsigned int i = 0; i < engineReturn[0]->getGuyList().size(); ++i)
	{
		std::cout << "Guy Position " << engineReturn[0]->getGuyList()[i]->getX() << "," << engineReturn[0]->getGuyList()[i]->getY() << std::endl;
	}
	for (unsigned int i = 0; i < engineReturn[0]->getBoxList().size(); ++i)
	{
		std::cout << "Box Position " << engineReturn[0]->getBoxList()[i]->getX() << "," << engineReturn[0]->getBoxList()[i]->getY() << std::endl;
	}

	engineReturn = testEngine->getNextPlayerFrame(boost::shared_ptr<InputList>(new InputList(true,false,false,false,false,false,false,0,0)));
	std::cout << std::endl;
	for (unsigned int i = 0; i < engineReturn[0]->getGuyList().size(); ++i)
	{
		std::cout << "Guy Position " << engineReturn[0]->getGuyList()[i]->getX() << "," << engineReturn[0]->getGuyList()[i]->getY() << std::endl;
	}
	for (unsigned int i = 0; i < engineReturn[0]->getBoxList().size(); ++i)
	{
		std::cout << "Box Position " << engineReturn[0]->getBoxList()[i]->getX() << "," << engineReturn[0]->getBoxList()[i]->getY() << std::endl;
	}

	engineReturn = testEngine->getNextPlayerFrame(boost::shared_ptr<InputList>(new InputList(true,false,false,false,false,false,false,0,0)));
	std::cout << std::endl;
	for (unsigned int i = 0; i < engineReturn[0]->getGuyList().size(); ++i)
	{
		std::cout << "Guy Position " << engineReturn[0]->getGuyList()[i]->getX() << "," << engineReturn[0]->getGuyList()[i]->getY() << std::endl;
	}
	for (unsigned int i = 0; i < engineReturn[0]->getBoxList().size(); ++i)
	{
		std::cout << "Box Position " << engineReturn[0]->getBoxList()[i]->getX() << "," << engineReturn[0]->getBoxList()[i]->getY() << std::endl;
	}

	engineReturn = testEngine->getNextPlayerFrame(boost::shared_ptr<InputList>(new InputList(true,false,false,false,false,false,false,0,0)));
	std::cout << std::endl;
	for (unsigned int i = 0; i < engineReturn[0]->getGuyList().size(); ++i)
	{
		std::cout << "Guy Position " << engineReturn[0]->getGuyList()[i]->getX() << "," << engineReturn[0]->getGuyList()[i]->getY() << std::endl;
	}
	for (unsigned int i = 0; i < engineReturn[0]->getBoxList().size(); ++i)
	{
		std::cout << "Box Position " << engineReturn[0]->getBoxList()[i]->getX() << "," << engineReturn[0]->getBoxList()[i]->getY() << std::endl;
	}

	engineReturn = testEngine->getNextPlayerFrame(boost::shared_ptr<InputList>(new InputList(false,true,false,false,false,false,false,0,0)));
	std::cout << std::endl;
	for (unsigned int i = 0; i < engineReturn[0]->getGuyList().size(); ++i)
	{
		std::cout << "Guy Position " << engineReturn[0]->getGuyList()[i]->getX() << "," << engineReturn[0]->getGuyList()[i]->getY() << std::endl;
	}
	for (unsigned int i = 0; i < engineReturn[0]->getBoxList().size(); ++i)
	{
		std::cout << "Box Position " << engineReturn[0]->getBoxList()[i]->getX() << "," << engineReturn[0]->getBoxList()[i]->getY() << std::endl;
	}

	engineReturn = testEngine->getNextPlayerFrame(boost::shared_ptr<InputList>(new InputList(false,false,true,false,false,false,false,0,0)));
	std::cout << std::endl;
	for (unsigned int i = 0; i < engineReturn[0]->getGuyList().size(); ++i)
	{
		std::cout << "Guy Position " << engineReturn[0]->getGuyList()[i]->getX() << "," << engineReturn[0]->getGuyList()[i]->getY() << std::endl;
	}
	for (unsigned int i = 0; i < engineReturn[0]->getBoxList().size(); ++i)
	{
		std::cout << "Box Position " << engineReturn[0]->getBoxList()[i]->getX() << "," << engineReturn[0]->getBoxList()[i]->getY() << std::endl;
	}

	engineReturn = testEngine->getNextPlayerFrame(boost::shared_ptr<InputList>(new InputList(false,true,false,false,false,false,false,0,0)));
	std::cout << std::endl;
	for (unsigned int i = 0; i < engineReturn[0]->getGuyList().size(); ++i)
	{
		std::cout << "Guy Position " << engineReturn[0]->getGuyList()[i]->getX() << "," << engineReturn[0]->getGuyList()[i]->getY() << std::endl;
	}
	for (unsigned int i = 0; i < engineReturn[0]->getBoxList().size(); ++i)
	{
		std::cout << "Box Position " << engineReturn[0]->getBoxList()[i]->getX() << "," << engineReturn[0]->getBoxList()[i]->getY() << std::endl;
	}

	engineReturn = testEngine->getNextPlayerFrame(boost::shared_ptr<InputList>(new InputList(false,true,false,false,false,false,false,0,0)));
	std::cout << std::endl;
	for (unsigned int i = 0; i < engineReturn[0]->getGuyList().size(); ++i)
	{
		std::cout << "Guy Position " << engineReturn[0]->getGuyList()[i]->getX() << "," << engineReturn[0]->getGuyList()[i]->getY() << std::endl;
	}
	for (unsigned int i = 0; i < engineReturn[0]->getBoxList().size(); ++i)
	{
		std::cout << "Box Position " << engineReturn[0]->getBoxList()[i]->getX() << "," << engineReturn[0]->getBoxList()[i]->getY() << std::endl;
	}

	engineReturn = testEngine->getNextPlayerFrame(boost::shared_ptr<InputList>(new InputList(false,true,false,false,false,false,false,0,0)));
	std::cout << std::endl;
	for (unsigned int i = 0; i < engineReturn[0]->getGuyList().size(); ++i)
	{
		std::cout << "Guy Position " << engineReturn[0]->getGuyList()[i]->getX() << "," << engineReturn[0]->getGuyList()[i]->getY() << std::endl;
	}
	for (unsigned int i = 0; i < engineReturn[0]->getBoxList().size(); ++i)
	{
		std::cout << "Box Position " << engineReturn[0]->getBoxList()[i]->getX() << "," << engineReturn[0]->getBoxList()[i]->getY() << std::endl;
	}

	int input;
	std::cin >> input;

	return 0;
}
