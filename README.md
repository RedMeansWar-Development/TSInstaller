# TS3 Plugin Packager

A C++ tool to package TeamSpeak 3 plugins into `.ts3_plugin` files. Supports both CLI and GUI modes.

## Features

- **CLI Mode**: Use command-line flags for automation and scripting
- **GUI Mode**: Interactive console interface when run without arguments
- **Cross-platform**: Works on Windows, Linux, and macOS
- Creates proper TS3 plugin structure with `package.ini` and `plugins/` directory
- Automatically packages into `.ts3_plugin` format

## Building

### Windows (Visual Studio)

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

The executable will be in `build/bin/Release/tsinstaller.exe`

### Windows (MinGW)

```bash
mkdir build
cd build
cmake -G "MinGW Makefiles" ..
cmake --build .
```

### Linux/macOS

```bash
mkdir build
cd build
cmake ..
make
```

The executable will be in `build/bin/tsinstaller`

### Requirements

- C++17 compatible compiler (GCC 8+, Clang 7+, MSVC 2017+)
- CMake 3.15 or higher
- PowerShell (Windows) or `zip` command (Linux/macOS)

## Usage

### CLI Mode

```bash
tsinstaller --name "My Plugin" \
             --author "Your Name" \
             --version "1.0.0" \
             --platforms "win32, win64, mac" \
             --description "A cool plugin" \
             --input "path/to/plugin.dll" \
             --output "MyPlugin"
```

#### CLI Options

- `-n, --name <name>` - Plugin name (required)
- `-a, --author <author>` - Plugin author (required)
- `-v, --version <version>` - Plugin version (required)
- `-p, --platforms <platforms>` - Supported platforms (required)
  - Example: `"win32, win64, mac"`
- `-d, --description <desc>` - Plugin description (required)
- `-i, --input <path>` - Path to plugin DLL file (required)
- `-o, --output <path>` - Output filename without extension (optional)
  - Default: uses plugin name
- `-h, --help` - Show help message

### GUI/Interactive Mode

Simply run the executable without any arguments:

```bash
tsinstaller
```

You'll be prompted to enter:
1. Plugin Name
2. Author
3. Version
4. Platforms
5. Description
6. Path to DLL (or press Enter to browse on Windows)
7. Output filename

## Example

### CLI Example

```bash
tsinstaller -n "Test Plugin" -a "TeamSpeak Systems" -v "1.0" \
             -p "win32, win64, mac" -d "This is a test plugin." \
             -i "C:/plugins/test.dll" -o "TestPlugin"
```

This creates `TestPlugin.ts3_plugin` with the following structure:
```
TestPlugin.ts3_plugin (zip file)
├── package.ini
└── plugins/
    └── test.dll
```

### package.ini Contents

```ini
[package.ini]
Name = Test Plugin
Type = Plugin
Author = TeamSpeak Systems
Version = 1.0
Platforms = win32, win64, mac
Description = "This is a test plugin."
```

## How It Works

1. Creates a temporary directory structure
2. Generates `package.ini` with your plugin information
3. Copies your DLL to `plugins/` subdirectory
4. Creates a ZIP archive of the structure
5. Renames the ZIP to `.ts3_plugin` extension
6. Cleans up temporary files

## Troubleshooting

### Windows: "Compress-Archive not found"

Make sure PowerShell is installed and available in your PATH.

### Linux/macOS: "zip command not found"

Install zip:
- Ubuntu/Debian: `sudo apt-get install zip`
- macOS: `brew install zip` (or use built-in zip)
- Fedora/RHEL: `sudo yum install zip`

### "Permission denied" errors

- Ensure the input DLL file is accessible
- Check write permissions in the output directory
- Run as administrator/sudo if necessary

## License

This tool is provided as-is for packaging TeamSpeak 3 plugins.

## Notes

- The tool uses `Type = Plugin` by default in package.ini
- Output files have the `.ts3_plugin` extension
- Temporary files are automatically cleaned up
- The DLL filename is preserved when copied to the plugins directory
