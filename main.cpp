#include<iostream>
#include<string>
#include"parser.h"
#include"simulation.h"
using namespace std;

bool readInt(const string& prompt,int& value){
    string line;
    while(true){
        cout<<prompt;
        if(!getline(cin,line)){
            return false;
        }
        if(line.empty()){
            continue;
        }
        bool ok=true;
        int start=0;
        if(line[0]=='-'){
            start=1;
        }
        if(start>=(int)line.size()){
            ok=false;
        }
        for(int i=start;i<(int)line.size()&&ok;i++){
            if(line[i]<'0'||line[i]>'9'){
                ok=false;
            }
        }
        if(ok&&line.size()<10){
            value=stoi(line);
            return true;
        }
        cout<<"Enter a whole number.\n";
    }
}

bool loadScenario(const string& path,bool echo,Parser*& parser,Simulation*& sim){
    delete sim;
    sim=nullptr;
    delete parser;
    parser=new Parser();
    if(!parser->load(path)){
        cout<<"Could not load "<<path<<": "<<parser->getError()<<"\n";
        delete parser;
        parser=nullptr;
        return false;
    }
    sim=new Simulation(parser);
    if(!sim->init(echo)){
        cout<<"Could not start the simulation for "<<path<<"\n";
        delete sim;
        sim=nullptr;
        delete parser;
        parser=nullptr;
        return false;
    }
    cout<<"Loaded scenario "<<parser->getScenarioName()<<": "<<parser->getLocationCount()<<" locations, "<<parser->getConnectionCount()<<" connections, "<<parser->getRobotCount()<<" robots, "<<parser->getRequestCount()<<" requests, "<<parser->getEventCount()<<" events\n";
    return true;
}

void finishRun(Simulation* sim){
    sim->printStats(cout);
    string path;
    if(sim->saveStats(path)){
        cout<<"Statistics saved to "<<path<<"\n";
    }
}

void printMenu(Simulation* sim){
    cout<<"\n=== AI Rescue Robots ===  step "<<sim->getStep();
    if(sim->isFinished()){
        cout<<" (finished)";
    }
    cout<<"\n";
    cout<<"1. Run one step\n";
    cout<<"2. Run several steps\n";
    cout<<"3. Run to the end\n";
    cout<<"4. Show robots\n";
    cout<<"5. Show requests\n";
    cout<<"6. Show environment\n";
    cout<<"7. Show statistics\n";
    cout<<"8. Save statistics to file\n";
    cout<<"9. Load another scenario\n";
    cout<<"0. Exit\n";
}

int main(int argc,char** argv){
    string path="scenario_provided.txt";
    bool batch=false;
    for(int i=1;i<argc;i++){
        string a=argv[i];
        if(a=="--run"){
            batch=true;
        }
        else{
            path=a;
        }
    }
    Parser* parser=nullptr;
    Simulation* sim=nullptr;
    if(batch){
        if(!loadScenario(path,false,parser,sim)){
            return 1;
        }
        sim->runAll();
        finishRun(sim);
        delete sim;
        delete parser;
        return 0;
    }
    cout<<"AI Rescue Robots - disaster response simulator\n";
    if(argc<2){
        cout<<"Scenario file (press Enter for "<<path<<"): ";
        string line;
        if(getline(cin,line)&&!line.empty()){
            path=line;
        }
    }
    int echoChoice=0;
    if(!readInt("Print log lines to the screen while running? (1 = yes, 0 = no): ",echoChoice)){
        return 0;
    }
    bool echo=(echoChoice==1);
    while(!loadScenario(path,echo,parser,sim)){
        cout<<"Scenario file (or type exit): ";
        if(!getline(cin,path)||path=="exit"){
            return 1;
        }
    }
    while(true){
        printMenu(sim);
        int choice=-1;
        if(!readInt("Choose: ",choice)){
            break;
        }
        if(choice==0){
            break;
        }
        else if(choice==1){
            if(!sim->stepOnce()){
                cout<<"The simulation has already finished.\n";
            }
            else if(sim->isFinished()){
                cout<<"The simulation finished at step "<<sim->getStep()<<".\n";
                finishRun(sim);
            }
        }
        else if(choice==2){
            int n=0;
            if(!readInt("How many steps? ",n)){
                break;
            }
            if(n<1){
                cout<<"Enter at least 1.\n";
                continue;
            }
            int done=0;
            while(done<n&&sim->stepOnce()){
                done++;
            }
            cout<<"Ran "<<done<<" step(s). Now at step "<<sim->getStep()<<".\n";
            if(sim->isFinished()){
                cout<<"The simulation finished.\n";
                finishRun(sim);
            }
        }
        else if(choice==3){
            if(sim->isFinished()){
                cout<<"The simulation has already finished.\n";
            }
            else{
                sim->runAll();
                cout<<"The simulation finished at step "<<sim->getStep()<<".\n";
                finishRun(sim);
            }
        }
        else if(choice==4){
            sim->printRobots(cout);
        }
        else if(choice==5){
            sim->printRequests(cout);
        }
        else if(choice==6){
            sim->printEnvironment(cout);
        }
        else if(choice==7){
            sim->printStats(cout);
        }
        else if(choice==8){
            string out;
            if(sim->saveStats(out)){
                cout<<"Statistics saved to "<<out<<"\n";
            }
            else{
                cout<<"Could not save the statistics file.\n";
            }
        }
        else if(choice==9){
            cout<<"Scenario file: ";
            string newPath;
            if(!getline(cin,newPath)){
                break;
            }
            if(!loadScenario(newPath,echo,parser,sim)){
                cout<<"Reloading "<<path<<" instead.\n";
                if(!loadScenario(path,echo,parser,sim)){
                    break;
                }
            }
            else{
                path=newPath;
            }
        }
        else{
            cout<<"Choose a number from the menu.\n";
        }
    }
    delete sim;
    delete parser;
    cout<<"Goodbye.\n";
    return 0;
}