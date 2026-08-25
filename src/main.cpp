#include <iostream>
#include <filesystem>
#include <unistd.h>
#include <map>
#include <string>

#include <fstream>

std::multimap<std::string, std::string> hook_file;

void print_help() {
	std::cout << "Root filesystem tools\n\tQuick and simple tool for creation usr-merged root filesystems\n";
	std::cout << "root-fs-tools create-fs <folder> - Create filesystem in selected directory. (it has to be empty)\n";
	std::cout << "root-fs-tools install-hook <fs folder> <folder with hook.list> - Install selected hook in provided filesystem\n";
}

void create_folder(std::filesystem::path& p, std::string dir_name, char* argv[]) {
	std::filesystem::create_directory(p.append(dir_name));
	p = argv[2];
}

void create_sym(std::filesystem::path& p, std::string to, std::string link) {
	std::filesystem::path link2{p};
	link2.append(link);

	std::filesystem::create_directory_symlink(to, link2);
}

void read_hook_file(std::ifstream& stream) {
	std::string line;

	while(std::getline(stream, line)) {
		bool by_space = false;
		std::string inside{""};
		std::string to{""};
		for(char c : line) {
			if(c == ' ' && !by_space) {
				by_space = true;
				continue;
			}

			if(!by_space) {
				inside.push_back(c);
			} else {
				to.push_back(c);
			}
		}

		hook_file.insert({inside, to});
	}
}

int main(int argc, char* argv[]) {

	if(getuid() != 0) {
		std::cout << "Only root can do that!\n";
		return 1;
	}

	if(argc < 3 || argc > 4) {
		print_help();
		return 1;
	}

	if(argc == 3) {
		std::filesystem::path p{argv[2]};
		std::cout << "Target folder: " << argv[2] << "\n";

		std::filesystem::directory_entry entry{p};

		if(!entry.exists() || !entry.is_directory() || !std::filesystem::is_empty(p)) {
			std::cout << "Not a valid directory!\n";
			return 1;
		}

		create_folder(p, "etc", argv);
		create_folder(p, "dev", argv);
		create_folder(p, "home", argv);
		create_folder(p, "root", argv);
		create_folder(p, "tmp", argv);
		create_folder(p, "usr", argv);
		create_folder(p, "usr/lib", argv);
		create_folder(p, "usr/lib32", argv);
		create_folder(p, "usr/lib64", argv);
		create_folder(p, "usr/bin", argv);
		create_folder(p, "usr/include", argv);
		create_folder(p, "usr/share", argv);
		create_folder(p, "var", argv);
		create_folder(p, "proc", argv);
		create_folder(p, "sys", argv);
		create_folder(p, "mnt", argv);

		create_sym(p, "./usr/bin", "bin");
		create_sym(p, "./usr/lib32", "lib32");
		create_sym(p, "./usr/lib64", "lib64");
		create_sym(p, "./usr/lib", "lib");
	} else {
		std::filesystem::path fs_folder_path{argv[2]};
		std::filesystem::path hook_folder_path{argv[3]};

		std::filesystem::directory_entry fs_folder{fs_folder_path};
		std::filesystem::directory_entry hook_folder{hook_folder_path};

		if(!fs_folder.exists() || std::filesystem::is_empty(fs_folder) || !hook_folder.exists() || std::filesystem::is_empty(hook_folder)) {
			std::cout << "Invalid folders provided!\n";
			return 1;
		}

		std::cout << "Installing hook: " << argv[3] << "\n";

		std::filesystem::path hook_file_path{hook_folder_path};
		hook_file_path.append("hook.list");

		std::ifstream hook_file_stream{hook_file_path};

		if(!hook_file_stream.is_open()) {
			std::cout << "Failed to open hook.list\n";
			return 1;
		}

		read_hook_file(hook_file_stream);

		for(auto pair : hook_file) {

			std::cout << pair.first << " " << pair.second << "\n";

			std::filesystem::path _inside{hook_folder_path};
			std::filesystem::path _in_fs{fs_folder_path};

			_inside.append(pair.first);
			_in_fs.append(pair.second);

			std::filesystem::copy(_inside, _in_fs, std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive | std::filesystem::copy_options::copy_symlinks);
		}
	}

	return 0;
}