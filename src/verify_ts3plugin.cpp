#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>

using namespace std;
namespace fs = filesystem;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Usage: verify_ts3plugin <file.ts3_plugin>" << endl;
        return 1;
    }
    
    string pluginFile = argv[1];
    
    if (!fs::exists(pluginFile)) {
        cerr << "Error: File does not exist: " << pluginFile << endl;
        return 1;
    }
    
    cout << "Verifying: " << pluginFile << endl;
    cout << "File size: " << fs::file_size(pluginFile) << " bytes" << endl;
    
    // Check if it's a valid ZIP by reading the file signature
    ifstream file(pluginFile, ios::binary);
    if (!file) {
        cerr << "Error: Could not open file" << endl;
        return 1;
    }
    
    // Read first 4 bytes (ZIP signature: 50 4B 03 04)
    unsigned char signature[4];
    file.read(reinterpret_cast<char*>(signature), 4);
    
    cout << "File signature: ";
    for (int i = 0; i < 4; i++) {
        printf("%02X ", signature[i]);
    }
    cout << endl;
    
    // Check for ZIP signature
    if (signature[0] == 0x50 && signature[1] == 0x4B && 
        signature[2] == 0x03 && signature[3] == 0x04) {
        cout << "✓ Valid ZIP signature detected" << endl;
    } else {
        cout << "✗ Invalid ZIP signature - file may be corrupted!" << endl;
        cout << "  Expected: 50 4B 03 04" << endl;
        return 1;
    }
    
    file.close();
    
    // Try to extract and verify contents
    string tempDir = fs::temp_directory_path().string() + "/ts3_verify_temp";
    
    // Clean up if exists
    if (fs::exists(tempDir)) {
        fs::remove_all(tempDir);
    }
    
    fs::create_directories(tempDir);
    
#ifdef _WIN32
    string command = "powershell -NoProfile -Command \"Expand-Archive -LiteralPath '" + 
                         fs::absolute(pluginFile).string() + "' -DestinationPath '" + 
                         tempDir + "' -Force\"";
#else
    string command = "unzip -q \"" + fs::absolute(pluginFile).string() + 
                         "\" -d \"" + tempDir + "\"";
#endif
    
    cout << "\nExtracting to verify contents..." << endl;
    int result = system(command.c_str());
    
    if (result != 0) {
        cerr << "✗ Failed to extract archive - file may be corrupted!" << endl;
        fs::remove_all(tempDir);
        return 1;
    }
    
    // Check for required files
    bool hasPackageIni = fs::exists(tempDir + "/package.ini");
    bool hasPluginsDir = fs::exists(tempDir + "/plugins") && fs::is_directory(tempDir + "/plugins");
    
    cout << "\nContents check:" << endl;
    cout << (hasPackageIni ? "✓" : "✗") << " package.ini " 
              << (hasPackageIni ? "(" + to_string(fs::file_size(tempDir + "/package.ini")) + " bytes)" : "(missing)") 
              << endl;
    cout << (hasPluginsDir ? "✓" : "✗") << " plugins/ directory" << endl;
    
    if (hasPluginsDir) {
        int dllCount = 0;
        for (const auto& entry : fs::directory_iterator(tempDir + "/plugins")) {
            if (entry.path().extension() == ".dll") {
                dllCount++;
                cout << "  ✓ " << entry.path().filename().string() 
                         << " (" << fs::file_size(entry.path()) << " bytes)" << endl;
            }
        }
        if (dllCount == 0) {
            cout << "  ✗ No DLL files found in plugins directory!" << endl;
        }
    }
    
    if (hasPackageIni) {
        cout << "\npackage.ini contents:" << endl;
        cout << "---" << endl;
        ifstream iniFile(tempDir + "/package.ini");
        string line;
        while (getline(iniFile, line)) {
            cout << line << endl;
        }
        cout << "---" << endl;
    }
    
    // Clean up
    fs::remove_all(tempDir);
    
    cout << "\nVerification complete!" << endl;
    
    return 0;
}
