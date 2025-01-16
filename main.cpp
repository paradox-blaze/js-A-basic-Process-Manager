#include <iostream>
#include <string>
#include <fstream>
#include "json.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <sstream>
#include <signal.h>

using namespace std;
using json = nlohmann::json;

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        cout << "Usage: js <command> [arguments]" << endl;
        return 1;
    }

    string command = argv[1];

    if (command == "add")
    {
        if (argc < 3)
        {
            cout << "Usage: js add <AppName> [AppPath]" << endl;
        }

        string AppName = argv[2];
        string AppPath = argv[3];
        json data;
        ifstream inFile("datastore.json");
        if (!inFile)
        {
            cerr << "Error opening file" << endl;
            return 1;
        }
        inFile >> data;
        data[AppName] = AppPath;
        inFile.close();
        ofstream outFile("datastore.json");
        if (!outFile)
        {
            cerr << "Could not open file datastore.json for writing!" << endl;
        }
        outFile << data.dump(4);
        outFile.close();

        cout << "Added app successfully" << endl;
    }

    else if (command == "start")
    {
        string AppName = argv[2];
        json data;
        ifstream inFile("datastore.json");
        inFile >> data;
        inFile.close();
        pid_t pid = fork();
        string AppPath = data[AppName];
        if (pid < 0)
        {
            cerr << "Could not start the App" << endl;
        }
        else if (pid == 0)
        {
            pid_t sid = setsid();
            if (sid < 0)
            {
                cerr << "Could not create a new session" << endl;
                exit(1);
            }

            freopen("app_output.log", "a", stdout);
            freopen("app_error.log", "a", stderr);

            execlp(AppPath.c_str(), AppName.c_str(), NULL);
            perror("execlp failed");
            exit(1);
        }
        else
        {
            ifstream inFile("pids.json");
            if (!inFile.is_open())
            {
                cerr << "File could not be opened" << endl;
                return 1;
            }
            json pidData;

            inFile >> pidData;

            inFile.close();
            ofstream pidFile("pids.json");

            pidData[AppName] = pid;
            pidFile << pidData.dump(4);
            pidFile.close();
        }
        ifstream pidFile("pids.json");
        json pidData;
        pidFile >> pidData;
        cout << pidData[AppName] << " " << AppName << endl;
    }
    else if (command == "start-all")
    {
        cout << "starting all" << endl;
        ifstream inFile("datastore.json");
        json data;
        inFile >> data;
        inFile.close();
        for (auto &[key, value] : data.items())
        {
            pid_t pid = fork();
            string AppName = key;
            string AppPath = value;
            if (pid < 0)
            {
                cerr << "Error forking process " << key << endl;
                return 1;
            }
            else if (pid == 0)
            {
                pid_t sid = setsid();
                if (sid < 0)
                {
                    cerr << "Session couldn't be created" << endl;
                    return 1;
                }
                freopen("app_error.log", "a", stderr);
                freopen("app_output.log", "a", stdout);
                execlp(AppPath.c_str(), AppName.c_str(), NULL);
                perror("execlp failed");
                exit(1);
            }
            else
            {
                ifstream inFile("pids.json");
                if (!inFile.is_open())
                {
                    cerr << "File could not be opened" << endl;
                    return 1;
                }
                json pidData;

                inFile >> pidData;

                inFile.close();
                ofstream pidFile("pids.json");

                pidData[AppName] = pid;
                pidFile << pidData.dump(4);
                pidFile.close();
            }
            ifstream pidFile("pids.json");
            json pidData;
            pidFile >> pidData;
            cout << pidData[AppName] << " " << AppName << endl;
        }
    }
    else if (command == "stop")
    {
        string AppName = argv[2];
        json data;
        ifstream inFile("pids.json");
        inFile >> data;
        if (data.contains(AppName))
        {
            pid_t pid = (pid_t)data[AppName];
            kill(pid, 9);
            if (data.contains(AppName))
            {
                data.erase(AppName);
            }
            else
            {
                cout << AppName << "Not running currently" << endl;
            }
            inFile.close();
            ofstream outFile("pids.json");
            outFile << data.dump(4) << endl;
            outFile.close();
        }
        else
        {
            cout << "Requested process isn't running right now, use -help to see if you're using the right command" << endl;
        }
    }
    else if (command == "stop-all")
    {
        json data;
        ifstream inFile("pids.json");
        inFile >> data;
        for (auto &[key, value] : data.items())
        {
            string AppName = key;
            pid_t pid = (pid_t)value;
            kill(pid, 9);
        }
        inFile.close();
        data = {};
        ofstream outFile("pids.json");
        outFile << data.dump(4) << endl;
    }
    else if (command == "remove")
    {
        string AppName = argv[2];
        ifstream inFile("datastore.json");
        json data;
        inFile >> data;
        if (data.contains(AppName))
        {
            data.erase(AppName);
        }
        inFile.close();
        ofstream outFile("datastore.json");
        outFile << data.dump(4) << endl;
        outFile.close();

        cout << "removed from the inventory" << endl;
    }
    else if (command == "show")
    {
        json data;
        ifstream inFile("datastore.json");
        ifstream inPids("pids.json");
        json pids;
        inPids >> pids;
        if (!inFile)
        {
            cerr << "Error opening file" << endl;
            return 1;
        }
        inFile >> data;
        for (auto &[key, value] : data.items())
        {
            if (pids.contains(key))
            {
                cout << "running " << pids[key] << "\t" << key << endl;
            }
            else
            {
                cout << "not running " << "\t" << key << endl;
            }
        }
    }
    else if (command == "-help")
    {
        cout << "add: creates a new entry in the inventory" << endl;
        cout << "start: starts a particular app from the inventory" << endl;
        cout << "start-all: runs all the apps in the inventory" << endl;
        cout << "show: shows running processes" << endl;
        cout << "stop: stops a particular app from the inventory" << endl;
        cout << "stop-all: stops all the apps from the inventory" << endl;
        cout << "remove: removes a particular app from the inventory" << endl;
    }
    else
    {
        cout << "Wrong command use -help to check for commands" << endl;
    }
    return 0;
}