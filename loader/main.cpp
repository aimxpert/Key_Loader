#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <ctime>
#include <windows.h>
#include <sstream>
#include <thread>

using namespace std;

struct KeyInfo {
    char key[20];
    char hwid[50];
    char expiry[11];
};

// Hardcoded keys with durations
std::map<std::string, int> validKeys = {
    {"123", 1},   // keyname / 1-day key
    {"1234", 7},   // keyname / 7-day key
    {"12345", 30}   // keyname / 1-month key
};

// File to store key and HWID bindings
const char* HWID_FILE = "C:\\ProgramData\\ssh\\hwid_data.txt";

// Get HWID (without std::string)
char* GetHWID() {
    static HW_PROFILE_INFO hwProfile;
    if (GetCurrentHwProfile(&hwProfile)) {
        return hwProfile.szHwProfileGuid;
    }
    return (char*)"UNKNOWN_HWID";
}

void ConvertTimeToString(time_t expiry, char* buffer) {
    struct tm* timeinfo = localtime(&expiry);
    strftime(buffer, 11, "%d-%m-%Y", timeinfo);
}

void SaveKeyToFile(const char* key, const char* hwid, time_t expiry) {
    char expiryDate[11];
    ConvertTimeToString(expiry, expiryDate);

    std::ofstream file(HWID_FILE, std::ios::app);
    if (file.is_open()) {
        file << key << "," << hwid << "," << expiryDate << "\n";
        file.close();
    }
}

std::map<std::string, KeyInfo> LoadStoredKeys() {
    std::map<std::string, KeyInfo> storedKeys;
    std::ifstream file(HWID_FILE);
    if (!file.is_open()) return storedKeys;

    std::string line;
    while (getline(file, line)) {
        std::istringstream iss(line);
        std::string key, hwid, expiry;
        if (getline(iss, key, ',') && getline(iss, hwid, ',') && getline(iss, expiry, ',')) {
            KeyInfo info;
            strcpy(info.key, key.c_str());
            strcpy(info.hwid, hwid.c_str());
            strcpy(info.expiry, expiry.c_str());
            storedKeys[key] = info;
        }
    }
    file.close();
    return storedKeys;
}

time_t ConvertStringToTime(const char* date) {
    struct tm timeinfo = {};
    sscanf(date, "%d-%d-%d", &timeinfo.tm_mday, &timeinfo.tm_mon, &timeinfo.tm_year);
    timeinfo.tm_mon -= 1;  // Month is 0-based in `tm`
    timeinfo.tm_year -= 1900; // Year is since 1900
    return mktime(&timeinfo);
}

bool checkKey(const char* enteredKey) {
    std::map<std::string, KeyInfo> storedKeys = LoadStoredKeys();
    const char* userHWID = GetHWID();

    if (storedKeys.find(enteredKey) != storedKeys.end()) {
        KeyInfo keyInfo = storedKeys[enteredKey];

        if (strcmp(keyInfo.hwid, userHWID) != 0) {
            std::cout << "This key is locked to another device!\n";
            return false;
        }

        time_t expiryTime = ConvertStringToTime(keyInfo.expiry);
        time_t now = time(nullptr);
        if (expiryTime < now) {
            std::cout << "Your key has expired!\n";
            return false;
        }

        std::cout << "Key authenticated successfully! Welcome.\n";
        return true;
    }

    if (validKeys.find(enteredKey) != validKeys.end()) {
        int days = validKeys[enteredKey];
        time_t expiry = time(nullptr) + (days * 86400);

        SaveKeyToFile(enteredKey, userHWID, expiry);
        std::cout << "Key activated successfully! Locked to your HWID.\n";
        return true;
    }

    std::cout << "Invalid key!\n";
    return false;
}

void showMenu() {
    cout << "Welcome to the menu!\n";
    cout << "1. Option 1\n";
    cout << "2. Option 2\n";
    cout << "3. Option 3\n";
    cout << "Enter your choice: ";
}

std::string RandomString(const int len)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    std::string tmp_s;
    tmp_s.reserve(len);

    for (int i = 0; i < len; ++i) {
        tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    return tmp_s;
}

void NameChanger()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    std::string NAME = (std::string)(RandomString(15));
    SetConsoleTitleA(NAME.c_str());

}

DWORD ChangeName(LPVOID in)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    while (true)
    {
        NameChanger();
    }
}

int main() {

    CreateThread(NULL, NULL, ChangeName, NULL, NULL, NULL);

    char enteredKey[20];
    cout << "Enter your key: ";
    cin >> enteredKey;

    if (checkKey(enteredKey)) {
        cout << "Access granted!\n";

        system("cls");

        showMenu();

        int choice;
        cin >> choice;

        switch (choice) {
        case 1:
            cout << "You chose Option 1\n";
            break;
        case 2:
            cout << "You chose Option 2\n";
            break;
        case 3:
            cout << "You chose Option 3\n";
            break;
        default:
            cout << "Invalid option\n";
            break;
        }
    }
    else {
        cout << "Access denied!\n";
    }

    system("pause");
    return 0;
}