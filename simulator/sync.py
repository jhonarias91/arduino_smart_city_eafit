import os
import time
import shutil
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler

# Define the destination path where you want to copy the files
destination_folder = r'D:\felipe\Documentos\EAFIT\autoadaptables\arduino_smart_city_eafit\simulator'

# Function to copy the required files
def copy_files(source_folder):
    # List of files to look for
    files_to_copy = ['high_level.ino.elf', 'high_level.ino.hex']
    
    # Loop through the list of files
    for filename in files_to_copy:
        source_file = os.path.join(source_folder, filename)
        
        # Check if the file exists in the source folder
        if os.path.exists(source_file):
            try:
                # Define the destination file path
                destination_file = os.path.join(destination_folder, filename)
                
                # Copy the file to the destination folder
                shutil.copy2(source_file, destination_file)
                print(f"Copied {filename} to {destination_folder}")
            except Exception as e:
                print(f"Error copying {filename}: {e}")
        else:
            print(f"File {filename} not found in {source_folder}")

# Define the event handler for folder changes
class FolderChangeHandler(FileSystemEventHandler):
    def on_modified(self, event):
        if event.is_directory:
            print(f"Folder modified: {event.src_path}")
            copy_files(event.src_path)  # Call the copy function

    def on_created(self, event):
        if event.is_directory:
            print(f"New folder created: {event.src_path}")
            copy_files(event.src_path)  # Call the copy function

    def on_deleted(self, event):
        if event.is_directory:
            print(f"Folder deleted: {event.src_path}")

# Function to monitor the folder
def monitor_folder(parent_folder):
    # Set up the event handler and observer
    event_handler = FolderChangeHandler()
    observer = Observer()
    observer.schedule(event_handler, parent_folder, recursive=True)  # Watch the folder recursively
    observer.start()

    print(f"Monitoring changes in folder: {parent_folder}")

    try:
        while True:
            time.sleep(1)  # Keep the script running
    except KeyboardInterrupt:
        observer.stop()
        print("Folder monitoring stopped.")
    observer.join()

# Example usage
parent_folder = r'C:\Users\felip\AppData\Local\Temp\arduino\sketches'  # Raw string for Windows path
monitor_folder(parent_folder)
