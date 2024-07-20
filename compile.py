import os

include_line_numbers = False  # Set to True to include line numbers

def read_files(folder_path, file_extensions, excluded_subdirs):
    combined_content = ""
    for root, dirs, files in os.walk(folder_path):
        dirs[:] = [d for d in dirs if d not in excluded_subdirs]
        for file in files:
            if any(file.endswith(ext) for ext in file_extensions):
                file_path = os.path.join(root, file)
                with open(file_path, 'r', encoding='utf-8') as f:
                    content = f.readlines()
                    combined_content += f"Filename: {file}\n"
                    combined_content += f"File Path: {file_path}\n"
                    combined_content += "Content:\n"
                    if include_line_numbers:
                        for i, line in enumerate(content, start=1):
                            combined_content += f"{i}: {line}"
                    else:
                        for line in content:
                            combined_content += line
                    combined_content += "=" * 50 + "\n\n"
    return combined_content

def write_combined_file(output_file, combined_content):
    with open(output_file, 'w', encoding='utf-8') as f:
        f.write(combined_content)

# Specify the folder path and file extensions to include
folder_path = "./"
file_extensions = ['.cpp', '.hpp', '.md', '.txt']

# Specify the subdirectories to exclude
excluded_subdirs = ['build', 'old', 'tests']

# Specify the output file path
output_file = "combined_files.txt"

# Read files and combine their content
combined_content = read_files(folder_path, file_extensions, excluded_subdirs)

# Write the combined content to the output file
write_combined_file(output_file, combined_content)

print(f"Combined file created: {output_file}")