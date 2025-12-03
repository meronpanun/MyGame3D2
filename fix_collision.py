import csv
import os
import sys

# Use raw string for path to avoid escape sequence issues
file_path = r'c:\Users\由留部 瑛人.FICPCROOMB14\Documents\GitHub\MyGame3D2\3DShooting\3DShooting\data\CSV\TutorialStageCollisionData.csv'
temp_file_path = file_path + '.tmp'

Y_THRESHOLD = 50.0
HEIGHT_INCREASE = 1000.0

modified_count = 0
row_count = 0

print(f"Processing file: {file_path}")

try:
    # Use utf-8-sig to handle potential BOM
    with open(file_path, 'r', newline='', encoding='utf-8-sig') as infile, \
         open(temp_file_path, 'w', newline='', encoding='utf-8') as outfile:
        
        reader = csv.reader(infile)
        writer = csv.writer(outfile)
        
        try:
            header = next(reader)
            writer.writerow(header)
            print(f"Header: {header}")
        except StopIteration:
            print("Error: File is empty")
            sys.exit(1)
        
        for row in reader:
            row_count += 1
            if not row:
                continue
            
            # Check if it's a UNIConcrete object
            if row[0].strip() == 'UNIConcrete':
                # Y coordinates are at indices 2, 5, 8
                for i in [2, 5, 8]:
                    if i < len(row):
                        try:
                            y = float(row[i])
                            if y > Y_THRESHOLD:
                                row[i] = f"{y + HEIGHT_INCREASE:.4f}"
                                modified_count += 1
                        except ValueError:
                            print(f"Warning: Could not parse float at row {row_count}, index {i}: {row[i]}")
            
            writer.writerow(row)

    print(f"Processed {row_count} rows.")
    print(f"Modified {modified_count} vertices.")

    if modified_count > 0:
        # Force remove the original file first to avoid permission issues
        if os.path.exists(file_path):
            os.remove(file_path)
        os.rename(temp_file_path, file_path)
        print("File updated successfully.")
    else:
        print("No changes made. Deleting temp file.")
        if os.path.exists(temp_file_path):
            os.remove(temp_file_path)

except Exception as e:
    print(f"An error occurred: {e}")
    if os.path.exists(temp_file_path):
        os.remove(temp_file_path)
    sys.exit(1)
