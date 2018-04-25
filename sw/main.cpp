#include <iostream>
#include <list>
#include <iterator>
#include <vector>

#define BLANK_ID_BASE     1000
#define STORAGE_ID   0
#define MACHINE_BASE 1
#define ROBOT_ID     2000

// Some globals to emulate UART
bool robotToHandlerPut;
bool robotToHandlerGrab;
unsigned int robotToHandlerBlankID;
unsigned int robotToHandlerWhereToPut;
unsigned int robotPos;
unsigned int robotBlank;



class Blank {
public:

    Blank(unsigned int ID) {
        blankID = ID;
        whereIs = STORAGE_ID;
        nextMachine = STORAGE_ID;
        blankProcessed = false;
    }


    unsigned int PopNextMachine() {
        if(recipe.empty()) {
            nextMachine = STORAGE_ID;
            blankProcessed = true;
        }
        else {
            nextMachine = recipe.front();
            recipe.erase(recipe.begin());
        }

        return nextMachine;
    }


    unsigned int GetNextMachine() {
        return nextMachine;
    }



    void Move (unsigned int placeToMove) {
        // TODO some fool-proofing checks
        whereIs = placeToMove;
    }

    unsigned int GetID () {
        return blankID;
    }

    unsigned int GetPosition() {
        return whereIs;
    }

    void FillListFromVector(std::vector<unsigned int> &vectorIn) {
        recipe = vectorIn;
        PopNextMachine();
    }


    bool GetProcessedFlag(){
        return blankProcessed;
    }

private:
    std::vector <unsigned int> recipe;
    unsigned int whereIs;
    unsigned int blankID;
    unsigned int nextMachine;
    bool blankProcessed;

} ;


class Machine {
public:


    Machine(unsigned int ID, unsigned int BusyTime){
        machineID = ID;
        machineBusyTime = BusyTime;
        emptyFlag = true;
        readyFlag = false;
        blankID = 0;
        tickCounter = 0;
    }

    void ProcessTick() {
        if(!emptyFlag && !readyFlag) {
            if(tickCounter == machineBusyTime)
                readyFlag = true;

            tickCounter ++;
        }
    }

    void InsertBlank(Blank & BlankToInsert) {

        if(!emptyFlag) {
            std::cout << "Error! Machine " << machineID << " in not empty!" << std::endl;
            return;
        }

        if(BlankToInsert.GetNextMachine() != machineID) {
            std::cout << "Error! Wrong machine " << machineID << std::endl;
            return;
        }

        blankID = BlankToInsert.GetID();

        BlankToInsert.Move(machineID);

        tickCounter = 0;
        emptyFlag = false;
        readyFlag = false;

        BlankToInsert.PopNextMachine();

    }

    void RemoveBlank(Blank & BlankToRemove) {

        //std::cout << "Removing " << BlankToRemove.GetID() << "from " << machineID << std::endl;

        if(emptyFlag) {
            std::cout << "Error! Machine " << machineID << " is already empty!" << std::endl;
            return;
        }

        if(!readyFlag) {
            std::cout << "Error! Machine " << machineID << " is busy!" << std::endl;
            return;
        }

        if(blankID != BlankToRemove.GetID()) {
            std::cout << "Error! Trying to remove wrong blank from machine " << machineID << std::endl;
            return;
        }



        blankID = 0;
        BlankToRemove.Move(ROBOT_ID);


        readyFlag = false;
        emptyFlag = true;

    }

    bool GetReadyFlag() {
        return readyFlag;
    }

    bool GetEmptyFlag() {
        return emptyFlag;
    }

    unsigned int GetBlankID() {
        return blankID;
    }

    unsigned int GetMachineID() {
        return machineID;
    }

private:

    unsigned int machineID;
    unsigned int machineBusyTime;
    bool emptyFlag;
    bool readyFlag;
    unsigned int blankID;
    unsigned int tickCounter;
};


class Storage {
public:

    Storage(unsigned int ID, unsigned int Slots, unsigned int Filling) {
        storageID = ID;
        storageFilling = Filling;
        storageSlots = Slots;
    }

    void InsertBlank(Blank & blankToInsert) {

     /*   if(blankToInsert.GetPosition() == storageID) {
            std::cout << "Error! Blank " << blankToInsert.GetID() << " is already in storage!" << std::endl;
            return;
        } */


        if(storageFilling >= storageSlots) {
            std::cout << "Error! Storage is full!" << std::endl;
            return;
        }

        blankToInsert.Move(storageID);
        storageFilling++;
    }

    void RemoveBlank(Blank & blankToRemove, unsigned int robotID) {

        if(blankToRemove.GetPosition() != storageID) {
            std::cout << "Error! Blank " << blankToRemove.GetID() << " is not inside storage!" << std::endl;
            return;
        }


        if(storageFilling <= 0) {
            std::cout << "Error! Storage is empty!" << std::endl;
            return;
        }

        blankToRemove.Move(robotID);
        storageFilling--;
    }

    unsigned int GetFilling (){
        return storageFilling;
    }

private:
    unsigned int storageFilling;
    unsigned int storageID;
    unsigned int storageSlots;
};


// MESSAGE FOR ME
// Please remember that robot should communicate with ProductionLineHandler ONLY
//

class Robot {
public:
    Robot(unsigned int ID) {
        robotID = ID;
        robotPosition = STORAGE_ID;
        blankID = 0;
        robotBlank = 0;
        robotEmpty = true;
    }

    void AddTask(unsigned int destinationA, unsigned int destinationB, unsigned int BlankID){
        taskDestinationA.push_back(destinationA);
        taskDestinationB.push_back(destinationB);
        taskBlankID.push_back(BlankID);
    }

    // TODO add some logics for task completion



    void Tick() {



        if(robotEmpty && !taskDestinationA.empty()) {
            if(robotPosition == taskDestinationA[0]) {
                GrabBlank(taskBlankID[0]);
            } else {
                Move(taskDestinationA[0]);
            }
            //return;
        }
        else
        if(!robotEmpty && !taskDestinationA.empty()) {
            if(robotPosition == taskDestinationB[0]) {
                PutBlank(taskBlankID[0]);
                FinishTask();
            } else {
                Move(taskDestinationB[0]);
            }
            //return;
        }

        robotBlank = blankID;
    }


    void GrabBlank(unsigned int inBlankID){
        robotToHandlerGrab = true;
        robotToHandlerBlankID = inBlankID;
        robotEmpty = false;
        blankID = inBlankID;
    }

    void PutBlank(unsigned int outBlankID){
        robotToHandlerPut = true;
        robotToHandlerBlankID = outBlankID;
        robotEmpty = true;
        blankID = 0;

    }

    unsigned int GetPosition() {
        return robotPosition;
    }

    void FinishTask() {
        taskDestinationA.erase(taskDestinationA.begin());
        taskDestinationB.erase(taskDestinationB.begin());
        taskBlankID.erase(taskBlankID.begin());
    }

    void Move(unsigned int destination) {
        if(robotPosition < destination) {
            robotPosition++;
            robotPos = robotPosition;
            return;
        }

        if(robotPosition > destination) {
            robotPosition--;
            robotPos = robotPosition;
            return;
        }
    }


private:
    unsigned int robotID;
    unsigned int robotPosition;
    unsigned int blankID;
    bool robotEmpty;

    // Tasks list
    std::vector<unsigned int> taskDestinationA;
    std::vector<unsigned int> taskDestinationB;
    std::vector<unsigned int> taskBlankID;

};


class ProductionLineHandler {
public:
    ProductionLineHandler() {
        Robot newRobot(ROBOT_ID);
        Robots.push_back(newRobot);
        Storage newStorage(STORAGE_ID, 100, 0);
        Storages.push_back(newStorage);
        newBlanksInTaskQueue = 0;
    }

    void AddBlank(std::vector<unsigned int> recipe){
        Blank newBlank(Blanks.size() + BLANK_ID_BASE);
        newBlank.FillListFromVector(recipe);
        Blanks.push_back(newBlank);
        Storages[0].InsertBlank(Blanks[Blanks.size() - 1]);
        blankProcessed.push_back(false);
        blankInProgress.push_back(false);
    }

    unsigned int FindBlankVectorElement(unsigned int ID) {
        for (unsigned int j = 0; j < Blanks.size(); j++) {
            if (Blanks[j].GetID() == ID)
                return j;
        }
        std::cout << "No Blank with ID " << ID << " found!" << std::endl;
        return 0;
    }

    unsigned int FindMachineVectorElement(unsigned int ID) {
        for (unsigned int j = 0; j < Machines.size(); j++) {
            if (Machines[j].GetMachineID() == ID)
                return j;
        }
        std::cout << "No Machine with ID " << ID << " found!" << std::endl;
        return 0;
    }


    void AddMachine(unsigned int BusyTime) {
        Machine newMachine(Machines.size() + MACHINE_BASE, BusyTime);
        Machines.push_back(newMachine);
        machineLocked.push_back(false);
    }



    void ProcessTasks() {

        // TODO prevent task pipeline halt if all machines are busy and none of blanks is fully processed



        // New blanks
        for (int i = 0; i < Blanks.size(); i++) {
            if ((Blanks[i].GetPosition() == STORAGE_ID) &&
                    (!blankInProgress[i]) &&
                    !machineLocked[FindMachineVectorElement(Blanks[i].GetNextMachine())] &&
                    Machines[FindMachineVectorElement(Blanks[i].GetNextMachine())].GetEmptyFlag()
                    ) {

                machineLocked[FindMachineVectorElement(Blanks[i].GetNextMachine())] = true;
                Robots[0].AddTask(STORAGE_ID, Blanks[i].GetNextMachine(), Blanks[i].GetID());
                //newBlanksInTaskQueue++;
                blankInProgress[i] = true;
                return;
            }
        }



        // Blank inside machine and ready for next stage
        for (int i = 0; i < Machines.size(); i++) {
            if (Machines[i].GetReadyFlag()) {

                unsigned int blankID = Machines[i].GetBlankID();
                unsigned int nextMachineID = Blanks[FindBlankVectorElement(blankID)].GetNextMachine();
                unsigned int blankVectElement = FindBlankVectorElement(blankID);

                if(nextMachineID == STORAGE_ID) {
                    if(!blankProcessed[blankVectElement]) {
                        Robots[0].AddTask(Machines[i].GetMachineID(), nextMachineID, blankID);
                        machineLocked[i] = false;
                        blankProcessed[blankVectElement] = true;
                    }
                    return;
                }

                unsigned int nextMachineVectElement = FindMachineVectorElement(nextMachineID);

                //if(Machines[nextMachineVectElement].GetEmptyFlag()) {
                if(!machineLocked[nextMachineVectElement] && Machines[nextMachineVectElement].GetEmptyFlag()) {
                    Robots[0].AddTask(Machines[i].GetMachineID(), nextMachineID, blankID);
                    machineLocked[nextMachineVectElement] = true;
                    machineLocked[i] = false;
                    return;
                }
            }
        }


/*      // Conflict management
        for (int i = 0; i < Machines.size(); i++) {

            if (!Machines[i].GetReadyFlag())
                continue;

            unsigned int blankv = FindBlankVectorElement(Machines[i].GetBlankID());
            unsigned int nextmachine = Blanks[blankv].GetNextMachine();

            if(nextmachine == STORAGE_ID)
                continue;

            unsigned int nextmachinev = FindMachineVectorElement(nextmachine);

            if (!machineLocked[nextmachinev])
                continue;


            for (int j = 0; j < Blanks.size(); j++) {
                if(Blanks[j].GetNextMachine() == STORAGE_ID)
                    continue;

                unsigned int blankcurrentpos = Blanks[j].GetPosition();


                if(!Blanks[j].GetProcessedFlag() && (blankcurrentpos == STORAGE_ID) &&
                   (Blanks[j].GetNextMachine() == Machines[i].GetMachineID())) {
                    Robots[0].AddTask(Machines[i].GetMachineID(), STORAGE_ID, Machines[i].GetBlankID());
                    blankInProgress[FindBlankVectorElement(Machines[i].GetBlankID())] = false;
                    machineLocked[i] = false;
                    break;
                }

                else
                if(Machines[FindMachineVectorElement(Blanks[j].GetPosition())].GetReadyFlag() &&
                   (Blanks[j].GetNextMachine() == Machines[i].GetMachineID())) {
                    Robots[0].AddTask(Machines[i].GetMachineID(), STORAGE_ID, Machines[i].GetBlankID());
                    blankInProgress[FindBlankVectorElement(Machines[i].GetBlankID())] = false;
                    machineLocked[i] = false;
                    break;
                }
            }
        }
*/


    }

    void ProcessRobot() {

        unsigned int blankID = FindBlankVectorElement(robotToHandlerBlankID);

        if(robotToHandlerGrab) {
            if(Robots[0].GetPosition() != STORAGE_ID) {
                unsigned int machineID = FindMachineVectorElement(Robots[0].GetPosition());
                Machines[machineID].RemoveBlank(Blanks[blankID]); // TODO this doesn't work
            } else {
                Storages[0].RemoveBlank(Blanks[blankID], ROBOT_ID);
            }
            robotToHandlerGrab = false;
        }

        if(robotToHandlerPut) {
            if(Robots[0].GetPosition() != STORAGE_ID) {
                Machines[FindMachineVectorElement(Robots[0].GetPosition())].InsertBlank(Blanks[FindBlankVectorElement(robotToHandlerBlankID)]);
            } else {
                Storages[0].InsertBlank(Blanks[blankID]);
            }
            robotToHandlerPut = false;
        }

    }



    // TODO write global Tick function
    void Tick() {
        for (int i = 0; i < Machines.size(); i++) {
            Machines[i].ProcessTick();
        }

        ProcessTasks();

        Robots[0].Tick();

        ProcessRobot();

    }


    void DrawScreen() {
        std::cout << "\x1B[2J\x1B[H";
        std::cout << "[S]";
        for (int i = 0; i < Machines.size(); i++) {
            std::cout << "[" << Machines[i].GetMachineID()-MACHINE_BASE << "]";
        }
        std::cout << std::endl;


        std::cout << "[" << Storages[0].GetFilling() << "]";
        for (int i = 0; i < Machines.size(); i++) {
            if(Machines[i].GetBlankID() == 0)
                std::cout << "[-]";
            else {
                if (Machines[i].GetReadyFlag())
                    std::cout << "{" << Machines[i].GetBlankID() - BLANK_ID_BASE << "}";
                else
                    std::cout << "[" << Machines[i].GetBlankID() - BLANK_ID_BASE << "]";
            }
        }
        std::cout << std::endl;

        for (int i = 0; i < robotPos; i++) {
            std::cout << "   ";
        }
        std::cout << "[R]";

        std::cout << std::endl;
        for (int i = 0; i < robotPos; i++) {
            std::cout << "   ";
        }

        if(robotBlank == 0)
            std::cout << "[-]";
        else
            std::cout << "[" << robotBlank-BLANK_ID_BASE << "]";

        std::cout << std::endl;
        std::cout << std::endl;
    }

private:

    unsigned int newBlanksInTaskQueue;
    std::vector <Blank> Blanks;
    std::vector <Machine> Machines;
    std::vector <Robot> Robots;
    std::vector <Storage> Storages;
    std::vector <bool> machineLocked;
    std::vector <bool> blankProcessed;
    std::vector <bool> blankInProgress;
};


int main() {

    bool exit = false;

    ProductionLineHandler Line;

    std::vector<unsigned int> recipe;// = {1, 2, 3};
    recipe.push_back(1);
    recipe.push_back(2);
    recipe.push_back(3);
    recipe.push_back(5);
    recipe.push_back(7);
    recipe.push_back(9);

    Line.AddBlank(recipe);
    Line.AddBlank(recipe);
    Line.AddBlank(recipe);

    for(int i = 0; i < 10; i++) {
        Line.AddMachine(10);
    }


    std::cout << std::endl;


    while (true) {
        Line.Tick();
        Line.DrawScreen();

        for(int i = 0; i< 100000000; i++);

    }


    return 0;



}
