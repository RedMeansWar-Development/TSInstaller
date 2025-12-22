#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <cstring>
#include <ctime>
#include "simple_zip.h"

#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "comdlg32.lib")
#endif

using namespace std;
namespace fs = filesystem;

// Structure to hold plugin information
struct PluginInfo {
    string name;
    string type;
    string author;
    string version;
    string platforms;
    string description;
    string dllPath;
    string outputPath;
};

// Function to create package.ini content
string createPackageIni(const PluginInfo& info) {
    string content;
    content += "Name = " + info.name + "\n";
    content += "Type = " + info.type + "\n";
    content += "Author = " + info.author + "\n";
    content += "Version = " + info.version + "\n";
    content += "Platforms = " + info.platforms + "\n";
    content += "Description = \"" + info.description + "\"\n";
    return content;
}

// Function to create ZIP archive using custom ZIP library
bool createZipArchive(const string& tempDir, const string& outputFile) {
    cout << "Creating ZIP archive..." << endl;
    
    SimpleZip zip;
    
    try {
        // Add package.ini
        string packageIniPath = tempDir + "/package.ini";
        if (!zip.addFileFromDisk("package.ini", packageIniPath)) {
            cerr << "Error: Failed to add package.ini to ZIP" << endl;
            return false;
        }
        cout << "  Added: package.ini" << endl;
        
        // Add all files from plugins directory
        string pluginsDir = tempDir + "/plugins";
        if (fs::exists(pluginsDir) && fs::is_directory(pluginsDir)) {
            for (const auto& entry : fs::directory_iterator(pluginsDir)) {
                if (entry.is_regular_file()) {
                    string zipPath = "plugins/" + entry.path().filename().string();
                    if (!zip.addFileFromDisk(zipPath, entry.path().string())) {
                        cerr << "Error: Failed to add " << entry.path().filename() << " to ZIP" << endl;
                        return false;
                    }
                    cout << "  Added: " << zipPath << " (" << fs::file_size(entry.path()) << " bytes)" << endl;
                }
            }
        }
        
        // Write the ZIP file
        if (!zip.write(outputFile)) {
            cerr << "Error: Failed to write ZIP file" << endl;
            return false;
        }
        
        // Verify the ZIP was created
        if (!fs::exists(outputFile)) {
            cerr << "Error: ZIP file was not created" << endl;
            return false;
        }
        
        cout << "ZIP archive created successfully" << endl;
        return true;
        
    } catch (const exception& e) {
        cerr << "Exception during ZIP creation: " << e.what() << endl;
        return false;
    }
}

// Function to create the plugin package
bool createPluginPackage(const PluginInfo& info) {
    try {
        // Create temporary directory
        string tempDir = fs::temp_directory_path().string() + "/ts3_plugin_temp";
        
        // Remove temp dir if it exists
        if (fs::exists(tempDir)) {
            fs::remove_all(tempDir);
        }
        
        // Create directory structure
        fs::create_directories(tempDir);
        fs::create_directories(tempDir + "/plugins");
        
        // Create package.ini
        ofstream iniFile(tempDir + "/package.ini");
        if (!iniFile) {
            cerr << "Error: Could not create package.ini" << endl;
            return false;
        }
        iniFile << createPackageIni(info);
        iniFile.close();
        
        // Verify package.ini was created
        if (!fs::exists(tempDir + "/package.ini")) {
            cerr << "Error: package.ini was not created" << endl;
            return false;
        }
        
        // Copy DLL to plugins folder with binary mode explicitly
        string dllDestination = tempDir + "/plugins/" + fs::path(info.dllPath).filename().string();
        
        // Use explicit binary copy to ensure no corruption
        ifstream src(info.dllPath, ios::binary);
        if (!src) {
            cerr << "Error: Could not open source DLL: " << info.dllPath << endl;
            return false;
        }
        
        ofstream dst(dllDestination, ios::binary);
        if (!dst) {
            cerr << "Error: Could not create destination DLL: " << dllDestination << endl;
            return false;
        }
        
        // Copy the file in binary mode
        dst << src.rdbuf();
        src.close();
        dst.close();
        
        // Verify DLL was copied
        if (!fs::exists(dllDestination)) {
            cerr << "Error: DLL was not copied to plugins folder" << endl;
            return false;
        }
        
        // Verify file sizes match
        auto srcSize = fs::file_size(info.dllPath);
        auto dstSize = fs::file_size(dllDestination);
        
        if (srcSize != dstSize) {
            cerr << "Error: DLL copy size mismatch!" << endl;
            cerr << "  Source: " << srcSize << " bytes" << endl;
            cerr << "  Destination: " << dstSize << " bytes" << endl;
            return false;
        }
        
        cout << "DLL copied successfully: " << srcSize << " bytes" << endl;
        
        cout << "Files prepared successfully." << endl;
        cout << "Structure:" << endl;
        cout << "  - package.ini (" << fs::file_size(tempDir + "/package.ini") << " bytes)" << endl;
        cout << "  - plugins/" << fs::path(info.dllPath).filename().string() 
                  << " (" << fs::file_size(dllDestination) << " bytes)" << endl;
        
        // Prepare output path
        string outputBase = info.outputPath;
        if (outputBase.empty()) {
            outputBase = info.name;
        }
        
        // Remove any existing extensions
        if (outputBase.size() > 11 && outputBase.substr(outputBase.size() - 11) == ".ts3_plugin") {
            outputBase = outputBase.substr(0, outputBase.size() - 11);
        }
        if (outputBase.size() > 4 && outputBase.substr(outputBase.size() - 4) == ".zip") {
            outputBase = outputBase.substr(0, outputBase.size() - 4);
        }
        
        // Create temporary zip file with unique name
        string tempZipPath = fs::absolute(outputBase + "_temp.zip").string();
        
        if (!createZipArchive(tempDir, tempZipPath)) {
            cerr << "Error: Failed to create zip archive" << endl;
            fs::remove_all(tempDir);
            return false;
        }
        
        // Verify ZIP file was created and has content
        if (!fs::exists(tempZipPath)) {
            cerr << "Error: ZIP file was not created" << endl;
            fs::remove_all(tempDir);
            return false;
        }
        
        if (fs::file_size(tempZipPath) < 100) {
            cerr << "Error: ZIP file is too small - may be corrupted" << endl;
            fs::remove(tempZipPath);
            fs::remove_all(tempDir);
            return false;
        }
        
        // Rename to final .ts3_plugin extension
        string finalPath = outputBase + ".ts3_plugin";
        if (fs::exists(finalPath)) {
            fs::remove(finalPath);
        }
        
        try {
            fs::rename(tempZipPath, finalPath);
        } catch (const exception& e) {
            cerr << "Error: Failed to rename file: " << e.what() << endl;
            fs::remove(tempZipPath);
            fs::remove_all(tempDir);
            return false;
        }
        
        // Verify final file exists
        if (!fs::exists(finalPath)) {
            cerr << "Error: Final .ts3_plugin file was not created" << endl;
            fs::remove_all(tempDir);
            return false;
        }
        
        auto finalSize = fs::file_size(finalPath);
        cout << "Final package size: " << finalSize << " bytes" << endl;
        
        // Basic sanity check - package should be reasonably sized
        auto originalDllSize = fs::file_size(info.dllPath);
        if (finalSize < originalDllSize * 0.9) {
            cerr << "Warning: Final package seems too small compared to DLL" << endl;
            cerr << "  DLL: " << originalDllSize << " bytes" << endl;
            cerr << "  Package: " << finalSize << " bytes" << endl;
        } else {
            cout << "✓ Package size looks good" << endl;
        }
        
        // Clean up temp directory
        fs::remove_all(tempDir);
        
        cout << "Successfully created: " << finalPath << endl;
        return true;
        
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return false;
    }
}

// CLI mode function
bool processCLI(int argc, char* argv[]) {
    PluginInfo info;
    info.type = "Plugin"; // Default
    
    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        
        if ((arg == "-n" || arg == "--name") && i + 1 < argc) {
            info.name = argv[++i];
        }
        else if ((arg == "-a" || arg == "--author") && i + 1 < argc) {
            info.author = argv[++i];
        }
        else if ((arg == "-v" || arg == "--version") && i + 1 < argc) {
            info.version = argv[++i];
        }
        else if ((arg == "-p" || arg == "--platforms") && i + 1 < argc) {
            info.platforms = argv[++i];
        }
        else if ((arg == "-d" || arg == "--description") && i + 1 < argc) {
            info.description = argv[++i];
        }
        else if ((arg == "-i" || arg == "--input") && i + 1 < argc) {
            info.dllPath = argv[++i];
        }
        else if ((arg == "-o" || arg == "--output") && i + 1 < argc) {
            info.outputPath = argv[++i];
        }
        else if (arg == "-h" || arg == "--help") {
            cout << "TS3 Plugin Packager\n\n";
            cout << "Usage: ts3_packager [options]\n\n";
            cout << "Options:\n";
            cout << "  -n, --name <name>           Plugin name\n";
            cout << "  -a, --author <author>       Plugin author\n";
            cout << "  -v, --version <version>     Plugin version\n";
            cout << "  -p, --platforms <platforms> Platforms (e.g., 'win32, win64, mac')\n";
            cout << "  -d, --description <desc>    Plugin description\n";
            cout << "  -i, --input <path>          Path to plugin DLL\n";
            cout << "  -o, --output <path>         Output file path (without extension)\n";
            cout << "  -h, --help                  Show this help message\n\n";
            cout << "If no arguments are provided, GUI mode will start.\n";
            return false;
        }
    }
    
    // Validate required fields
    if (info.name.empty() || info.author.empty() || info.version.empty() || 
        info.platforms.empty() || info.description.empty() || info.dllPath.empty()) {
        cerr << "Error: Missing required arguments. Use --help for usage information.\n";
        return false;
    }
    
    if (!fs::exists(info.dllPath)) {
        cerr << "Error: Input DLL file does not exist: " << info.dllPath << endl;
        return false;
    }
    
    return createPluginPackage(info);
}

#ifdef _WIN32
// GUI mode function (Windows only)
void showGUI() {
    cout << "\n=== TS3 Plugin Packager - GUI Mode ===\n\n";
    
    PluginInfo info;
    info.type = "Plugin"; // Set default type
    string input;
    
    // Get plugin information from user
    cout << "Plugin Name: ";
    getline(cin, info.name);
    
    cout << "Author: ";
    getline(cin, info.author);
    
    cout << "Version: ";
    getline(cin, info.version);
    
    cout << "Platforms (e.g., win32, win64, mac): ";
    getline(cin, info.platforms);
    
    cout << "Description: ";
    getline(cin, info.description);
    
    // Get DLL path
    cout << "\nEnter path to plugin DLL (or press Enter to browse): ";
    getline(cin, info.dllPath);
    
    if (info.dllPath.empty()) {
        // Show file picker
        OPENFILENAME ofn;
        char szFile[260] = {0};
        
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = NULL;
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = "DLL Files\0*.dll\0All Files\0*.*\0";
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
        
        if (GetOpenFileName(&ofn) == TRUE) {
            info.dllPath = ofn.lpstrFile;
        } else {
            cout << "No file selected. Exiting.\n";
            return;
        }
    }
    
    // Get output path
    cout << "Output filename (without extension, press Enter for default): ";
    getline(cin, info.outputPath);
    
    if (info.outputPath.empty()) {
        info.outputPath = info.name;
    }
    
    // Create the package
    cout << "\nCreating plugin package...\n";
    if (createPluginPackage(info)) {
        cout << "\nPackage created successfully!\n";
    } else {
        cout << "\nFailed to create package.\n";
    }
    
    cout << "\nPress Enter to exit...";
    cin.get();
}
#else
// GUI mode for non-Windows (console-based)
void showGUI() {
    cout << "\n=== TS3 Plugin Packager - Interactive Mode ===\n\n";
    
    PluginInfo info;
    info.type = "Plugin"; // Set default type
    
    cout << "Plugin Name: ";
    getline(cin, info.name);
    
    cout << "Author: ";
    getline(cin, info.author);
    
    cout << "Version: ";
    getline(cin, info.version);
    
    cout << "Platforms (e.g., win32, win64, mac): ";
    getline(cin, info.platforms);
    
    cout << "Description: ";
    getline(cin, info.description);
    
    cout << "Path to plugin DLL: ";
    getline(cin, info.dllPath);
    
    cout << "Output filename (without extension, press Enter for default): ";
    getline(cin, info.outputPath);
    
    if (info.outputPath.empty()) {
        info.outputPath = info.name;
    }
    
    cout << "\nCreating plugin package...\n";
    if (createPluginPackage(info)) {
        cout << "\nPackage created successfully!\n";
    } else {
        cout << "\nFailed to create package.\n";
    }
}
#endif

int main(int argc, char* argv[]) {
    if (argc > 1) {
        // CLI mode
        if (!processCLI(argc, argv)) {
            return 1;
        }
    } else {
        // GUI/Interactive mode
        showGUI();
    }
    
    return 0;
}