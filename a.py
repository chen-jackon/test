import os
import shutil

def convert_files(source_dir, destination_dir):
    for root, dirs, files in os.walk(source_dir):
        for file in files:
            if file.endswith(".cpp") or file.endswith(".h"):
                source_file_path = os.path.join(root, file)
                destination_path = root.replace(source_dir, destination_dir)
                destination_file_path = os.path.join(destination_path, f"{file}.txt")

                if not os.path.exists(destination_path):
                    os.makedirs(destination_path)

                with open(source_file_path, "r") as source_file, open(destination_file_path, "w") as dest_file:
                    dest_file.write(source_file.read())

if __name__ == "__main__":
    source_dir = "test"
    destination_dir = "test1"

    convert_files(source_dir, destination_dir)

